#include "common.hpp"
#include "component/gsc/script_loading.hpp"

#include "component/assets.hpp"
#include "component/filesystem.hpp"
#include "component/gsc/scripting.hpp"
//#include "component/gsc/script_extension.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"
#include "identification/game.hpp"
#include "component/gsc/lexer/function.hpp"
#include "memory/signature_store.hpp"
#include "resource.h"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/compression.hpp>
#include <utils/hook.hpp>
#include <utils/memory.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

namespace gsc {
	std::unique_ptr<xsk::gsc::s4::context> gsc_ctx = std::make_unique<xsk::gsc::s4::context>(xsk::gsc::instance::server);

	namespace {
		functions::DB_AllocXZoneMemoryT* DB_AllocXZoneMemory{};
		functions::DB_AllocXZoneMemoryInternalT* DB_AllocXZoneMemoryInternal{};
		functions::G_Spawn_LoadStructsT* G_Spawn_LoadStructs{};
		functions::Scr_BeginLoadScriptsT* Scr_BeginLoadScripts{};
		functions::Scr_EndLoadScriptsT* Scr_EndLoadScripts{};

		void* DB_GetRawBuffer_Call{};
		std::size_t* s_reserveSizes{};

		utils::hook::detour db_alloc_x_zone_memory_internal_hook;
		utils::hook::detour db_is_x_asset_default_hook;
		utils::hook::detour g_spawn_load_structs_hook;
		utils::hook::detour scr_begin_load_scripts_hook;
		utils::hook::detour scr_end_load_scripts_hook;

		std::unordered_map<std::string, std::uint32_t> main_handles;
		std::unordered_map<std::string, std::uint32_t> init_handles;

		utils::memory::allocator scriptfile_allocator;
		std::unordered_map<std::string, s4::ScriptFile*> loaded_scripts;
		std::unordered_map<std::string, std::string> embedded_scripts;

		char* script_mem_buf = nullptr;

		constexpr auto beta_script_file_asset_type = s4::XAssetType::ASSET_TYPE_SCRIPTFILE;

		bool uses_beta_script_file_layout() {
			return identification::game::is_config(identification::game::Beta);
		}

		s4::XAssetType get_script_file_asset_type() {
			return uses_beta_script_file_layout()
				? beta_script_file_asset_type
				: s4::XAssetType::ASSET_TYPE_SCRIPTFILE;
		}

		std::size_t get_script_file_size() {
			return uses_beta_script_file_layout() ? sizeof(s4::ScriptFileBeta) : sizeof(s4::ScriptFile);
		}

		int get_script_compressed_len(const s4::ScriptFile* script_file) {
			if (uses_beta_script_file_layout()) {
				return reinterpret_cast<const s4::ScriptFileBeta*>(script_file)->compressed_len_;
			}

			return script_file->compressed_len_;
		}

		const char* get_script_buffer(const s4::ScriptFile* script_file) {
			if (uses_beta_script_file_layout()) {
				return reinterpret_cast<const s4::ScriptFileBeta*>(script_file)->buffer_;
			}

			return script_file->buffer_;
		}

		void initialize_script_file(s4::ScriptFile* script_file, const char* name, const int stack_len,
			const int bytecode_len, const char* stack, std::uint8_t* bytecode) {
			if (uses_beta_script_file_layout()) {
				auto* beta_script_file = reinterpret_cast<s4::ScriptFileBeta*>(script_file);
				beta_script_file->name_ = name;
				beta_script_file->compressed_len_ = 0;
				beta_script_file->len_ = stack_len;
				beta_script_file->bytecode_len_ = bytecode_len;
				beta_script_file->buffer_ = stack;
				beta_script_file->bytecode_ = bytecode;
				return;
			}

			script_file->name_ = name;
			script_file->compressed_len_ = 0;
			script_file->len_ = stack_len;
			script_file->bytecode_len_ = bytecode_len;
			script_file->buffer_ = stack;
			script_file->bytecode_ = bytecode;
		}

		std::vector<std::function<void()>> begin_scripts_callbacks;

