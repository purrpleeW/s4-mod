#include "common.hpp"
#include "component/assets.hpp"
#include "component/assets/asset_read.hpp"
#include "component/assets/camo/camo_dump.hpp"
#include "component/command.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/storage/storage_generic.hpp>
#include <utils/string.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>

namespace asset_dumper::camo {
	namespace {
		using assets::reader::read_memory;
		using assets::reader::read_object;
		using assets::reader::read_string;
		using assets::reader::read_table;

		using menu_entries = std::map<std::string, table_entry>;
		using loot_id_map = std::unordered_map<std::string, std::uint32_t>;
		using loaded_camo_map = std::unordered_map<std::string, std::string>;

		struct pool_snapshot {
			std::uintptr_t base;
			std::size_t stride;
			std::vector<std::size_t> occupied;
		};

		bool asset_name(const std::string& name) {
			return !name.empty() && name.size() <= 256 && std::all_of(name.begin(), name.end(), [](unsigned char c) {
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '/' ||
					c == '.';
			});
		}

		std::string label(const std::string& key) {
			if (key.empty()) {
				return "<unlocalized>";
			}

			const auto* address = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_LOCALIZE, key.c_str(), 0).localize_;
			const auto entry = read_object(address);
			if (!entry) {
				return key;
			}

			const auto name = read_string(entry->name_);
			if (!name || utils::string::to_lower(*name) != utils::string::to_lower(key)) {
				return key;
			}

			auto value = read_string(entry->value_);
			if (!value || value->empty()) {
				return key;
			}

			if ((static_cast<unsigned char>((*value)[0]) & 0xC0) != 0x80) {
				return *value;
			}

			if (!game::StrCypher_DecodeEncryptedString || !encrypted_label_is_bounded(*value)) {
				return key;
			}

			const auto decoded = read_string(game::StrCypher_DecodeEncryptedString(value->data()));
			return decoded ? *decoded : key;
		}

		std::string texture_details(const s4::Camo& camo) {
			std::string output;
			if (camo.texture_def_count_ > 256 || (camo.texture_def_count_ && !camo.texture_defs_)) {
				return "  [Invalid texture defs]\n";
			}

			std::vector<s4::MaterialTextureDef> defs(camo.texture_def_count_);
			if (!defs.empty() &&
				!read_memory(reinterpret_cast<std::uintptr_t>(camo.texture_defs_), defs.data(), defs.size() * sizeof(defs[0]))) {
				return "  [Texture defs changed or is unreadable]\n";
			}

			output += defs.empty() ? "  [No textures]\n" : "  Textures:\n";
			for (const auto& def : defs) {
				const auto image = read_object(def.image_);
				const auto name = image ? read_string(image->name_) : std::optional<std::string>{};
				output += std::format("   > slot {}: {} ({}x{}, format {}, packed semantic/category 0x{:02X})\n", def.index_,
					name.value_or("<null/unreadable>"), image ? image->width_ : 0, image ? image->height_ : 0, image ? image->format_ : 0,
					image ? image->semantic_category_ : 0);
			}

			output += std::format("  Blend map channels: {}\n", static_cast<unsigned>(camo.blend_map_channels_));

			for (int layer = 0; layer < 3; ++layer) {
				const auto& uv = camo.uv_scale_offset_[layer];
				const auto& rotation = camo.uv_rotate_[layer];
				output += std::format("  Layer {}: tint=0x{:08X}, albedo=0x{:08X}, normal=0x{:08X}\n"
									  "    UV=[{}, {}, {}, {}], rotation=[{}, {}, {}, {}]\n",
					layer, camo.color_tint_[layer], camo.albedo_map_scale_bias_[layer], camo.normal_map_scale_bias_[layer], uv.x, uv.y,
					uv.z, uv.w, rotation.x, rotation.y, rotation.z, rotation.w);
			}

			return output;
		}

