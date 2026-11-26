#include "common.hpp"
#include "component/assets.hpp"
#include "component/command.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/compression.hpp>
#include <utils/io.hpp>
#include <utils/storage/storage_generic.hpp>

#include <xsk/stdinc.hpp>
#include <xsk/gsc/engine/s4.hpp>

namespace asset_dumper {
	namespace {
		std::unordered_map<std::string, std::vector<std::uint8_t>> files;
		std::unique_ptr<xsk::gsc::s4::context> script_context = std::make_unique<xsk::gsc::s4::context>(xsk::gsc::instance::server);

		std::filesystem::path get_asset_dump_file_path(s4::XAssetType category, const std::string& file_name) {
			auto path = utils::storage::get_game_mod_path() / "asset_dumper" / assets::get_content_type_name(category) / file_name;
			auto parent = path.parent_path();
			if (!std::filesystem::exists(parent)) {
				std::filesystem::create_directories(parent);
			}
			return path;
		}

		void dump_assets(s4::XAssetType type, std::function<void(s4::XAssetHeader)> callback) {
			auto asset_pools = game::s_assetPools;
			if (asset_pools == nullptr) {
				return;
			}

			auto target_pool = asset_pools[ENUM_UNDER(type)];
			for (auto x = 0; x < target_pool.pool_size_; x++) {
				s4::XAssetHeader asset_header = {
					.data_ = PTR_AS(void*, PTR_AS(std::uintptr_t, target_pool.entries_) + (x * target_pool.element_size_))
				};
				if (asset_header.data_ == nullptr) {
					continue;
				}
				callback(asset_header);
			}
		}

		std::vector<std::uint8_t> zlib_decomp_vec(const void* data, std::size_t data_size) {
			std::vector<std::uint8_t> vec{};
			auto decomp = utils::compression::zlib::decompress(std::string(PTR_AS(const char*, data), data_size));
			for (auto x = 0; x < decomp.size(); x++) {
				vec.push_back(static_cast<std::uint8_t>(decomp.data()[x]));
			}
			return vec;
		}

		template <typename T>
		std::vector<T> ptr_to_vec(const T* data, std::size_t data_size) {
			std::vector<std::uint8_t> vec{};
			for (auto x = 0; x < data_size; x++) {
				vec.push_back(data[x]);
			}
			return vec;
		}

		std::pair<xsk::gsc::buffer, std::vector<std::uint8_t>> fs_read(xsk::gsc::context const* ctx, std::string const& name) {
			auto path = std::filesystem::current_path() / std::filesystem::path{ name };

			auto bin_ext = ".gscbin";
			auto gsc_ext = ".gsc";
			auto gsh_ext = ".gsh";

			if (ctx->instance() == xsk::gsc::instance::client) {
				bin_ext = ".cscbin";
				gsc_ext = ".csc";
			}

			if (!std::filesystem::exists(path)) {
				auto const name_noext = path.replace_extension("").string();

				auto id = ctx->token_id(name_noext);
				if (id > 0) {
					path = std::filesystem::path{ std::to_string(id) + bin_ext };
				}

				if (!std::filesystem::exists(path)) {
					path = std::filesystem::path{ name_noext + bin_ext };
				}
			}

			auto file_data = utils::io::read_file(path);
			std::vector<std::uint8_t> data{};
			for (auto x = 0; x < file_data.size(); x++) {
				data.push_back(static_cast<std::uint8_t>(file_data.data()[x]));
			}

			if (path.extension().string() == bin_ext || (path.extension().string() != gsh_ext && path.extension().string() != gsc_ext)) {
				xsk::gsc::asset s;
				s.deserialize(data);
				auto stk = zlib_decomp_vec(s.buffer.data(), s.len);
				auto res = files.insert({ path.filename().string(), std::move(s.bytecode)});

				if (res.second) {
					return { { res.first->second.data(), res.first->second.size() }, std::move(stk) };
				}
			}
			else {
				return { {}, std::move(data) };
			}

			throw std::runtime_error("file read error");
		}

		std::string serialize_script_file(s4::ScriptFile* script_file) {
			std::string serialized{};

			// script name
			serialized.append(script_file->name_);
			serialized.push_back(0x00);

			// script compressed size (zlib compressed, like other assets)
			for (auto x = 0; x < 4; x++) {
				serialized.push_back(static_cast<char>((script_file->compressed_len_ >> (8 * x)) & 0xFF));
			}

			// script uncompressed size (for validation!)
			for (auto x = 0; x < 4; x++) {
				serialized.push_back(static_cast<char>((script_file->len_ >> (8 * x)) & 0xFF));
			}

			// script bytecode size (this isn't compressed, i don't think.)
			for (auto x = 0; x < 4; x++) {
				serialized.push_back(static_cast<char>((script_file->bytecode_len_ >> (8 * x)) & 0xFF));
			}

			// script data (keep this zlib compressed)
			for (auto x = 0; x < script_file->compressed_len_; x++) {
				serialized.push_back(script_file->buffer_[x]);
			}

			// script bytecode
			for (auto x = 0; x < script_file->bytecode_len_; x++) {
				serialized.push_back(script_file->bytecode_[x]);
			}

			return serialized;
		}
	}

	struct component final : generic_component {
		void post_unpack() override {
			command::add("dump_luaFiles", [] {
				dump_assets(s4::XAssetType::ASSET_TYPE_LUAFILE, [](s4::XAssetHeader asset) {
					auto lua_file = asset.lua_file_;
					if (lua_file->name_ == nullptr || lua_file->buffer_ == nullptr || lua_file->len_ < 1) {
						return;
					}

					std::uint8_t zlib_compression_header[2] = { 0x78, 0xDA };
					std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_LUAFILE, lua_file->name_), std::ios::binary);
					if (memcmp(lua_file->buffer_, zlib_compression_header, sizeof(zlib_compression_header)) == 0) {
						std::string file_data = utils::compression::zlib::decompress(std::string(PTR_AS(const char*, lua_file->buffer_), lua_file->len_));
						file.write(file_data.data(), file_data.size());
					}
					else {
						file.write(PTR_AS(const char*, lua_file->buffer_), lua_file->len_);
					}

					file.close();
				});
			});