		struct {
			char* buf = nullptr;
			char* pos = nullptr;
			const std::uint64_t size = 0x100000i64;
		} script_memory;

		char* allocate_buffer(size_t size) {
			if (script_memory.buf == nullptr) {
				script_memory.buf = script_mem_buf;
				script_memory.pos = script_memory.buf;
			}

			if (script_memory.pos + size > script_memory.buf + script_memory.size) {
				game::Com_Error_impl(s4::errorParm_t::ERR_FATAL, "Out of custom script memory");
			}

			const auto pos = script_memory.pos;
			script_memory.pos += size;
			return pos;
		}

		void free_script_memory() {
			if (script_memory.buf != nullptr) {
				memset(script_memory.buf, 0, PTR_AS(std::size_t, script_memory.pos) - PTR_AS(std::size_t, script_memory.buf));
				script_memory.buf = nullptr;
				script_memory.pos = nullptr;
			}
		}

		void clear() {
			main_handles.clear();
			init_handles.clear();
			loaded_scripts.clear();
			scriptfile_allocator.clear();
			free_script_memory();
			embedded_scripts.clear();
		}

		void db_alloc_x_zone_memory_internal_stub(std::uint64_t* block_size, const char* file_name, s4::XZoneMemory* zone_mem, s4::XArchiveBlocks* archive_blocks,
			s4::DBMemoryType type)
		{
			auto XFILE_BLOCK_SCRIPT = 8;
			bool patch = false; // ugly fix for script memory allocation

			if (_stricmp(file_name, "code_post_gfx") == 0 && type == s4::DBMemoryType::DM_MEMORY_SCRIPT) {
				patch = true;
				LOG("Component/GSC/ScriptLoading", INFO, "Patching memory for '{}'", file_name);
			}

			// TODO: type is different all below this
			if (patch) {
				block_size[XFILE_BLOCK_SCRIPT] += script_memory.size;
			}

			db_alloc_x_zone_memory_internal_hook.invoke<void>(block_size, file_name, zone_mem, archive_blocks, type);

			if (patch) {
				block_size[XFILE_BLOCK_SCRIPT] -= script_memory.size;
				script_mem_buf = PTR_AS(char*, archive_blocks->blocks_[XFILE_BLOCK_SCRIPT].data_ + block_size[XFILE_BLOCK_SCRIPT]);
			}
		}

		void db_alloc_x_zone_memory_stub(std::uint64_t* block_size, const char* file_name, s4::XZoneMemory* zone_mem, s4::XArchiveBlocks* archive_blocks) {
			for (auto x = 0; x < ENUM_UNDER(s4::DBMemoryType::DM_MEMORY_COUNT); x++) {
				db_alloc_x_zone_memory_internal_stub(block_size, file_name, zone_mem, archive_blocks, static_cast<s4::DBMemoryType>(x));
			}
		}

		bool read_raw_script_file(const std::string& name_in, std::string* data) {
			// TODO: you can store rawfile assets and load them here
			/*
			const auto* name_str = name.data();
			if (game::DB_XAssetExists(game::ASSET_TYPE_RAWFILE, name_str) &&
				!game::DB_IsXAssetDefault(game::ASSET_TYPE_RAWFILE, name_str))
			{
				const auto asset = game::DB_FindXAssetHeader(game::ASSET_TYPE_RAWFILE, name_str, false);
				const auto len = game::DB_GetRawFileLen(asset.rawfile);
				data->resize(len);
				game::DB_GetRawBuffer(asset.rawfile, data->data(), len);
				if (len > 0)
				{
					data->pop_back();
				}

				return true;
			}
			*/
			auto name = name_in;
			if (!utils::string::ends_with(name, ".gsc")) {
				name.append(".gsc");
			}
			if (embedded_scripts.contains(name)) {
				*data = embedded_scripts[name];
				return true;
			}
			return filesystem::read_file(name, data);
		}

