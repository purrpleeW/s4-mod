#pragma once
#include "common_core.hpp"

namespace logger::console {
	using output_callback_t = void(__cdecl*)(const char* text, std::size_t message_offset);

	void init(bool avoid_alloc_console = false, const std::string& alternate_title = "");
	void destroy();

	void set_full_title();

	HANDLE get_output_handle();
	DWORD get_console_mode();
	void set_console_mode(DWORD console_mode);
	void set_icon(HICON icon);
	void set_title(const std::string& title);

	void set_cursor_pos(COORD pos);
	COORD get_cursor_pos();
	std::string get_text_at_pos(COORD pos, int length);

	void start_input_thread();

	void write(const std::string& text, std::size_t message_offset = 0);

	extern "C" {
		__declspec(dllexport) void set_output_callback(output_callback_t cb);
	}

	inline bool owns_console = false;
	inline HANDLE pipe_handle = nullptr;
	inline std::function<void(std::string)> on_input{};
	inline std::function<void(std::string, std::size_t)> on_output{};
	inline std::mutex mutex{};
}