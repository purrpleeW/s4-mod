#include "common.hpp"

#include <loader/component_loader.hpp>
#include <utils/flags.hpp>
#include <utils/hook.hpp>

namespace patches::dual_instance {
	namespace {
		utils::hook::detour create_file_a_hook;
		utils::hook::detour create_file_w_hook;
		utils::hook::detour delete_file_a_hook;
		utils::hook::detour delete_file_w_hook;

		std::optional<std::filesystem::path> get_instance_file(const std::filesystem::path& path) {
			if (path.filename() != "__Vanguard") {
				return std::nullopt;
			}

			auto instance_path = path;
			instance_path.replace_filename("__Vanguard-2");
			return instance_path;
		}

		HANDLE create_file_a_stub(LPCSTR file_name, DWORD access, DWORD sharing, LPSECURITY_ATTRIBUTES security,
			DWORD disposition, DWORD attributes, HANDLE template_file)
		{
			if (!file_name) {
				return create_file_a_hook.invoke<HANDLE>(file_name, access, sharing, security, disposition, attributes, template_file);
			}

			const auto instance_path = get_instance_file(file_name);
			const auto instance_file = instance_path ? instance_path->string() : std::string{};
			return create_file_a_hook.invoke<HANDLE>(instance_path ? instance_file.c_str() : file_name,
				access, sharing, security, disposition, attributes, template_file);
		}

		HANDLE create_file_w_stub(LPCWSTR file_name, DWORD access, DWORD sharing, LPSECURITY_ATTRIBUTES security,
			DWORD disposition, DWORD attributes, HANDLE template_file)
		{
			if (!file_name) {
				return create_file_w_hook.invoke<HANDLE>(file_name, access, sharing, security, disposition, attributes, template_file);
			}

			const auto instance_path = get_instance_file(file_name);
			const auto instance_file = instance_path ? instance_path->wstring() : std::wstring{};
			return create_file_w_hook.invoke<HANDLE>(instance_path ? instance_file.c_str() : file_name,
				access, sharing, security, disposition, attributes, template_file);
		}

		BOOL delete_file_a_stub(LPCSTR file_name) {
			const auto instance_path = file_name ? get_instance_file(file_name) : std::nullopt;
			const auto instance_file = instance_path ? instance_path->string() : std::string{};
			return delete_file_a_hook.invoke<BOOL>(instance_path ? instance_file.c_str() : file_name);
		}

		BOOL delete_file_w_stub(LPCWSTR file_name) {
			const auto instance_path = file_name ? get_instance_file(file_name) : std::nullopt;
			const auto instance_file = instance_path ? instance_path->wstring() : std::wstring{};
			return delete_file_w_hook.invoke<BOOL>(instance_path ? instance_file.c_str() : file_name);
		}
	}

	struct component final : client_component {
		void post_load() override {
			if (!utils::flags::has_flag("dual")) {
				return;
			}

			create_file_a_hook.create("kernel32.dll", "CreateFileA", create_file_a_stub);
			create_file_w_hook.create("kernel32.dll", "CreateFileW", create_file_w_stub);
			delete_file_a_hook.create("kernel32.dll", "DeleteFileA", delete_file_a_stub);
			delete_file_w_hook.create("kernel32.dll", "DeleteFileW", delete_file_w_stub);
		}
	};
}

REGISTER_COMPONENT(patches::dual_instance::component)
