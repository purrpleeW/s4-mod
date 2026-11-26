#include "common.hpp"
#include "component/assets.hpp"
#include "component/command.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/string.hpp>
#include <utils/storage/storage_local.hpp>

#include "resource.h"

namespace patches::window {
	namespace {
		HWND game_window{};

		utils::hook::iat_detour create_window_ex_w_hook;
		utils::hook::iat_detour register_class_ex_w_hook;

		utils::concurrency::container<std::string> window_title{};

		void store_window_title(const std::string& title) {
			auto _ = utils::storage::local_storage.set("windowTitle", title);
		}

		void activate_window_title(std::string new_window_title) {
			window_title.access([&](std::string& window_title) {
				window_title = std::move(new_window_title);
			});
		}

		void update_window_title(const std::string& window_title) {
			activate_window_title(window_title);
			store_window_title(window_title);
		}

		void setup_window_title(const std::string& initial_window_title) {
			update_window_title(initial_window_title);
		}

		void load_window_title(const std::string& initial_window_title) {
			const auto window_title_storage = utils::storage::local_storage.get("windowTitle");
			window_title_storage.has_value() ? activate_window_title(window_title_storage.value()) : setup_window_title(initial_window_title);
		}

		std::string cp1252_byte_to_utf8(std::uint8_t byte) {
			wchar_t wch;
			MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char*>(&byte), 1, &wch, 1);

			char utf8[4];
			int len = WideCharToMultiByte(CP_UTF8, 0, &wch, 1, utf8, sizeof(utf8), nullptr, nullptr);

			return std::string(utf8, len);
		}

		std::string title_auto_convert_hex(const std::string& s) {
			std::string out;
			for (size_t i = 0; i < s.size(); ++i) {
				if (s[i] != '#' || i + 1 >= s.size()) {
					out.push_back(s[i]);
					continue;
				}
				if (s[i + 1] != 'x' || i + 3 >= s.size() || !std::isxdigit(s[i + 2]) || !std::isxdigit(s[i + 3])) {
					out.push_back(s[++i]);
					continue;
				}
				std::uint8_t v = 0;
				v += (std::isdigit(s[i + 2]) ? s[i + 2] - '0' : std::tolower(s[i + 2]) - 'a' + 10) * 0x10;
				v += (std::isdigit(s[i + 3]) ? s[i + 3] - '0' : std::tolower(s[i + 3]) - 'a' + 10);
				out += cp1252_byte_to_utf8(v);
				i += 3;
			}
			return out;
		}

		std::string format_title(const std::string& title) {
			auto out = title_auto_convert_hex(title);
			out = utils::string::replace(out, "{client_build_branch_name}", BUILD_BRANCH_NAME);
			out = utils::string::replace(out, "{client_build_commit_hash}", BUILD_COMMIT_HASH);
			out = utils::string::replace(out, "{client_build_version_string}", BUILD_VERSION_STRING);
			out = utils::string::replace(out, "{client_name}", identification::utility::get_client_name());
			out = utils::string::replace(out, "{version}", identification::utility::get_game_version());
			out = utils::string::replace(out, "{version_deltaless}", identification::utility::get_game_deltaless_version());
			out = utils::string::replace(out, "{version_full}", identification::utility::get_game_version_full());
			return out;
		}

		HICON get_icon() {
			static HICON icon{};
			static bool attempted_obtainment = false;
			if (!attempted_obtainment) {
				auto icon_path = assets::get_content_path("icon.ico");
				if (std::filesystem::exists(icon_path)) {
					icon = PTR_AS(HICON, LoadImageW(nullptr, icon_path.wstring().c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
				}
				if (!icon) {
					icon = LoadIconA(client_module, MAKEINTRESOURCE(DEFAULT_ICON));
				}
				attempted_obtainment = true;
			}
			return icon;
		}

		HWND create_window_ex_w_stub(DWORD dw_ex_style, LPCWSTR lp_class_name, LPCWSTR lp_window_name, DWORD dw_style, int x, int y,
			int n_width, int n_height, HWND h_wnd_parent, HMENU h_menu, HINSTANCE h_instance, LPVOID lp_param)
		{
			static bool loaded_title = false;

			auto is_main_window = dw_style == (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX) /* main window style */;
			auto patched_window_name = std::wstring(lp_window_name);

			if (is_main_window) {
				if (!loaded_title) {
					load_window_title("{version_full}");
					loaded_title = true;
				}
				patched_window_name = utils::string::convert(format_title(window_title.copy()));
			}

			auto res = create_window_ex_w_hook.invoke<HWND>(dw_ex_style, lp_class_name, patched_window_name.c_str(), dw_style, x, y,
				n_width, n_height, h_wnd_parent, h_menu, h_instance, lp_param);

			if (is_main_window) {
				{
					BOOL value = TRUE;
					DwmSetWindowAttribute(res, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
				}
				game_window = res;
			}

			return res;
		}

		ATOM register_class_ex_w_stub(WNDCLASSEXW* wc) {
			HICON icon = get_icon();
			if (icon) {
				wc->hIcon = icon;
				wc->hIconSm = icon;
			}
			return register_class_ex_w_hook.invoke<ATOM>(wc);
		}
	}

	struct component final : generic_component {
		void post_load() override {
			create_window_ex_w_hook.create("user32.dll", "CreateWindowExW", create_window_ex_w_stub);
			register_class_ex_w_hook.create("user32.dll", "RegisterClassExW", register_class_ex_w_stub);

			logger::console::set_icon(get_icon());
		}

		void post_unpack() override {
			command::add("wnd_setTitle", [](const command::params_cl& params) {
				if (params.arg_count() != 1) {
					LOG("Component/Patches/Window", WARN, "Invalid number of arguments.");
					return;
				}
				if (!game_window) {
					LOG("Component/Patches/Window", WARN, "No game window detected.");
					return;
				}
				auto title = params.get_arg<std::string>(0);
				update_window_title(title);
				SetWindowTextW(game_window, utils::string::convert(format_title(title)).data());    
			});
		}
	};
}

REGISTER_COMPONENT(patches::window::component) // straight iw8-mod pasting, didn't find a need to re-make this ~ purrplee
