#include "common.hpp"

#include <nlohmann/json.hpp>
#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' \
	publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace piped_console {
	int message_box(HWND h_wnd, const std::string& text, const std::string& caption, std::uint32_t type) {
		return MessageBoxA(h_wnd, text.c_str(), caption.c_str(), type);
	}

	void pipe_thread() {
		const char* pipe_name = R"(\\.\pipe\ZeroProxyConPipe)";
		HANDLE h_pipe = CreateNamedPipeA(pipe_name, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 0, 0, NULL);

		SetConsoleTitle(TEXT("Waiting..."));

		if (h_pipe == INVALID_HANDLE_VALUE) {
			message_box(nullptr, std::format("Failed to create console pipe. Error code: {}", GetLastError()), "ZeroProxy", MB_ICONWARNING | MB_OK);
			return;
		}

		if (!ConnectNamedPipe(h_pipe, NULL)) {
			message_box(nullptr, std::format("Failed to connect to client. Error code: {}", GetLastError()), "ZeroProxy", MB_ICONWARNING | MB_OK);
			CloseHandle(h_pipe);
			return;
		}

		SetConsoleTitle(TEXT("Client connected."));

		char buffer[0x402];
		DWORD bytes_read;
		while (ReadFile(h_pipe, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
			if (bytes_read == 0) {
				break;
			}

			for (std::size_t x = bytes_read; x < sizeof(buffer); x++) {
				buffer[x] = '\0';
			}

			auto message_type = static_cast<pipe_message_type>(buffer[0]);
			char* readable_buffer = &buffer[1];

			switch (message_type) {
			case pipe_message_type::set_console_title:
				SetConsoleTitleW(utils::string::convert(readable_buffer).c_str());
				break;
			case pipe_message_type::print:
				std::cout << readable_buffer << std::endl;
				break;
			case pipe_message_type::clear:
				system("cls");
				break;
			}
		}

		CloseHandle(h_pipe);
		SetConsoleTitle(TEXT("Client disconnected."));
	}
}

int main() {
	SetConsoleOutputCP(CP_UTF8);
	while (true) {
		piped_console::pipe_thread();
		Sleep(1);
	}
	return 0;
}