		s4::ScriptFile* load_custom_script(const char* file_name, const std::string& real_name) {
			if (game::Com_FrontEnd_IsInFrontEnd()) {
				return nullptr;
			}

			if (loaded_scripts.contains(file_name)) {
				return loaded_scripts[file_name];
			}

			std::string source_buffer{};
			if (!read_raw_script_file(real_name, &source_buffer) || source_buffer.empty()) {
				return nullptr;
			}

			// filter out "GSC rawfiles" that were used for development usage and are not meant for us.
			// each "GSC rawfile" has a ScriptFile counterpart to be used instead
			const auto script_file_asset_type = get_script_file_asset_type();
			if (game::DB_XAssetExists(script_file_asset_type, file_name) &&
				!game::DB_IsXAssetDefault(script_file_asset_type, file_name))
			{
				if ((real_name.starts_with("scripts/createfx") || real_name.starts_with("scripts/createart") || real_name.starts_with("scripts/mp"))
					&& (real_name.ends_with("_fx") || real_name.ends_with("_fog") || real_name.ends_with("_hdr")))
				{
					LOG("Component/GSC/ScriptLoading", DEBUG, "Refusing to compile rawfile '{}'", real_name);
					return assets::find_x_asset_header(script_file_asset_type, file_name, false).script_file_;
				}
			}

			try {
				auto& compiler = gsc_ctx->compiler();
				auto& assembler = gsc_ctx->assembler();

				std::vector<std::uint8_t> data;
				data.assign(source_buffer.begin(), source_buffer.end());

				const auto assembly_ptr = compiler.compile(real_name, data);
				const auto output_script = assembler.assemble(*assembly_ptr);

				const auto bytecode = get<0>(output_script);
				const auto stack = get<1>(output_script);

				const auto script_file_ptr = static_cast<s4::ScriptFile*>(scriptfile_allocator.allocate(get_script_file_size()));

				const auto stack_size = static_cast<std::uint32_t>(stack.size + 1);
				const auto byte_code_size = static_cast<std::uint32_t>(bytecode.size + 1);

				const auto stack_buffer = static_cast<char*>(scriptfile_allocator.allocate(stack_size));
				std::memcpy(stack_buffer, stack.data, stack.size);

				const auto bytecode_buffer = PTR_AS(std::uint8_t*, allocate_buffer(byte_code_size));
				std::memcpy(bytecode_buffer, bytecode.data, bytecode.size);

				initialize_script_file(script_file_ptr, file_name, static_cast<int>(stack.size),
					static_cast<int>(bytecode.size), stack_buffer, bytecode_buffer);

				loaded_scripts[file_name] = script_file_ptr;

				LOG("Component/GSC/ScriptLoading", INFO, "Loaded script file: {} [{}]", real_name, file_name);

				return script_file_ptr;
			}
			catch (const std::exception& e) {
				auto error_message = std::format("Exception caught:\n"
					"*********** script compile error *************\n"
					"failed to compile '{}':\n"
					"{}\n"
					"**********************************************",
					real_name, e.what());
				LOG("Component/GSC/ScriptLoading", ERROR, error_message);
				MessageBoxA(nullptr, error_message.c_str(), "Script Loader", MB_OK | MB_ICONWARNING);
				return nullptr;
			}

			return nullptr;
		}

		std::string get_raw_script_file_name(const std::string& name) {
			if (name.ends_with(".gsh") || name.ends_with(".gsc")) {
				return name;
			}

			return name + ".gsc";
		}

		std::string get_script_file_name(const std::string& name) {
			const auto id = gsc_ctx->token_id(name);
			if (!id) {
				return name;
			}

			return std::to_string(id);
		}

		std::pair<xsk::gsc::buffer, std::vector<std::uint8_t>> read_compiled_script_file(const std::string& name, const std::string& real_name) {
			const auto* script_file = game::DB_FindXAssetHeader(get_script_file_asset_type(), name.data(), false).script_file_;
			if (script_file == nullptr) {
				throw std::runtime_error(std::format("Failed to load script file: '{}'", real_name));
			}

			LOG("Component/GSC/ScriptLoading", INFO, "Decompiling script file: '{}'", real_name);

			const auto len = get_script_compressed_len(script_file);
			const std::string stack{ get_script_buffer(script_file), static_cast<std::uint32_t>(len) };

			const auto decompressed_stack = utils::compression::zlib::decompress(stack);

			std::vector<std::uint8_t> stack_data;
			stack_data.assign(decompressed_stack.begin(), decompressed_stack.end());

			return { { get_script_bytecode(const_cast<s4::ScriptFile*>(script_file)),
				static_cast<std::uint32_t>(get_script_bytecode_len(script_file)) }, stack_data };
		}

