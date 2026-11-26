#include "common.hpp"

#include "component/assets.hpp"
#include "component/assets/camo/asset_camo_ui.hpp"
#include "component/assets/asset_image.hpp"
#include "component/assets/asset_read.hpp"
#include "component/assets/camo/camo_runtime.hpp"
#include "component/assets/camo/camo_settings.hpp"
#include "component/command.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace assets::camo {
	namespace {
		struct layer_slot {
			const char* suffix;
			std::uint8_t first;
			bool linear;
		};

		constexpr std::array layer_slots = { layer_slot{ "color", 1, false }, layer_slot{ "normal", 9, true },
			layer_slot{ "gray", 25, true } };

		struct camo_override {
			s4::Camo* original{};
			const char* original_name{};
			s4::MaterialTextureDef* original_defs{};
			std::uint32_t original_count{};
			std::array<std::uint32_t, 9> original_ids{};
			animation::properties original_properties;
			animation::properties base_properties;
			animation::configuration animation;
			std::chrono::steady_clock::time_point animation_start;
			std::array<s4::GfxImage*, 9> custom_images{};
		};

		struct image_override {
			s4::GfxImage* original;
			s4::GfxImage* custom;
		};

		struct camo_identity {
			s4::Camo* asset;
			s4::MaterialTextureDef* defs;
		};

		using camo_asset_map = std::unordered_map<std::string, s4::Camo*>;
		using image_asset_map = std::unordered_map<std::string, s4::GfxImage*>;
		using image_file_map = std::unordered_map<std::string, std::filesystem::path>;
		using image_override_map = std::unordered_map<std::string, image_override>;

		struct image_reload {
			image_file_map files;
			image_asset_map attempts;
			image_override_map overrides;
			bool has_global_images{};
		};

		struct pending_image_override {
			std::filesystem::path file;
			s4::GfxImage* original;
		};

		struct frontend_work {
			camo_asset_map camos;
			std::unordered_map<std::string, pending_image_override> images;
		};

		struct override_registry {
			std::unordered_set<std::string> custom_folders;
			camo_asset_map observed_camos;
			camo_asset_map pending_camos;
			std::unordered_map<std::string, camo_identity> handled_assets;

			image_asset_map pending_image_assets;
			image_file_map image_files;
			image_asset_map image_attempts;
			image_override_map image_overrides;
			bool has_global_images{};
		};

		std::mutex state_mutex;
		override_registry registry;

		std::unordered_map<std::string, camo_override> active_camos;
		std::atomic_bool reload_requested = true;
		bool was_in_frontend = false;
		const void* camo_property_layout_marker{};
		const void* camo_blend_map_layout_marker{};

		animation::properties capture_camo_properties(const s4::Camo& camo) {
			animation::properties result;
			if (camo_blend_map_layout_marker) {
				result.blend_map_channels = camo.blend_map_channels_;
			}

			std::copy_n(camo.color_tint_, 3, result.tints.begin());
			std::copy_n(camo.albedo_map_scale_bias_, 3, result.albedo.begin());
			std::copy_n(camo.normal_map_scale_bias_, 3, result.normal.begin());
			for (std::size_t index = 0; index < 3; ++index) {
				const auto& uv = camo.uv_scale_offset_[index];
				const auto& rotation = camo.uv_rotate_[index];
				result.uv[index] = { uv.x, uv.y, uv.z, uv.w };
				result.rotation[index] = { rotation.x, rotation.y, rotation.z, rotation.w };
			}

			return result;
		}

		void apply_camo_properties(s4::Camo& camo, const animation::properties& values) {
			if (camo_blend_map_layout_marker) {
				camo.blend_map_channels_ = values.blend_map_channels;
			}

			std::copy_n(values.tints.begin(), 3, camo.color_tint_);
			std::copy_n(values.albedo.begin(), 3, camo.albedo_map_scale_bias_);
			std::copy_n(values.normal.begin(), 3, camo.normal_map_scale_bias_);
			for (std::size_t index = 0; index < 3; ++index) {
				const auto& uv = values.uv[index];
				const auto& rotation = values.rotation[index];
				camo.uv_scale_offset_[index] = { uv.x, uv.y, uv.z, uv.w };
				camo.uv_rotate_[index] = { rotation.x, rotation.y, rotation.z, rotation.w };
			}
		}

		bool load_camo_configuration(camo_override& saved, const std::filesystem::path& directory, const std::string& name) {
			const auto props = directory / "properties.json";
			const auto anim = directory / "animation.json";
			if (!camo_property_layout_marker) {
				if (std::filesystem::exists(props) || std::filesystem::exists(anim)) {
					LOG("Component/Assets/Camo", WARN, "'{}': properties disabled because the native layout check failed", name);
				}

				return false;
			}

			std::vector<std::string> errors;
			bool changed = false;
			if (auto document = reader::read_json(props)) {
				if (document->is_object() && document->contains("blend_map_channels") && !camo_blend_map_layout_marker) {
					errors.emplace_back("blend_map_channels disabled because the native layout check failed");
					document->erase("blend_map_channels");
				}

				changed = settings::properties(*document, saved.base_properties, errors);
			}

			if (const auto document = reader::read_json(anim)) {
				saved.animation = settings::animation_config(*document, errors);
			}

			for (const auto& error : errors) {
				LOG("Component/Assets/Camo", WARN, "'{}': {}", name, error);
			}

			saved.animation_start = std::chrono::steady_clock::now();
			return changed || animation::active(saved.animation);
		}

		bool has_compatible_asset_pools() {
			if (!game::s_assetPools) {
				return false;
			}

			const auto& camo_pool = game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_CAMO)];
			const auto& image_pool = game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_IMAGE)];
			return camo_pool.entries_ && image_pool.entries_ && camo_pool.element_size_ == sizeof(s4::Camo) &&
				image_pool.element_size_ == sizeof(s4::GfxImage);
		}

		std::filesystem::path get_safe_asset_path(const std::filesystem::path& root, const std::string& name) {
			const std::filesystem::path relative(name);
			if (relative.empty() || relative.is_absolute() || relative.has_root_name()) {
				return {};
			}

			for (const auto& part : relative) {
				if (part == "..") {
					return {};
				}
			}

			return root / relative;
		}

		bool is_current_camo(const camo_override& saved) {
			if (!has_compatible_asset_pools()) {
				return false;
			}

			const auto& pool = game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_CAMO)];
			return runtime::in_pool(reinterpret_cast<std::uintptr_t>(saved.original), reinterpret_cast<std::uintptr_t>(pool.entries_),
					   pool.element_size_, pool.pool_size_) &&
				runtime::same_asset(*saved.original, saved.original_name, saved.original_defs, saved.original_count);
		}

		void restore_camo_override(camo_override& saved) {
			if (!is_current_camo(saved)) {
				return;
			}

			auto* current = saved.original;
			std::copy_n(saved.original_ids.data(), 3, current->color_texture_ids_);
			std::copy_n(saved.original_ids.data() + 3, 3, current->normal_texture_ids_);
			std::copy_n(saved.original_ids.data() + 6, 3, current->gray_texture_ids_);
			// some streamed images change IDs after reloading between states (eg. menu -> match -> menu)
			runtime::refresh_native_cache(*current);
			if (camo_property_layout_marker) {
				apply_camo_properties(*current, saved.original_properties);
			}
		}

		bool has_asset_name(const char* pointer, const std::string& name) {
			if (!pointer || name.empty() || name.size() > 256) {
				return false;
			}

			std::array<char, 257> bytes;
			return reader::read_memory(reinterpret_cast<std::uintptr_t>(pointer), bytes.data(), name.size() + 1) &&
				bytes[name.size()] == '\0' && std::string_view(bytes.data(), name.size()) == name;
		}

		camo_override capture_camo_override(s4::Camo* camo) {
			camo_override saved{};
			saved.original = camo;
			saved.original_name = camo->name_;
			saved.original_defs = camo->texture_defs_;
			saved.original_count = camo->texture_def_count_;

			if (camo_property_layout_marker) {
				saved.original_properties = capture_camo_properties(*camo);
				saved.base_properties = saved.original_properties;
			}

			std::copy_n(camo->color_texture_ids_, 3, saved.original_ids.data());
			std::copy_n(camo->normal_texture_ids_, 3, saved.original_ids.data() + 3);
			std::copy_n(camo->gray_texture_ids_, 3, saved.original_ids.data() + 6);
			return saved;
		}

		void mark_camo_handled(const std::string& name, s4::Camo* camo) {
			std::lock_guard lock(state_mutex);
			registry.handled_assets[name] = { camo, camo->texture_defs_ };
		}

		std::size_t load_named_textures(camo_override& saved, const std::filesystem::path& directory, const std::string& name) {
			std::size_t applied = 0;

			for (std::uint32_t index = 0; index < saved.original_count; ++index) {
				const auto& definition = saved.original_defs[index];
				const auto cache = runtime::cache_slot(definition.index_);
				if (!cache || !definition.image_ || !definition.image_->name_) {
					continue;
				}

				const auto* image_name = definition.image_->name_;
				const auto local = get_safe_asset_path(directory, image_name);
				const auto global = get_safe_asset_path(assets::get_content_path("image"), image_name);
				std::filesystem::path file;
				const bool found =
					(!local.empty() && image::find_image_file(local, file)) || (!global.empty() && image::find_image_file(global, file));
				if (!found) {
					continue;
				}

				const bool linear =
					(definition.index_ >= 9 && definition.index_ <= 11) || (definition.index_ >= 25 && definition.index_ <= 27);
				auto* custom = image::load_custom_texture(definition.image_, std::format("{}~{}", name, image_name), file, linear);
				if (!custom) {
					continue;
				}

				saved.custom_images[*cache] = custom;
				++applied;
			}

			return applied;
		}

		bool load_layer_texture(camo_override& saved, const std::filesystem::path& directory, const std::string& name, std::uint8_t layer,
			const layer_slot& slot) {
			const auto texture_name = std::format("layer{}_{}", layer, slot.suffix);
			std::filesystem::path file;
			if (!image::find_image_file(directory / texture_name, file)) {
				return false;
			}

			const auto target = static_cast<std::uint8_t>(slot.first + layer);
			auto* original = runtime::find_upload_template(saved.original_defs, saved.original_count, target);
			if (!original) {
				return false;
			}

			auto* custom = image::load_custom_texture(original, std::format("{}_{}", name, texture_name), file, slot.linear);
			if (!custom) {
				return false;
			}

			saved.custom_images[*runtime::cache_slot(target)] = custom;
			return true;
		}

		std::size_t load_layer_textures(camo_override& saved, const std::filesystem::path& directory, const std::string& name) {
			std::size_t applied = 0;

			for (std::uint8_t layer = 0; layer < 3; ++layer) {
				for (const auto& slot : layer_slots) {
					applied += load_layer_texture(saved, directory, name, layer, slot);
				}
			}

			return applied;
		}

		void apply_camo_override(const std::string& name, s4::Camo* observed = nullptr) {
			auto* camo = observed ? observed : assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_CAMO, name.c_str(), 0).camo_;
			const auto& pool = game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_CAMO)];
			if (!runtime::in_pool(reinterpret_cast<std::uintptr_t>(camo), reinterpret_cast<std::uintptr_t>(pool.entries_),
					pool.element_size_, pool.pool_size_) ||
				!has_asset_name(camo->name_, name) || !camo->texture_defs_ || camo->texture_def_count_ > 256) {
				return;
			}

			mark_camo_handled(name, camo);
			if (const auto it = active_camos.find(name); it != active_camos.end()) {
				if (it->second.original == camo && is_current_camo(it->second)) {
					return;
				}

				active_camos.erase(it);
			}

			auto saved = capture_camo_override(camo);
			const auto root = assets::get_content_path("camo");
			const auto directory = get_safe_asset_path(root, name);
			if (directory.empty()) {
				return;
			}

			const bool properties_applied = load_camo_configuration(saved, directory, name);
			auto applied = load_named_textures(saved, directory, name);
			applied += load_layer_textures(saved, directory, name);
			if (!applied && !properties_applied) {
				return;
			}

			if (!is_current_camo(saved)) {
				return;
			}

			const auto [stored, inserted] = active_camos.emplace(name, std::move(saved));
			if (!inserted) {
				return;
			}

			runtime::overlay(*camo, stored->second.custom_images);
			if (camo_property_layout_marker) {
				apply_camo_properties(*camo, stored->second.base_properties);
			}

			mark_camo_handled(name, camo);
			LOG("Component/Assets/Camo", INFO, "Applied '{}' ({} texture(s), properties/animation: {})", name, applied, properties_applied);
		}

		void restore_active_camos() {
			for (auto& [name, saved] : active_camos) {
				restore_camo_override(saved);
			}

			active_camos.clear();
			image::stop_animations();

			std::lock_guard lock(state_mutex);
			registry.handled_assets.clear();
		}

		std::unordered_set<std::string> discover_custom_camos() {
			std::unordered_set<std::string> discovered;
			const auto camo_root = assets::get_content_path("camo");
			std::error_code error;
			if (!std::filesystem::is_directory(camo_root, error)) {
				return discovered;
			}

			for (const auto& entry : std::filesystem::recursive_directory_iterator(camo_root)) {
				if (!entry.is_directory()) {
					continue;
				}

				const auto name = entry.path().lexically_relative(camo_root).generic_string();
				discovered.emplace(name);
				apply_camo_override(name);
			}

			return discovered;
		}

		image_file_map discover_global_images(const std::filesystem::path& root) {
			image_file_map files;
			std::error_code error;
			if (!std::filesystem::is_directory(root, error)) {
				return files;
			}

			for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
				if (!entry.is_regular_file()) {
					continue;
				}

				const auto extension = entry.path().extension().string();
				const bool supported =
					std::any_of(std::begin(image::supported_exts), std::end(image::supported_exts), [&](const auto* ext) {
						return extension == std::format(".{}", ext);
					});

				if (!supported) {
					continue;
				}

				auto relative = entry.path().lexically_relative(root);
				relative.replace_extension();
				files.emplace(relative.generic_string(), std::filesystem::path{});
			}

			return files;
		}

		image_reload load_image_overrides(const std::unordered_set<std::string>& folders) {
			const auto image_root = assets::get_content_path("image");
			image_reload reload;
			reload.files = discover_global_images(image_root);
			// icons don't need reapplying
			reload.has_global_images = !reload.files.empty();

			for (const auto& [name, file] : ui::refresh(folders).icons) {
				reload.files.insert_or_assign(name, file);
			}

			for (auto& [name, file] : reload.files) {
				if (file.empty() && !image::find_image_file(image_root / name, file)) {
					continue;
				}

				auto* original = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_IMAGE, name.c_str(), 0).image_;
				if (original) {
					reload.attempts.emplace(name, original);
				}

				if (!original || !has_asset_name(original->name_, name)) {
					continue;
				}

				if (auto* custom = image::load_custom_texture(original, name, file)) {
					reload.overrides.emplace(name, image_override{ original, custom });
				}
			}

			return reload;
		}

		void publish_reload(std::unordered_set<std::string> folders, image_reload reload) {
			std::lock_guard lock(state_mutex);
			registry.custom_folders = std::move(folders);
			registry.has_global_images = reload.has_global_images;
			registry.image_files = std::move(reload.files);
			registry.image_attempts = std::move(reload.attempts);
			registry.image_overrides = std::move(reload.overrides);

			if (registry.has_global_images) {
				registry.pending_camos.insert(registry.observed_camos.begin(), registry.observed_camos.end());
			}
		}

		void reload_custom_assets() {
			restore_active_camos();

			auto folders = discover_custom_camos();
			auto images = load_image_overrides(folders);
			publish_reload(std::move(folders), std::move(images));

			LOG("Component/Assets/Camo", INFO, "Custom texture files refreshed ({} camo(s))", active_camos.size());
		}

		frontend_work take_frontend_work() {
			frontend_work work;
			std::lock_guard lock(state_mutex);
			work.camos.swap(registry.pending_camos);

			for (const auto& [name, original] : registry.pending_image_assets) {
				const auto it = registry.image_files.find(name);
				if (it == registry.image_files.end()) {
					continue;
				}

				work.images.emplace(name, pending_image_override{ it->second, original });
			}

			registry.pending_image_assets.clear();
			return work;
		}

		void apply_pending_image(const std::string& name, const pending_image_override& pending) {
			{
				std::lock_guard lock(state_mutex);
				registry.image_attempts[name] = pending.original;
			}

			if (!pending.original || !has_asset_name(pending.original->name_, name)) {
				return;
			}

			auto* custom = image::load_custom_texture(pending.original, name, pending.file);
			if (!custom) {
				return;
			}

			std::lock_guard lock(state_mutex);
			registry.image_overrides[name] = { pending.original, custom };
		}

		void process_frontend_updates() {
			if (reload_requested.exchange(false)) {
				reload_custom_assets();
			}

			const auto work = take_frontend_work();
			for (const auto& [name, pending] : work.images) {
				apply_pending_image(name, pending);
			}

			for (const auto& [name, original] : work.camos) {
				apply_camo_override(name, original);
			}
		}

		void update_camo(const camo_override& saved) {
			if (!is_current_camo(saved)) {
				return;
			}

			auto* current = saved.original;
			runtime::overlay(*current, saved.custom_images);
			if (!camo_property_layout_marker || !animation::active(saved.animation)) {
				return;
			}

			const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - saved.animation_start).count();
			apply_camo_properties(*current, animation::evaluate(saved.base_properties, saved.animation, elapsed));
		}

		void update_camos() {
			if (!image::available() || !has_compatible_asset_pools()) {
				return;
			}

			const bool in_frontend = game::Com_FrontEnd_IsInFrontEnd();
			if (in_frontend && !was_in_frontend) {
				reload_requested = true;
			}

			was_in_frontend = in_frontend;

			// stall streamed textures during ff loading, otherwise game freezes up
			if (assets::is_loading_fastfile()) {
				return;
			}

			try {
				if (in_frontend) {
					process_frontend_updates();
				}

				image::animate();
				for (const auto& [name, saved] : active_camos) {
					update_camo(saved);
				}
			}
			catch (const std::exception& error) {
				LOG("Component/Assets/Camo", WARN, "Custom texture refresh failed, error: {}", error.what());
			}
		}

		void on_camo_lookup(const char* name, s4::XAssetHeader& result) {
			if (!name || !result.camo_) {
				return;
			}

			std::lock_guard lock(state_mutex);
			registry.observed_camos[name] = result.camo_;

			const auto handled = registry.handled_assets.find(name);
			if (handled != registry.handled_assets.end() && handled->second.asset == result.camo_ &&
				handled->second.defs == result.camo_->texture_defs_) {
				return;
			}

			if (registry.custom_folders.contains(name) || registry.has_global_images) {
				registry.pending_camos[name] = result.camo_;
			}
		}

		void on_image_lookup(const char* name, s4::XAssetHeader& result) {
			if (!name) {
				return;
			}

			std::lock_guard lock(state_mutex);
			const auto replacement = registry.image_overrides.find(name);
			if (replacement != registry.image_overrides.end() && result.image_ == replacement->second.original) {
				result.image_ = replacement->second.custom;
				return;
			}

			if (!result.image_ || !registry.image_files.contains(name)) {
				return;
			}

			const auto attempted = registry.image_attempts.find(name);
			if (attempted == registry.image_attempts.end() || attempted->second != result.image_) {
				registry.pending_image_assets[name] = result.image_;
			}
		}

		void on_table_lookup(s4::XAssetType type, const char* name, int, s4::XAssetHeader& result) {
			if (type != s4::XAssetType::ASSET_TYPE_STRINGTABLE || !name || std::string_view(name) != "mp/camotable.csv") {
				return;
			}

			if (result.string_table_ && !reader::is_camo_table_current(result.string_table_)) {
				reload_requested = true;
			}
		}

		void list_camo_textures(const std::string& name) {
			if (!has_compatible_asset_pools()) {
				return;
			}

			auto* camo = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_CAMO, name.c_str(), 0).camo_;
			if (!camo || !camo->texture_defs_ || camo->texture_def_count_ > 256) {
				LOG("Component/Assets/Camo", WARN, "Camo '{}' is not loaded or doesn't contain any valid textures?", name);
				return;
			}

			for (std::uint32_t index = 0; index < camo->texture_def_count_; ++index) {
				const auto& definition = camo->texture_defs_[index];
				const auto* image = definition.image_;
			}
		}
	}

	class component final : public client_component {
	public:
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(camo_blend_map_layout_marker), "4A 8B 44 ? 08 0F B6 88 1E 01 00 00 E9");

			batch.add(SETUP_POINTER(camo_property_layout_marker), // for tint, UV etc
				"8B 44 3E 10 89 07 8B 44 3E 1C 0F BA E8 1F 89 47 0C 8B 44 3E 28 89 47 18 8B 44 3E 34 89 47 24 8B 44 3E 40 89 47 30 8B 44 3E 4C "
				"89 47 3C C4 A1 78 10 44 36 D8 C4 C1 78 11 46 C8 C4 A1 78 10 44 36 14 C4 C1 78 11 46 F8");
		}

		void post_unpack() override {
			assets::register_callback(s4::XAssetType::ASSET_TYPE_CAMO, on_camo_lookup);
			assets::register_callback(s4::XAssetType::ASSET_TYPE_IMAGE, on_image_lookup);

			command::add("images_reload", [] {
				reload_requested = true;
			});

			assets::on_asset_load(on_table_lookup);

			command::add("camo_listTextures", [](const command::params_cl& params) {
				if (params.arg_count() != 1) {
					LOG("Component/Assets/Camo", INFO, "Usage: camo_listTextures <camo asset name>");
					return;
				}

				const std::string name(params.get_arg(0));
				scheduler::once<scheduler::pipeline::client>([name] {
					list_camo_textures(name);
				});
			});

			scheduler::loop<scheduler::pipeline::client>(update_camos);
		}
	};
}

REGISTER_COMPONENT(assets::camo::component)