			command::add("dump_rawFiles", [] {
				dump_assets(s4::XAssetType::ASSET_TYPE_RAWFILE, [](s4::XAssetHeader asset) {
					auto raw_file = asset.raw_file_;
					if (raw_file->name_ == nullptr || raw_file->buffer_ == nullptr) {
						return;
					}

					std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_RAWFILE, raw_file->name_), std::ios::binary);
					std::string file_data(raw_file->buffer_, raw_file->compressed_len_);

					file_data = utils::compression::zlib::decompress(file_data);
					file.write(file_data.data(), file_data.size());

					file.close();
				});
			});

			command::add("dump_scriptFiles", [] {
				dump_assets(s4::XAssetType::ASSET_TYPE_SCRIPTFILE, [](s4::XAssetHeader asset) {
					auto scr = asset.script_file_;
					if (scr->name_ == nullptr || scr->buffer_ == nullptr || scr->bytecode_ == nullptr) {
						return;
					}

					std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_SCRIPTFILE, assets::get_clean_script_file_name(scr->name_, true)),
						std::ios::binary);
					std::string file_data = serialize_script_file(scr);
					file.write(file_data.data(), file_data.size());
					file.close();
				});
				script_context->init(xsk::gsc::build::prod, fs_read);
				dump_assets(s4::XAssetType::ASSET_TYPE_SCRIPTFILE, [](s4::XAssetHeader asset) {
					auto scr = asset.script_file_;
					if (scr->name_ == nullptr || scr->buffer_ == nullptr || scr->bytecode_ == nullptr) {
						return;
					}

					try {
						auto script = ptr_to_vec(scr->bytecode_, scr->bytecode_len_);
						auto stack = zlib_decomp_vec(scr->buffer_, scr->compressed_len_);

						auto out_asm = script_context->disassembler().disassemble(script, stack);
						auto out_ast = script_context->decompiler().decompile(*out_asm);
						auto out_src = script_context->source().dump(*out_ast);

						std::ofstream decomp_file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_SCRIPTFILE,
							assets::get_clean_script_file_name(scr->name_, false)), std::ios::binary);
						decomp_file.write(PTR_AS(char*, out_src.data()), out_src.size());
						decomp_file.close();
					}
					catch (std::exception& e) {
						LOG("Component/AssetDumper", WARN, "Failed to decompile game script file: {}", e.what());
					}
				});
				script_context->cleanup();
			});

			command::add("dump_stringTables", [] {
				dump_assets(s4::XAssetType::ASSET_TYPE_STRINGTABLE, [](s4::XAssetHeader asset) {
					auto str_table = asset.string_table_;
					if (str_table->name_ == nullptr || str_table->cell_indices_ == nullptr || str_table->strings_ == nullptr) {
						return;
					}

					std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_STRINGTABLE, str_table->name_), std::ios::binary);

					std::vector<std::string> data_lines{};
					for (auto r = 0; r < str_table->row_count_; r++) {
						std::vector<std::string> row_data{};
						for (auto c = 0; c < str_table->column_count_; c++) {
							row_data.push_back(game::StringTable_GetColumnValueForRow(str_table, r, c));
						}
						data_lines.push_back(utils::string::join(row_data, ","));
					}

					std::string file_data = utils::string::join(data_lines, "\n");
					file.write(file_data.data(), file_data.size());

					file.close();
				});
			});

			command::add("dump_ttfs", [] {
				dump_assets(s4::XAssetType::ASSET_TYPE_TTF, [](s4::XAssetHeader asset) {
					auto ttf = asset.ttf_def_;
					if (ttf->name_ == nullptr || ttf->file_ == nullptr || ttf->file_len_ < 1) {
						return;
					}

					std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_TTF, ttf->name_), std::ios::binary);
					file.write(ttf->file_, ttf->file_len_);

					file.close();
				});
			});

			command::add("dump_localize", [] {
				std::unordered_map<std::string, std::string> localize_entries{};

				auto decode_value = [](const char* raw_value) -> std::string {
					const auto* raw_bytes = reinterpret_cast<const unsigned char*>(raw_value);

					if ((raw_bytes[0] & 0xC0) != 0x80 || !game::StrCypher_DecodeEncryptedString) {
						return raw_value;
					}

					std::string scratch(raw_value);
					const char* decoded_ptr = game::StrCypher_DecodeEncryptedString(scratch.data());
					return decoded_ptr ? std::string(decoded_ptr) : scratch;
				};

				dump_assets(s4::XAssetType::ASSET_TYPE_LOCALIZE, [&](s4::XAssetHeader asset) {
					auto localize = asset.localize_;
					if (!localize->name_ || !localize->value_) {
						return;
					}

					localize_entries[localize->name_] = decode_value(localize->value_);
				});

				nlohmann::json localize_json{};
				for (auto& [key, value] : localize_entries) {
					localize_json[key] = value;
				}

				std::string file_data = localize_json.dump(1, '\t');
				std::ofstream file(get_asset_dump_file_path(s4::XAssetType::ASSET_TYPE_LOCALIZE, "localize.json"), std::ios::binary);
				file.write(file_data.data(), file_data.size());
				file.close();
			});
		}
	};
}

REGISTER_COMPONENT(asset_dumper::component)