		void db_get_raw_buffer_stub(const s4::RawFile* raw_file, char* buf, const int size) {
			if (raw_file->len_ > 0 && raw_file->compressed_len_ == 0) {
				std::memset(buf, 0, size);
				std::memcpy(buf, raw_file->buffer_, std::min(static_cast<int>(raw_file->len_), size));
				return;
			}
			game::DB_GetRawBuffer(raw_file, buf, size);
		}

		void register_embedded_scripts() {
			// nope
		}

		void load_script(const std::string& name) {
			const auto scr_context = game::ScriptContext_Server();
			if (!game::Scr_LoadScript(scr_context, name.data())) {
				return;
			}

			const auto main_handle = game::Scr_GetFunctionHandle(scr_context, name.data(), gsc_ctx->token_id("main"));
			if (main_handle) {
				LOG("Component/GSC/ScriptLoading", INFO, "Loaded '{}::main'", name);
				main_handles[name] = main_handle;
			}

			const auto init_handle = game::Scr_GetFunctionHandle(scr_context, name.data(), gsc_ctx->token_id("init"));
			if (init_handle) {
				LOG("Component/GSC/ScriptLoading", INFO, "Loaded '{}::init'", name);
				init_handles[name] = init_handle;
			}
		}

		void load_scripts(const std::filesystem::path& root_dir, const std::filesystem::path& subfolder) {
			std::filesystem::path script_dir = root_dir / subfolder;
			if (!utils::io::directory_exists(script_dir.generic_string())) {
				return;
			}

			const auto scripts = utils::io::list_files(script_dir.generic_string());
			for (const auto& script : scripts) {
				if (!script.generic_string().ends_with(".gsc")) {
					continue;
				}
				std::filesystem::path path(script);
				const auto relative = path.lexically_relative(root_dir).generic_string();
				load_script(relative);
			}
		}

		void load_scripts() {
			if (!game::Com_FrontEnd_IsInFrontEnd()) {
				for (const auto& path : filesystem::get_search_paths()) {
					load_scripts(path, "scripts/");			// meant to override stock GSC
					load_scripts(path, "custom_scripts/");	// for no issues, use custom_scripts/
				}

				// load embedded scripts (registered via register_embedded_scripts)
				for (const auto& [path, _] : embedded_scripts) {
					auto name = path;
					if (name.ends_with(".gsc")) {
						name = name.substr(0, name.size() - 4);
					}
					const auto id = gsc_ctx->token_id(name);
					load_script(id ? std::to_string(id) : name);
				}
			}
			else { // for having fun
				for (const auto& path : filesystem::get_search_paths()) {
					load_scripts(path, "frontend_scripts/");
				}
			}
		}

		int db_is_x_asset_default_stub(s4::XAssetType type, const char* name) {
			auto res = db_is_x_asset_default_hook.invoke<int>(type, name);
			if (type == get_script_file_asset_type() && loaded_scripts.contains(name)) {
				return 0;
			}
			return res;
		}

		void g_spawn_load_structs_stub() {
			if (game::Com_FrontEnd_IsInFrontEnd()) {
				g_spawn_load_structs_hook.invoke<void>();
				return;
			}

			const auto scr_context = game::ScriptContext_Server();
			for (auto& function_handle : main_handles) {
				LOG("Component/GSC/ScriptLoading", DEBUG, "Executing '{}::main'", function_handle.first);
				game::Scr_FreeThread(scr_context, game::Scr_ExecThread(scr_context, function_handle.second, 0));
			}

			for (auto& function_handle : init_handles) {
				LOG("Component/GSC/ScriptLoading", DEBUG, "Executing '{}::init'", function_handle.first);
				game::Scr_FreeThread(scr_context, game::Scr_ExecThread(scr_context, function_handle.second, 0));
			}

			g_spawn_load_structs_hook.invoke<void>();
		}