		pool_snapshot read_pool_snapshot() {
			if (!game::s_assetPools) {
				throw std::runtime_error("Asset pools are not initialized yet!");
			}

			const auto* address = &game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_CAMO)];
			const auto pool = read_object(address);
			if (!pool || pool->element_size_ != sizeof(s4::Camo) || !pool->pool_size_ || pool->pool_size_ > 8192) {
				throw std::runtime_error("Unsupported or uninitialized camo pool layout?");
			}

			std::vector<std::byte> snapshot(static_cast<std::size_t>(pool->pool_size_) * pool->element_size_);
			const auto base = reinterpret_cast<std::uintptr_t>(pool->entries_);
			if (!read_memory(base, snapshot.data(), snapshot.size())) {
				throw std::runtime_error("Cannot read camo pool, we're cooked");
			}

			auto occupied = occupied_slots(snapshot, base, pool->element_size_, reinterpret_cast<std::uintptr_t>(pool->free_head_));
			const auto after = read_object(address);
			if (!occupied || !after || after->entries_ != pool->entries_ || after->free_head_ != pool->free_head_ ||
				after->pool_size_ != pool->pool_size_ || after->element_size_ != pool->element_size_) {
				throw std::runtime_error("Camo pool changed or the free list is invalid? try again after zone loading is complete?");
			}

			return { base, pool->element_size_, std::move(*occupied) };
		}

		menu_entries read_menu_entries() {
			menu_entries entries;

			for (const auto& row : read_table("mp/camotable.csv")) {
				if (const auto entry = parse_row(row); entry && asset_name(entry->name)) {
					entries.emplace(entry->name, *entry);
				}
			}

			if (entries.empty()) {
				LOG("Component/AssetDumper", WARN, "mp/camotable.csv unavailable, the dump will not include IDs, categories, localization or icons");
			}

			return entries;
		}

		loot_id_map read_loot_ids() {
			loot_id_map loot_ids;

			for (const auto& row : read_table("loot/camo_ids.csv")) {
				if (row.size() < 2) {
					continue;
				}

				if (const auto id = numeric_id(row[0])) {
					loot_ids.emplace(row[1], *id);
				}
			}

			return loot_ids;
		}

		loaded_camo_map collect_loaded_camos(const pool_snapshot& pool, menu_entries& entries) {
			loaded_camo_map loaded;

			for (const auto slot : pool.occupied) {
				const auto* camo_address = reinterpret_cast<const s4::Camo*>(pool.base + slot * pool.stride);
				const auto camo = read_object(camo_address);
				if (!camo) {
					continue;
				}

				const auto name = read_string(camo->name_);
				if (!name || !asset_name(*name) ||
					assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_CAMO, name->c_str(), 0).camo_ != camo_address) {
					continue;
				}

				entries.try_emplace(*name, table_entry{ *name, {}, {}, {}, {} });
				loaded.emplace(*name, texture_details(*camo));
			}

			return loaded;
		}

		std::string format_names(const menu_entries& entries, const loot_id_map& loot_ids, const loaded_camo_map& loaded) {
			std::string output = "S4 camo names/IDs\nUse the asset name for asset_replacer/camo/<name>/.\n";
			for (const auto& [name, entry] : entries) {
				const auto details = loaded.find(name);
				const auto loot = loot_ids.find(name);
				output += std::format(
					"{} ({}):\n  Menu ID: {}\n  Global loot ID: {}\n  Loaded: {}\n  Category: {}\n  Localization: {}\n  Icon: {}\n", name,
					label(entry.localization), entry.menu_id ? std::to_string(*entry.menu_id) : "<unlisted>",
					loot != loot_ids.end() ? std::to_string(loot->second) : "<unlisted>", details != loaded.end() ? "yes" : "no",
					entry.category, entry.localization, entry.icon);

				if (details != loaded.end()) {
					output += details->second;
				}

				output += '\n';
			}

			return output;
		}

		std::filesystem::path save_names(const std::string& output) {
			const auto directory = utils::storage::get_game_mod_path() / "asset_dumper" / "camo";
			std::filesystem::create_directories(directory);
			const auto file = directory / "names.txt";
			const auto temporary = directory / "names.txt.tmp";
			std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
			stream.write(output.data(), static_cast<std::streamsize>(output.size()));
			stream.close();
			if (!stream || !MoveFileExW(temporary.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
				throw std::runtime_error("Could not save camo names.txt! We might not have write access?");
			}

			return file;
		}

		void dump_names() {
			const auto pool = read_pool_snapshot();
			auto entries = read_menu_entries();
			const auto loot_ids = read_loot_ids();
			const auto loaded = collect_loaded_camos(pool, entries);
			if (entries.empty()) {
				throw std::runtime_error("No camos are currently loaded. Try again in the multiplayer lobby?");
			}

			const auto file = save_names(format_names(entries, loot_ids, loaded));
			LOG("Component/AssetDumper", INFO, "Dumped {} camo(s), {} loaded, to '{}'", entries.size(), loaded.size(), file.string());
		}
	}

	class component final : public client_component {
	public:
		void post_unpack() override {
			command::add("dump_camoNames", [] {
				scheduler::once<scheduler::pipeline::client>([] {
					try {
						dump_names();
					}
					catch (const std::exception& error) {
						LOG("Component/AssetDumper", WARN, "Camo dump failed, error: {}", error.what());
					}
				});
			});
		}
	};
}

REGISTER_COMPONENT(asset_dumper::camo::component)
