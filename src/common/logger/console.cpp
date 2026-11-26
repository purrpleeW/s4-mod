#include "common_core.hpp"
#include "logger/console.hpp"

#include "identification/utility.hpp"
#include "identification/game.hpp"
#include "utils/hook.hpp"
#include "utils/nt.hpp"
#include "utils/string.hpp"

#include <version.hpp>

#include "f2p_protection.hpp"

namespace logger {
	namespace {
		console::output_callback_t external_output_cb = nullptr;

		void send_pipe_message(pipe_message_type type, const std::string& message = "") {
			char buffer[0x401];
			auto msg_size = std::min(sizeof(buffer) - 1, message.size());
			ZeroMemory(buffer, sizeof(buffer));
			buffer[0] = static_cast<char>(type);
			utils::hook::copy(&buffer[1], message.data(), msg_size);
			DWORD bytes_written;
			WriteFile(console::pipe_handle, buffer, msg_size + 1, &bytes_written, NULL);
		}
	}

	void console::init(bool avoid_alloc_console, const std::string& alternate_title) {
		owns_console = !avoid_alloc_console;

		if (utils::nt::registry_key console_reg = utils::nt::open_or_create_registry_key(HKEY_CURRENT_USER, "Console")) {
			DWORD target_val = 1;
			RegSetValueExA(console_reg, "VirtualTerminalLevel", 0, REG_DWORD, PTR_AS(BYTE*, &target_val), sizeof(DWORD));
		}

		if (owns_console) {
			crash_timebomb();

			AllocConsole();
			SetConsoleCP(CP_UTF8);
			SetConsoleOutputCP(CP_UTF8);
			freopen_s(PTR_AS(FILE**, stdout), "CONOUT$", "w", stdout);
			freopen_s(PTR_AS(FILE**, stdin), "CONIN$", "r", stdin);

			auto title = alternate_title.empty() ? identification::utility::get_client_name() : alternate_title.c_str();
			SetConsoleTitleW(utils::string::convert(title).c_str());
		}
		else {
			const auto pipe_name = TEXT(R"(\\.\pipe\ZeroProxyConPipe)");
			HANDLE h_pipe = nullptr;
			while (true) {
				h_pipe = CreateFile(pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

				if (h_pipe != INVALID_HANDLE_VALUE) {
					pipe_handle = h_pipe;
					break;
				}

				if (GetLastError() != ERROR_PIPE_BUSY) {
					auto err = std::format("Failed to connect to pipe. Error code: {}", GetLastError());
					MessageBoxA(nullptr, err.c_str(), "ZeroProxy", MB_OK | MB_ICONWARNING);
				}

				WaitNamedPipe(pipe_name, 2000);
			}
			send_pipe_message(pipe_message_type::clear);
			set_title(identification::utility::get_client_name());
		}
	}

	void console::destroy() {
		if (owns_console) {
			fclose(stdin);
			fclose(stdout);
			FreeConsole();
		}
		else if (pipe_handle != nullptr) {
			CloseHandle(pipe_handle);
		}
	}

	void console::set_full_title() {
		auto info = identification::game::get_target_info_game();
		auto game = identification::game::get_target_game();
		if (game.game_hash != 0x0) {
			set_title(std::format("{}: " BUILD_VERSION_STRING " - {}", game.client_name, identification::game::get_full_display_name()));
		}
		else {
			set_title(std::format("{}: " BUILD_VERSION_STRING " - <unknown client version>", info.client_name));
		}
	}

	HANDLE console::get_output_handle() {
		return GetStdHandle(STD_OUTPUT_HANDLE);
	}

	DWORD console::get_console_mode() {
		if (!owns_console) {
			return 0;
		}

		DWORD mode = 0;
		::GetConsoleMode(get_output_handle(), &mode);
		return mode;
	}

	void console::set_console_mode(DWORD console_mode) {
		if (!owns_console) {
			return;
		}

		::SetConsoleMode(get_output_handle(), console_mode);
	}

	void console::set_icon(HICON icon) {
		if (!owns_console || !icon) {
			return;
		}

		auto console_window = GetConsoleWindow();
		SendMessageA(console_window, WM_SETICON, ICON_SMALL, PTR_AS(LPARAM, icon));
		SendMessageA(console_window, WM_SETICON, ICON_BIG, PTR_AS(LPARAM, icon));
		SetWindowPos(console_window, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}

	void console::set_title(const std::string& title) {
		if (owns_console) {
			SetConsoleTitleA(title.c_str());
		}
		else {
			send_pipe_message(pipe_message_type::set_console_title, title);
		}
	}

	void console::set_cursor_pos(COORD pos) {
		if (!owns_console) {
			return;
		}

		SetConsoleCursorPosition(get_output_handle(), pos);
	}

	COORD console::get_cursor_pos() {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (owns_console && GetConsoleScreenBufferInfo(get_output_handle(), &csbi)) {
			return csbi.dwCursorPosition;
		}

		return { -1, -1 };
	}

	std::string console::get_text_at_pos(COORD pos, int length) {
		if (length <= 0 || !owns_console) {
			return "";
		}

		short length_i16 = static_cast<short>(length);

		CHAR_INFO* buffer = new CHAR_INFO[length];
		SMALL_RECT read_region = { pos.X, pos.Y, static_cast<short>(pos.X + length_i16 - 1), pos.Y };
		ReadConsoleOutputA(get_output_handle(), buffer, { length_i16, 1 }, { 0, 0 }, &read_region);

		std::string result;
		for (int i = 0; i < length; ++i) {
			result += buffer[i].Char.AsciiChar;
		}

		delete[] buffer;
		return result;
	}

	void input_thread() {
		while (true) {
			std::string input;
			std::getline(std::cin, input);
			if (console::on_input) {
				console::on_input(input);
			}
		}
	}

	void console::start_input_thread() {
		if (!owns_console) {
			return;
		}

		std::thread thread(input_thread);
		thread.detach();
	}

	void console::write(const std::string& text, std::size_t message_offset) {
		std::lock_guard lock(mutex);
		if (owns_console) {
			set_console_mode(get_console_mode() | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

			COORD cursor_pos = get_cursor_pos();
			COORD target_cursor_pos = { 0, cursor_pos.Y };
			std::string current_input = get_text_at_pos(target_cursor_pos, cursor_pos.X);
			set_cursor_pos(target_cursor_pos);
			std::cout << text << std::flush << std::endl;
			std::cout << current_input << std::flush;
		}
		else if (pipe_handle != nullptr) {
			send_pipe_message(pipe_message_type::print, text);
		}

		if (console::on_output) {
			console::on_output(text, message_offset);
		}

		if (external_output_cb) {
			external_output_cb(text.data(), message_offset);
		}
	}

	extern "C" {
		__declspec(dllexport) void set_output_callback(console::output_callback_t cb) {
			external_output_cb = cb;
		}
	}
}