		using fs_callback = std::pair<xsk::gsc::buffer, std::vector<std::uint8_t>>;
		fs_callback init_compiler_internal(const xsk::gsc::context*, const std::string& include_name) {
			const auto real_name = get_raw_script_file_name(include_name);

			std::string file_buffer;
			if (!read_raw_script_file(real_name, &file_buffer) || file_buffer.empty()) {
				auto name = get_script_file_name(include_name);
				if (name == include_name && include_name.ends_with(".gsc")) {
					name = get_script_file_name(include_name.substr(0, include_name.size() - 4));
				}
				if (game::DB_XAssetExists(get_script_file_asset_type(), name.c_str())) {
					return read_compiled_script_file(name, real_name);
				}
				if (name.ends_with(".gsc")) {
					name = name.substr(0, name.size() - 4);
					if (game::DB_XAssetExists(get_script_file_asset_type(), name.c_str())) {
						return read_compiled_script_file(name, real_name);
					}
				}
				else {
					name += ".gsc";
					if (game::DB_XAssetExists(get_script_file_asset_type(), name.c_str())) {
						return read_compiled_script_file(name, real_name);
					}
				}

				throw std::runtime_error(std::format("Could not load gsc file '{}'", real_name));
			}

			std::vector<std::uint8_t> script_data;
			script_data.assign(file_buffer.begin(), file_buffer.end());

			return { {}, script_data };
		}

		void init_compiler() {
			/*
			const bool dev_script = developer_script ? developer_script->current.enabled : false;
			const auto comp_mode = dev_script ?
				xsk::gsc::build::dev :
				xsk::gsc::build::prod;
			*/
			const auto comp_mode = xsk::gsc::build::prod;
			gsc_ctx->init(comp_mode, init_compiler_internal);
		}

		void scr_begin_load_scripts_stub(s4::scrContext_t* context, char threadMode, std::uint32_t a3) {
			// callbacks to begin load scripts
			for (const auto& callback : begin_scripts_callbacks) {
				callback();
			}

			// start the compiler
			init_compiler();

			scr_begin_load_scripts_hook.invoke<void>(context, threadMode, a3);

			register_embedded_scripts();
			load_scripts();
		}

		void scr_end_load_scripts_stub(s4::scrContext_t* context) {
			// cleanup the compiler
			gsc_ctx->cleanup();
			scr_end_load_scripts_hook.invoke<void>(context);
		}
	}

	std::uint8_t* get_script_bytecode(s4::ScriptFile* script_file) {
		if (uses_beta_script_file_layout()) {
			return reinterpret_cast<s4::ScriptFileBeta*>(script_file)->bytecode_;
		}

		return script_file->bytecode_;
	}

	int get_script_bytecode_len(const s4::ScriptFile* script_file) {
		if (uses_beta_script_file_layout()) {
			return reinterpret_cast<const s4::ScriptFileBeta*>(script_file)->bytecode_len_;
		}

		return script_file->bytecode_len_;
	}

	s4::ScriptFile* find_script(s4::XAssetType type, const char* name, int allow_create_default) {
		std::string real_name = name;
		const auto id = static_cast<std::uint16_t>(std::atoi(name));
		if (id) {
			real_name = gsc_ctx->token_name(id);
		}

		auto* script = load_custom_script(name, real_name);
		if (script) {
			return script;
		}

		if (uses_beta_script_file_layout()) {
			type = get_script_file_asset_type();
		}

		return assets::find_x_asset_header(type, name, allow_create_default).script_file_;
	}

	std::string get_real_script_name(const std::string& given_name) {
		const auto id = static_cast<std::uint16_t>(std::atoi(given_name.c_str()));
		if (id != 0) {
			return gsc_ctx->token_name(id);
		}
		return given_name;
	}

	std::uint32_t get_main_handle(const std::string& name) {
		const auto it = main_handles.find(get_script_file_name(name));
		return it != main_handles.end() ? it->second : 0;
	}

	void register_script(const std::string& path, int resource_id, bool allowed) {
		if (!allowed) {
			return;
		}

		const auto data = utils::nt::load_resource(resource_id);
		if (data.empty()) {
			LOG("Component/GSC/ScriptLoading", ERROR, "Couldn't load resource for '{}'", path);
			return;
		}

		embedded_scripts[path] = data;
		LOG("Component/GSC/ScriptLoading", INFO, "Registered embedded script '{}'", path);
	}

	void on_begin_scripts(const std::function<void()>& callback) {
		begin_scripts_callbacks.push_back(callback);
	}

	class loading final : public generic_component {
	public:
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(DB_AllocXZoneMemory), "E8 ? ? ? ? 4C 8B 6C 24 ? 33 D2 41 B8", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(DB_AllocXZoneMemoryInternal), "E8 ? ? ? ? 48 8B 8F ? ? ? ? 4C 8B C6", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(G_Spawn_LoadStructs), "48 89 5C 24 ? 57 48 83 EC ? 48 8B 1D ? ? ? ? E8 ? ? ? ? 8B 53 ? 45 33 C0 48 8B C8 48 8B F8 E8 ? ? ? ?"
				" 8B D0 48 8B CF E8");

			batch.add(SETUP_POINTER(Scr_BeginLoadScripts), "E8 ? ? ? ? C7 44 24 ? ? ? ? ? E8 ? ? ? ? 85 C0", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(Scr_EndLoadScripts), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? B8");

			batch.add(SETUP_POINTER(DB_GetRawBuffer_Call), "E8 ? ? ? ? 41 6B 87");

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(s_reserveSizes), "4B 8B ? 2E ? ? ? ? FF 15", SETUP_MOD(add(4).based_rel()));
			}
			else {
				batch.add(SETUP_POINTER(s_reserveSizes), "4B 8B B4 26", SETUP_MOD(add(4).based_rel()));
			}
		}

		void post_unpack() override {
			if (!gsc::scripting::should_enable_injector()) {
				return;
			}

			// Allocate script memory (PMem doesn't work)
			db_alloc_x_zone_memory_internal_hook.create(DB_AllocXZoneMemoryInternal, db_alloc_x_zone_memory_internal_stub);
			utils::hook::jump(DB_AllocXZoneMemory, db_alloc_x_zone_memory_stub);

			// Increase allocated script memory
			s_reserveSizes[1 /* DB Script */] += script_memory.size;

			// Load our scripts with an uncompressed stack
			utils::hook::call(DB_GetRawBuffer_Call, db_get_raw_buffer_stub);

			// Compiler start and cleanup, also loads scripts
			scr_begin_load_scripts_hook.create(Scr_BeginLoadScripts, scr_begin_load_scripts_stub);
			scr_end_load_scripts_hook.create(Scr_EndLoadScripts, scr_end_load_scripts_stub);

			// ProcessScript: hook xasset functions to return our own custom scripts
			db_is_x_asset_default_hook.create(game::DB_IsXAssetDefault, db_is_x_asset_default_stub);

			// execute main handle after G_LoadStructs (now called G_Spawn_LoadStructs)
			g_spawn_load_structs_hook.create(G_Spawn_LoadStructs, g_spawn_load_structs_stub);

			assets::on_asset_load([](s4::XAssetType type, const char* given_name, int allow_create_default, s4::XAssetHeader& res) {
				if (type != get_script_file_asset_type()) {
					return;
				}

				auto* script = load_custom_script(given_name, get_real_script_name(given_name));
				if (script) {
					utils::hook::copy(res.script_file_, script, get_script_file_size());
				}
			});

			// clear memory (SV_GameMP_ShutdownGameVM)
			gsc::scripting::on_shutdown([](bool free_scripts, bool is_post_shutdown) {
				if (free_scripts && is_post_shutdown) {
					clear();
				}
			});
		}
	};
}

REGISTER_COMPONENT(gsc::loading)
