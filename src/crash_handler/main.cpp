#include "common.hpp"

#include <wincodec.h>

#include <nlohmann/json.hpp>
#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#include "resource.h"

#pragma comment(lib, "windowscodecs.lib")

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' \
	publicKeyToken='6595b64144ccf1df' language='*'\"")

#define INLINE_HTML_URL(url) \
	"<a href=\"" url "\">" url "</a>"

namespace dialog {
	class game_context_record {
	public:
		/* Register parameter home addresses */
		std::uintptr_t P1Home;
		std::uintptr_t P2Home;
		std::uintptr_t P3Home;
		std::uintptr_t P4Home;
		std::uintptr_t P5Home;
		std::uintptr_t P6Home;

		/* Control flags */
		std::uint32_t ContextFlags;
		std::uint32_t MxCsr;

		/* Segment Registers and processor flags */
		std::uint16_t SegCs;
		std::uint16_t SegDs;
		std::uint16_t SegEs;
		std::uint16_t SegFs;
		std::uint16_t SegGs;
		std::uint16_t SegSs;
		std::uint32_t EFlags;

		/* Debug registers */
		std::uintptr_t Dr0;
		std::uintptr_t Dr1;
		std::uintptr_t Dr2;
		std::uintptr_t Dr3;
		std::uintptr_t Dr6;
		std::uintptr_t Dr7;

		/* Integer registers */
		std::uintptr_t Rax;
		std::uintptr_t Rcx;
		std::uintptr_t Rdx;
		std::uintptr_t Rbx;
		std::uintptr_t Rsp;
		std::uintptr_t Rbp;
		std::uintptr_t Rsi;
		std::uintptr_t Rdi;
		std::uintptr_t R8;
		std::uintptr_t R9;
		std::uintptr_t R10;
		std::uintptr_t R11;
		std::uintptr_t R12;
		std::uintptr_t R13;
		std::uintptr_t R14;
		std::uintptr_t R15;

		/* Program counter */
		std::uintptr_t Rip;

		/* Vector registers */
		std::uintptr_t VectorControl;

		/* Special debug control registers */
		std::uintptr_t DebugControl;
		std::uintptr_t LastBranchToRip;
		std::uintptr_t LastBranchFromRip;
		std::uintptr_t LastExceptionToRip;
		std::uintptr_t LastExceptionFromRip;

		void initialize_from_json(nlohmann::json& data) {
#			define VALUE_FROM_JSON_DATA(n) this->n = static_cast<decltype(this->n)>(data[#n].get<std::int64_t>())
			VALUE_FROM_JSON_DATA(P1Home);
			VALUE_FROM_JSON_DATA(P2Home);
			VALUE_FROM_JSON_DATA(P3Home);
			VALUE_FROM_JSON_DATA(P4Home);
			VALUE_FROM_JSON_DATA(P5Home);
			VALUE_FROM_JSON_DATA(P6Home);
			VALUE_FROM_JSON_DATA(ContextFlags);
			VALUE_FROM_JSON_DATA(MxCsr);
			VALUE_FROM_JSON_DATA(SegCs);
			VALUE_FROM_JSON_DATA(SegDs);
			VALUE_FROM_JSON_DATA(SegEs);
			VALUE_FROM_JSON_DATA(SegFs);
			VALUE_FROM_JSON_DATA(SegGs);
			VALUE_FROM_JSON_DATA(SegSs);
			VALUE_FROM_JSON_DATA(EFlags);
			VALUE_FROM_JSON_DATA(Dr0);
			VALUE_FROM_JSON_DATA(Dr1);
			VALUE_FROM_JSON_DATA(Dr2);
			VALUE_FROM_JSON_DATA(Dr3);
			VALUE_FROM_JSON_DATA(Dr6);
			VALUE_FROM_JSON_DATA(Dr7);
			VALUE_FROM_JSON_DATA(Rax);
			VALUE_FROM_JSON_DATA(Rcx);
			VALUE_FROM_JSON_DATA(Rdx);
			VALUE_FROM_JSON_DATA(Rbx);
			VALUE_FROM_JSON_DATA(Rsp);
			VALUE_FROM_JSON_DATA(Rbp);
			VALUE_FROM_JSON_DATA(Rsi);
			VALUE_FROM_JSON_DATA(Rdi);
			VALUE_FROM_JSON_DATA(R8);
			VALUE_FROM_JSON_DATA(R9);
			VALUE_FROM_JSON_DATA(R10);
			VALUE_FROM_JSON_DATA(R11);
			VALUE_FROM_JSON_DATA(R12);
			VALUE_FROM_JSON_DATA(R13);
			VALUE_FROM_JSON_DATA(R14);
			VALUE_FROM_JSON_DATA(R15);
			VALUE_FROM_JSON_DATA(Rip);
			VALUE_FROM_JSON_DATA(VectorControl);
			VALUE_FROM_JSON_DATA(DebugControl);
			VALUE_FROM_JSON_DATA(LastBranchToRip);
			VALUE_FROM_JSON_DATA(LastBranchFromRip);
			VALUE_FROM_JSON_DATA(LastExceptionToRip);
			VALUE_FROM_JSON_DATA(LastExceptionFromRip);
#			undef VALUE_FROM_JSON_DATA
		}
	};

	class game_exception_record {
	public:
		std::uint32_t ExceptionCode;
		std::uint32_t ExceptionFlags;
		std::uintptr_t ExceptionAddress;
		std::vector<std::uintptr_t> ExceptionInformation;

		void initialize_from_json(nlohmann::json& data) {
#			define VALUE_FROM_JSON_DATA(n) this->n = static_cast<decltype(this->n)>(data[#n].get<std::int64_t>())
			VALUE_FROM_JSON_DATA(ExceptionCode);
			VALUE_FROM_JSON_DATA(ExceptionFlags);
			VALUE_FROM_JSON_DATA(ExceptionAddress);
			for (auto& info : data["ExceptionInformation"]) {
				this->ExceptionInformation.push_back(static_cast<std::uintptr_t>(info.get<std::int64_t>()));
			}
#			undef VALUE_FROM_JSON_DATA
		}
	};

	class game_loaded_module {
	public:
		game_loaded_module(const std::string& name, std::uintptr_t base, std::uint32_t size)
			: name_(name)
			, base_(base)
			, size_(size)
		{}

		std::string name_;
		std::uintptr_t base_;
		std::uint32_t size_;

		std::string get_filename() {
			std::filesystem::path path = this->name_;
			return path.filename().string();
		}

		bool is_address_in_range(std::uintptr_t address) {
			return address >= this->base_ && address <= this->base_ + this->size_;
		}

		bool is_proxy_x64(std::filesystem::path game_path) {
			std::filesystem::path system32_path = "C:\\Windows\\System32";
			return std::filesystem::path(this->name_).parent_path() == game_path && std::filesystem::exists(system32_path / this->get_filename());
		}

		bool is_proxy_x86(std::filesystem::path game_path) {
			std::filesystem::path syswow64_path = "C:\\Windows\\SysWOW64";
			return std::filesystem::path(this->name_).parent_path() == game_path && std::filesystem::exists(syswow64_path / this->get_filename());
		}

		bool is_proxy(std::filesystem::path game_path) {
			return this->is_proxy_x64(game_path) || this->is_proxy_x86(game_path);
		}
	};

	HWND instance = nullptr;
	std::unordered_map<std::uint32_t, std::function<INT_PTR(HWND, WPARAM, LPARAM)>> procedure_callbacks{};

	const char* template_piped_data = R"({"context_record":{"ContextFlags":1048671,"DebugControl":0,"Dr0":0,"Dr1":0,"Dr2":0,"Dr3":0,"Dr6":0,"Dr7":0,"EFlags":66070,"LastBranchFromRip":0,"LastBranchToRip":0,"LastExceptionFromRip":0,"LastExceptionToRip":0,"MxCsr":40937,"P1Home":0,"P2Home":0,"P3Home":2817579181864,"P4Home":0,"P5Home":0,"P6Home":0,"R10":2817579294512,"R11":-9,"R12":92,"R13":31074,"R14":1,"R15":140702368251804,"R8":2817579890464,"R9":2817579184432,"Rax":1,"Rbp":140737488355327,"Rbx":2817579181864,"Rcx":20,"Rdi":2817597023024,"Rdx":2817582292872,"Rip":140702331961110,"Rsi":2817579178304,"Rsp":708707148048,"SegCs":51,"SegDs":43,"SegEs":43,"SegFs":83,"SegGs":43,"SegSs":43,"VectorControl":0},"exception_record":{"ExceptionAddress":140702331961110,"ExceptionCode":3221225477,"ExceptionFlags":0,"ExceptionInformation":[3,0]},"game":{"name":"Vanguard","version":"1.0.0.1337"},"loaded_modules":{"C:\\Windows\\SYSTEM32\\AUDIOSES.DLL":{"base":140705748877312,"size":1576960},"C:\\Windows\\SYSTEM32\\BiWinrt.dll":{"base":140705381810176,"size":339968},"C:\\Windows\\SYSTEM32\\CRYPTBASE.DLL":{"base":140706169356288,"size":49152},"C:\\Windows\\SYSTEM32\\CRYPTSP.dll":{"base":140706169421824,"size":98304},"C:\\Windows\\SYSTEM32\\CoreMessaging.dll":{"base":140705929756672,"size":991232},"C:\\Windows\\SYSTEM32\\CoreUIComponents.dll":{"base":140705922220032,"size":3518464},"C:\\Windows\\SYSTEM32\\D3D12Core.dll":{"base":140704889241600,"size":1888256},"C:\\Windows\\SYSTEM32\\D3DSCache.dll":{"base":140705783349248,"size":176128},"C:\\Windows\\SYSTEM32\\DNSAPI.dll":{"base":140706164047872,"size":827392},"C:\\Windows\\SYSTEM32\\DPAPI.dll":{"base":140706174009344,"size":40960},"C:\\Windows\\SYSTEM32\\FirewallAPI.dll":{"base":140706155134976,"size":614400},"C:\\Windows\\SYSTEM32\\HID.DLL":{"base":140706151071744,"size":53248},"C:\\Windows\\SYSTEM32\\IPHLPAPI.DLL":{"base":140706163720192,"size":241664},"C:\\Windows\\SYSTEM32\\MSASN1.dll":{"base":140706171715584,"size":73728},"C:\\Windows\\SYSTEM32\\NTASN1.dll":{"base":140706170273792,"size":241664},"C:\\Windows\\SYSTEM32\\POWRPROF.dll":{"base":140706165227520,"size":307200},"C:\\Windows\\SYSTEM32\\SSPICLI.DLL":{"base":140706174992384,"size":204800},"C:\\Windows\\SYSTEM32\\TextShaping.dll":{"base":140705786888192,"size":704512},"C:\\Windows\\SYSTEM32\\UMPDC.dll":{"base":140706163589120,"size":73728},"C:\\Windows\\SYSTEM32\\USERENV.dll":{"base":140706174795776,"size":188416},"C:\\Windows\\SYSTEM32\\WINHTTP.dll":{"base":140706105458688,"size":1089536},"C:\\Windows\\SYSTEM32\\WINMM.dll":{"base":140705820180480,"size":159744},"C:\\Windows\\SYSTEM32\\WTSAPI32.dll":{"base":140706113257472,"size":81920},"C:\\Windows\\SYSTEM32\\Windows.Networking.Connectivity.dll":{"base":140705633730560,"size":757760},"C:\\Windows\\SYSTEM32\\Windows.Networking.HostName.dll":{"base":140704800309248,"size":233472},"C:\\Windows\\SYSTEM32\\Windows.Networking.dll":{"base":140704791920640,"size":1003520},"C:\\Windows\\SYSTEM32\\Wldp.dll":{"base":140706170077184,"size":176128},"C:\\Windows\\SYSTEM32\\XINPUT9_1_0.dll":{"base":140705575337984,"size":28672},"C:\\Windows\\SYSTEM32\\XInput1_4.dll":{"base":140705782562816,"size":69632},"C:\\Windows\\SYSTEM32\\atiadlxx.dll":{"base":140705891352576,"size":2236416},"C:\\Windows\\SYSTEM32\\d3d12.dll":{"base":140705769193472,"size":151552},"C:\\Windows\\SYSTEM32\\dcomp.dll":{"base":140705920188416,"size":1982464},"C:\\Windows\\SYSTEM32\\dhcpcsvc.DLL":{"base":140706112667648,"size":118784},"C:\\Windows\\SYSTEM32\\dhcpcsvc6.DLL":{"base":140706112471040,"size":94208},"C:\\Windows\\SYSTEM32\\dwmapi.dll":{"base":140706113388544,"size":192512},"C:\\Windows\\SYSTEM32\\dxcore.dll":{"base":140705797701632,"size":241664},"C:\\Windows\\SYSTEM32\\dxgi.dll":{"base":140706151596032,"size":1007616},"C:\\Windows\\SYSTEM32\\fwbase.dll":{"base":140706154872832,"size":225280},"C:\\Windows\\SYSTEM32\\kernel.appcore.dll":{"base":140706151137280,"size":73728},"C:\\Windows\\SYSTEM32\\ktmw32.dll":{"base":140706099757056,"size":45056},"C:\\Windows\\SYSTEM32\\mskeyprotect.dll":{"base":140705570816000,"size":86016},"C:\\Windows\\SYSTEM32\\ncrypt.dll":{"base":140706170535936,"size":159744},"C:\\Windows\\SYSTEM32\\ntdll.dll":{"base":140706219622400,"size":2064384},"C:\\Windows\\SYSTEM32\\ntmarta.dll":{"base":140706162081792,"size":208896},"C:\\Windows\\SYSTEM32\\pdh.dll":{"base":140705790099456,"size":331776},"C:\\Windows\\SYSTEM32\\resourcepolicyclient.dll":{"base":140706113585152,"size":81920},"C:\\Windows\\SYSTEM32\\textinputframework.dll":{"base":140705898037248,"size":1019904},"C:\\Windows\\SYSTEM32\\twinapi.appcore.dll":{"base":140706065481728,"size":2109440},"C:\\Windows\\SYSTEM32\\windows.storage.dll":{"base":140706143010816,"size":8011776},"C:\\Windows\\SYSTEM32\\wintypes.dll":{"base":140706098249728,"size":1404928},"C:\\Windows\\System32\\ADVAPI32.dll":{"base":140706197471232,"size":724992},"C:\\Windows\\System32\\Bcp47Langs.dll":{"base":140705911472128,"size":372736},"C:\\Windows\\System32\\CRYPT32.dll":{"base":140706184036352,"size":1429504},"C:\\Windows\\System32\\DEVOBJ.dll":{"base":140706173616128,"size":208896},"C:\\Windows\\System32\\DriverStore\\FileRepository\\u0416003.inf_amd64_706cc64a85ddf686\\B415817\\amdenc64.dll":{"base":1682440192,"size":1433600},"C:\\Windows\\System32\\DriverStore\\FileRepository\\u0416003.inf_amd64_706cc64a85ddf686\\B415817\\amdihk64.dll":{"base":140705807597568,"size":245760},"C:\\Windows\\System32\\DriverStore\\FileRepository\\u0416003.inf_amd64_706cc64a85ddf686\\B415817\\amdxc64.dll":{"base":140735062671360,"size":81125376},"C:\\Windows\\System32\\GDI32.dll":{"base":140706210054144,"size":176128},"C:\\Windows\\System32\\IMM32.dll":{"base":140706203631616,"size":192512},"C:\\Windows\\System32\\InputHost.dll":{"base":140705896595456,"size":1384448},"C:\\Windows\\System32\\KERNEL32.DLL":{"base":140706218508288,"size":794624},"C:\\Windows\\System32\\KERNELBASE.dll":{"base":140706176172032,"size":3104768},"C:\\Windows\\System32\\MMDevApi.dll":{"base":140706039070720,"size":544768},"C:\\Windows\\System32\\MSCTF.dll":{"base":140706186199040,"size":1134592},"C:\\Windows\\System32\\NSI.dll":{"base":140706209333248,"size":32768},"C:\\Windows\\System32\\OLEAUT32.dll":{"base":140706200027136,"size":839680},"C:\\Windows\\System32\\PROPSYS.dll":{"base":140706097201152,"size":999424},"C:\\Windows\\System32\\PSAPI.DLL":{"base":140706210250752,"size":32768},"C:\\Windows\\System32\\RPCRT4.dll":{"base":140706202386432,"size":1191936},"C:\\Windows\\System32\\SETUPAPI.dll":{"base":140706204221440,"size":4653056},"C:\\Windows\\System32\\SHELL32.dll":{"base":140706189672448,"size":7790592},"C:\\Windows\\System32\\SHLWAPI.dll":{"base":140706200879104,"size":372736},"C:\\Windows\\System32\\SLC.dll":{"base":140706154217472,"size":167936},"C:\\Windows\\System32\\USER32.dll":{"base":140706187968512,"size":1691648},"C:\\Windows\\System32\\WININET.dll":{"base":140705688125440,"size":5070848},"C:\\Windows\\System32\\WINTRUST.dll":{"base":140706185674752,"size":471040},"C:\\Windows\\System32\\WS2_32.dll":{"base":140706201927680,"size":438272},"C:\\Windows\\System32\\WindowManagementAPI.dll":{"base":140705899085824,"size":659456},"C:\\Windows\\System32\\Windows.Gaming.UI.GameBar.dll":{"base":140705575206912,"size":110592},"C:\\Windows\\System32\\Windows.UI.dll":{"base":140705899806720,"size":1306624},"C:\\Windows\\System32\\appresolver.dll":{"base":140705874640896,"size":606208},"C:\\Windows\\System32\\bcrypt.dll":{"base":140706180038656,"size":159744},"C:\\Windows\\System32\\bcryptprimitives.dll":{"base":140706183446528,"size":532480},"C:\\Windows\\System32\\cfgmgr32.dll":{"base":140706180235264,"size":319488},"C:\\Windows\\System32\\clbcatq.dll":{"base":140706212216832,"size":692224},"C:\\Windows\\System32\\combase.dll":{"base":140706214969344,"size":3485696},"C:\\Windows\\System32\\fwpuclnt.dll":{"base":140706079309824,"size":524288},"C:\\Windows\\System32\\gdi32full.dll":{"base":140706180562944,"size":1150976},"C:\\Windows\\System32\\msvcp_win.dll":{"base":140706181742592,"size":643072},"C:\\Windows\\System32\\msvcrt.dll":{"base":140706201272320,"size":647168},"C:\\Windows\\System32\\ole32.dll":{"base":140706210447360,"size":1224704},"C:\\Windows\\System32\\rasadhlp.dll":{"base":140706087174144,"size":40960},"C:\\Windows\\System32\\sechost.dll":{"base":140706209398784,"size":651264},"C:\\Windows\\System32\\shcore.dll":{"base":140706212937728,"size":708608},"C:\\Windows\\System32\\sppc.dll":{"base":140706154020864,"size":151552},"C:\\Windows\\System32\\ucrtbase.dll":{"base":140706182397952,"size":1048576},"C:\\Windows\\System32\\win32u.dll":{"base":140706185478144,"size":139264},"C:\\Windows\\System32\\winrnr.dll":{"base":140705562886144,"size":73728},"C:\\Windows\\system32\\NLAapi.dll":{"base":140706106572800,"size":118784},"C:\\Windows\\system32\\Secur32.dll":{"base":140705910554624,"size":49152},"C:\\Windows\\system32\\mswsock.dll":{"base":140706167324672,"size":434176},"C:\\Windows\\system32\\napinsp.dll":{"base":140705570553856,"size":94208},"C:\\Windows\\system32\\ncryptsslp.dll":{"base":140705624686592,"size":155648},"C:\\Windows\\system32\\pnrpnsp.dll":{"base":140705570422784,"size":110592},"C:\\Windows\\system32\\rsaenh.dll":{"base":140706159656960,"size":229376},"C:\\Windows\\system32\\schannel.DLL":{"base":140706158673920,"size":618496},"C:\\Windows\\system32\\uxtheme.dll":{"base":140706111619072,"size":647168},"C:\\Windows\\system32\\version.dll":{"base":140706062860288,"size":40960},"C:\\Windows\\system32\\wshbth.dll":{"base":140705570291712,"size":118784},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\AnselSDK64.dll":{"base":140735588466688,"size":675840},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\GfeSDK.dll":{"base":140735143804928,"size":4255744},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\VERSION.dll":{"base":140735148064768,"size":2551808},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\amd_ags_x64.dll":{"base":140736076775424,"size":184320},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\bink2w64.dll":{"base":140735594561536,"size":503808},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\dbghelp.dll":{"base":1679622144,"size":1662976},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\discord_game_sdk.dll":{"base":140735150620672,"size":3899392},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\game_dx12_ship_replay.exe":{"base":140702328815616,"size":321171456},"E:\\Games\\Call of Duty Modern Warfare (1.20)\\oo2core_7_win64.dll":{"base":140735170543616,"size":1044480}}})";

	std::string game_name{};
	std::string game_version{};
	game_context_record game_ctx_record{};
	game_exception_record game_exc_record{};
	std::vector<game_loaded_module> game_loaded_modules{};
	std::vector<std::uintptr_t> game_callstack{};

	std::string target_clipboard_text = "Exception not initialized.";
	std::string window_title{};

	void initialize_exception_data(const std::string& json_str) {
		nlohmann::json json_data = nlohmann::json::parse(json_str);

		game_name = json_data["game"]["name"];
		game_version = json_data["game"]["version"];

		game_ctx_record.initialize_from_json(json_data["context_record"]);
		game_exc_record.initialize_from_json(json_data["exception_record"]);
		for (auto& item : json_data["loaded_modules"].items()) {
			game_loaded_modules.push_back(game_loaded_module(item.key(), static_cast<std::uintptr_t>(item.value()["base"].get<std::int64_t>()),
				static_cast<std::uint32_t>(item.value()["size"].get<std::int64_t>())));
		}

		if (json_data.contains("callstack")) {
			for (auto& item : json_data["callstack"]) {
				game_callstack.push_back(static_cast<std::uintptr_t>(item.get<std::int64_t>()));
			}
		}
	}

	std::string and_rel_ex(std::uintptr_t address) {
		for (auto& mod : game_loaded_modules) {
			if (!mod.is_address_in_range(address)) {
				continue;
			}
			return std::format("0x" FMT_PTR " ({}+0x{:X})", address, mod.get_filename(), address - mod.base_);
		}
		return std::format("0x" FMT_PTR, address);
	}

	void set_dialog_window_title(const WCHAR* title) {
		if (instance == nullptr) {
			return;
		}
		SetWindowTextW(instance, title);
	}

	template <typename T>
	void set_dialog_banner(T* ptr) {
		if (instance == nullptr || ptr == nullptr) {
			return;
		}
		SendDlgItemMessage(instance, IDC_EX_DLG_BANNER, STM_SETIMAGE, IMAGE_BITMAP, PTR_AS(LPARAM, ptr));
	}

	template <typename T>
	void set_dialog_icon(T* ptr) {
		if (instance == nullptr || ptr == nullptr) {
			return;
		}
		SendDlgItemMessage(instance, IDC_EX_DLG_ICON, STM_SETICON, PTR_AS(WPARAM, ptr), 0);
	}

	void set_dialog_title(const TCHAR* title) {
		if (instance == nullptr) {
			return;
		}

		HDC hdc = GetDC(nullptr);
		int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(nullptr, hdc);

		int font_height = -MulDiv(12, dpi, 72);

		HFONT bold_font = CreateFont(font_height, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft Sans Serif"));

		SendDlgItemMessage(instance, IDC_EX_DLG_TITLE, WM_SETFONT, PTR_AS(WPARAM, bold_font), TRUE);
		SetDlgItemText(instance, IDC_EX_DLG_TITLE, title);
	}

	void set_dialog_syslink_text(const TCHAR* text) {
		if (instance == nullptr) {
			return;
		}

		HWND dlg_item = GetDlgItem(instance, IDC_EX_DLG_SYSLINK);
		if (dlg_item == nullptr) {
			return;
		}

		SetWindowText(dlg_item, text);
	}

	void set_dialog_scan_button_text(const TCHAR* text) {
		if (instance == nullptr) {
			return;
		}

		HWND dlg_item = GetDlgItem(instance, IDC_EX_DLG_SCAN_BTN);
		if (dlg_item == nullptr) {
			return;
		}

		EnableWindow(dlg_item, text != nullptr ? TRUE : FALSE);
		ShowWindow(dlg_item, text != nullptr ? SW_SHOW : SW_HIDE);

		if (text != nullptr) {
			SetDlgItemText(instance, IDC_EX_DLG_SCAN_BTN, text);
		}
	}

	void set_dialog_close_button_text(const TCHAR* text) {
		if (instance == nullptr) {
			return;
		}

		HWND dlg_item = GetDlgItem(instance, IDC_EX_DLG_CLOSE_BTN);
		if (dlg_item == nullptr) {
			return;
		}

		EnableWindow(dlg_item, text != nullptr ? TRUE : FALSE);
		ShowWindow(dlg_item, text != nullptr ? SW_SHOW : SW_HIDE);

		if (text != nullptr) {
			SetDlgItemText(instance, IDC_EX_DLG_CLOSE_BTN, text);
		}
	}

	void register_procedure_callback(std::uint32_t target_msg, std::function<INT_PTR(HWND, WPARAM, LPARAM)> callback) {
		procedure_callbacks[target_msg] = callback;
	}

	INT_PTR CALLBACK dialog_proc(HWND h_dlg, UINT u_msg, WPARAM w_param, LPARAM l_param) {
		if (procedure_callbacks.contains(u_msg)) {
			return procedure_callbacks[u_msg](h_dlg, w_param, l_param);
		}
		return FALSE;
	}
}

int i_main(HINSTANCE h_inst, std::vector<std::string> c_args) {
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	INITCOMMONCONTROLSEX icex = {
		sizeof(icex), ICC_LINK_CLASS
	};
	InitCommonControlsEx(&icex);

	dialog::initialize_exception_data(utils::cryptography::base64::decode(c_args[1]));

	dialog::register_procedure_callback(WM_INITDIALOG, [](HWND h_dlg, WPARAM, LPARAM) {
		if (dialog::instance == nullptr) {
			dialog::instance = h_dlg;
		}

		RECT rc;
		GetClientRect(h_dlg, &rc);
		int dialog_width = rc.right - rc.left;
		int banner_height = MulDiv(dialog_width, 60, 650);

		dialog::window_title = std::format("Call of Duty®: {} - Crash handler", dialog::game_name);

		dialog::set_dialog_window_title(utils::string::convert(dialog::window_title).c_str());
		dialog::set_dialog_banner(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BANNER_S4_IMG), IMAGE_BITMAP, dialog_width, banner_height,
			LR_DEFAULTCOLOR | LR_CREATEDIBSECTION));

		dialog::set_dialog_icon(LoadIcon(nullptr, IDI_ERROR));
		dialog::set_dialog_title(TEXT("The application has unexpectedly stopped working."));

		std::filesystem::path game_path = "";
		std::string main_executable = "<unknown>";
		for (auto& mod : dialog::game_loaded_modules) {
			if (!utils::string::ends_with(mod.name_, ".exe")) {
				continue;
			}
			game_path = std::filesystem::path(mod.name_).parent_path();
			main_executable = mod.get_filename();
			break;
		}

		std::string exception_code_name = "<unknown>";
		static std::unordered_map<DWORD, std::string> exception_names = {
			{ 0x1337, "Manually triggered dev error" },
			{ 0x1338, "Manually triggered dev error" },
			{ EXCEPTION_ACCESS_VIOLATION, "Access violation" },
			{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array bounds exceeded" },
			{ EXCEPTION_BREAKPOINT, "Breakpoint" },
			{ EXCEPTION_DATATYPE_MISALIGNMENT, "Data type misalignment" },
			{ EXCEPTION_FLT_DENORMAL_OPERAND, "Float denormal operand" },
			{ EXCEPTION_FLT_DIVIDE_BY_ZERO, "Float / 0" },
			{ EXCEPTION_FLT_INEXACT_RESULT, "Float inexact result" },
			{ EXCEPTION_FLT_INVALID_OPERATION, "Float invalid operation" },
			{ EXCEPTION_FLT_OVERFLOW, "Float overflow" },
			{ EXCEPTION_FLT_STACK_CHECK, "Float stack check" },
			{ EXCEPTION_FLT_UNDERFLOW, "Float underflow" },
			{ EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal instruction" },
			{ EXCEPTION_IN_PAGE_ERROR, "In-page error" },
			{ EXCEPTION_INT_DIVIDE_BY_ZERO, "Int / 0" },
			{ EXCEPTION_INT_OVERFLOW, "Integer overflow" },
			{ EXCEPTION_INVALID_DISPOSITION, "Invalid disposition" },
			{ EXCEPTION_NONCONTINUABLE_EXCEPTION, "Non-continuable exception" },
			{ EXCEPTION_PRIV_INSTRUCTION, "Private instruction" },
			{ EXCEPTION_SINGLE_STEP, "Single step" },
			{ EXCEPTION_STACK_OVERFLOW, "Stack overflow" }
		};
		if (exception_names.contains(dialog::game_exc_record.ExceptionCode)) {
			exception_code_name = exception_names[dialog::game_exc_record.ExceptionCode];
		}

		std::vector<std::string> proxied_modules{};
		for (auto& mod : dialog::game_loaded_modules) {
			if (!mod.is_proxy(game_path)) {
				continue;
			}
			proxied_modules.push_back(mod.get_filename());
		}

		std::string syslink_text = std::format("\n"
			"Game version: {}"
			"\n"
			"Loaded Modules: {}\n"
			"Proxied Modules: {}\n"
			"\n"
			"Error Code: {} (0x{:X})\n"
			"Location: {}\n"
			"Executable: {}",
			dialog::game_version,
			dialog::game_loaded_modules.size(),
			proxied_modules.empty() ? "None" : utils::string::join(proxied_modules, ", "),
			exception_code_name, dialog::game_exc_record.ExceptionCode,
			dialog::and_rel_ex(dialog::game_exc_record.ExceptionAddress),
			main_executable);
		dialog::set_dialog_syslink_text(utils::string::va("Copy the full exception data to your clipboard to send it to our dev team.\n%s",
			syslink_text.c_str()));

		std::string clipboard_text = dialog::game_name + " crash log\n" + syslink_text + "\n";

		clipboard_text += "\n---- Exception Record ----\n";
		clipboard_text += std::format("Exception code: {} (0x{:08X})\n", exception_code_name, dialog::game_exc_record.ExceptionCode);
		clipboard_text += std::format("Exception flags: 0x{:08X}\n", dialog::game_exc_record.ExceptionFlags);
		clipboard_text += std::format("Exception address: {}\n", dialog::and_rel_ex(dialog::game_exc_record.ExceptionAddress));
		clipboard_text += std::format("Number of parameters: {}\n", dialog::game_exc_record.ExceptionInformation.size());
		for (auto x = 0; x < dialog::game_exc_record.ExceptionInformation.size(); x++) {
			clipboard_text += std::format(" > Parameter {}: {}\n", x, dialog::and_rel_ex(dialog::game_exc_record.ExceptionInformation[x]));
		}

		clipboard_text += "\n----- Context Record -----\n";
#		if defined(_WIN64)
			clipboard_text += std::format("P1Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P1Home));
			clipboard_text += std::format("P2Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P2Home));
			clipboard_text += std::format("P3Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P3Home));
			clipboard_text += std::format("P4Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P4Home));
			clipboard_text += std::format("P5Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P5Home));
			clipboard_text += std::format("P6Home: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.P6Home));

			clipboard_text += std::format("Context flags: 0x{:08X}\n", dialog::game_ctx_record.ContextFlags);
			clipboard_text += std::format("MxCsr: 0x{:08X}\n", dialog::game_ctx_record.MxCsr);

			clipboard_text += std::format("SegCs: 0x{:04X}\n", dialog::game_ctx_record.SegCs);
			clipboard_text += std::format("SegDs: 0x{:04X}\n", dialog::game_ctx_record.SegDs);
			clipboard_text += std::format("SegEs: 0x{:04X}\n", dialog::game_ctx_record.SegEs);
			clipboard_text += std::format("SegFs: 0x{:04X}\n", dialog::game_ctx_record.SegFs);
			clipboard_text += std::format("SegGs: 0x{:04X}\n", dialog::game_ctx_record.SegGs);
			clipboard_text += std::format("SegSs: 0x{:04X}\n", dialog::game_ctx_record.SegSs);
			clipboard_text += std::format("EFlags: 0x{:08X}\n", dialog::game_ctx_record.EFlags);

			clipboard_text += std::format("Dr0: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr0));
			clipboard_text += std::format("Dr1: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr1));
			clipboard_text += std::format("Dr2: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr2));
			clipboard_text += std::format("Dr3: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr3));
			clipboard_text += std::format("Dr6: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr6));
			clipboard_text += std::format("Dr7: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr7));

			clipboard_text += std::format("Rax: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rax));
			clipboard_text += std::format("Rcx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rcx));
			clipboard_text += std::format("Rdx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rdx));
			clipboard_text += std::format("Rbx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rbx));
			clipboard_text += std::format("Rsp: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rsp));
			clipboard_text += std::format("Rbp: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rbp));
			clipboard_text += std::format("Rsi: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rsi));
			clipboard_text += std::format("Rdi: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rdi));
			clipboard_text += std::format("R8: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R8));
			clipboard_text += std::format("R9: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R9));
			clipboard_text += std::format("R10: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R10));
			clipboard_text += std::format("R11: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R11));
			clipboard_text += std::format("R12: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R12));
			clipboard_text += std::format("R13: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R13));
			clipboard_text += std::format("R14: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R14));
			clipboard_text += std::format("R15: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.R15));

			clipboard_text += std::format("Rip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Rip));

			clipboard_text += std::format("VectorControl: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.VectorControl));

			clipboard_text += std::format("DebugControl: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.DebugControl));
			clipboard_text += std::format("LastBranchToRip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.LastBranchToRip));
			clipboard_text += std::format("LastBranchFromRip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.LastBranchFromRip));
			clipboard_text += std::format("LastExceptionToRip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.LastExceptionToRip));
			clipboard_text += std::format("LastExceptionFromRip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.LastExceptionFromRip));

			if (!dialog::game_callstack.empty()) {
				clipboard_text += "\n------- Callstack -------\n";
				for (std::size_t x = 0; x < dialog::game_callstack.size(); x++) {
					clipboard_text += std::format("Function address [{}]: {}\n", x, dialog::and_rel_ex(dialog::game_callstack[x]));
				}
			}
#		else
			clipboard_text += std::format("Context flags: 0x{:08X}\n", dialog::game_ctx_record.ContextFlags);

			clipboard_text += std::format("Dr0: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr0));
			clipboard_text += std::format("Dr1: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr1));
			clipboard_text += std::format("Dr2: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr2));
			clipboard_text += std::format("Dr3: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr3));
			clipboard_text += std::format("Dr6: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr6));
			clipboard_text += std::format("Dr7: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Dr7));

			clipboard_text += std::format("SegGs: 0x{:08X}\n", dialog::game_ctx_record.SegGs);
			clipboard_text += std::format("SegFs: 0x{:08X}\n", dialog::game_ctx_record.SegFs);
			clipboard_text += std::format("SegEs: 0x{:08X}\n", dialog::game_ctx_record.SegEs);
			clipboard_text += std::format("SegDs: 0x{:08X}\n", dialog::game_ctx_record.SegDs);

			clipboard_text += std::format("Edi: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Edi));
			clipboard_text += std::format("Esi: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Esi));
			clipboard_text += std::format("Ebx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Ebx));
			clipboard_text += std::format("Edx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Edx));
			clipboard_text += std::format("Ecx: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Ecx));
			clipboard_text += std::format("Eax: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Eax));

			clipboard_text += std::format("Ebp: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Ebp));
			clipboard_text += std::format("Eip: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Eip));
			clipboard_text += std::format("SegCs: 0x{:08X}\n", dialog::game_ctx_record.SegCs);
			clipboard_text += std::format("EFlags: 0x{:08X}\n", dialog::game_ctx_record.EFlags);
			clipboard_text += std::format("Esp: {}\n", dialog::and_rel_ex(dialog::game_ctx_record.Esp));
			clipboard_text += std::format("SegSs: 0x{:04X}\n", dialog::game_ctx_record.SegSs);
#		endif
		dialog::target_clipboard_text = clipboard_text;

		dialog::set_dialog_scan_button_text(TEXT("Copy to Clipboard"));

		/*
		dialog::set_dialog_title(TEXT("The application has unexpectedly stopped working."));

		dialog::set_dialog_syslink_text(TEXT("Select Scan and Repair to restart the game and authorize Battle.net to verify your"
			" installation.\n"
			"This will take a few minutes but it might solve your current issue.\n"
			"\n"
			"To contact customer service support, go to " INLINE_HTML_URL("https://support.activision.com/modern-warfare-ii") "\n"
			"\n"
			"Error Code: 0xC0000005 (0) N\n"
			"Signature: 3A2AB5BF-65678C69-C37034BB-3E8EBA04\n"
			"Location: 0x0000000143E911A7 (15332815)\n"
			"Executable: game_dx12_ship.exe"));

		dialog::set_dialog_scan_button_text(TEXT("Scan and Repair"));
		*/

		/*
		dialog::set_dialog_title(TEXT("DirectX Error"));

		dialog::set_dialog_syslink_text(TEXT("DirectX encountered an unrecoverable error.\n"
			"\n"
			"To contact customer service support, go to " INLINE_HTML_URL("https://support.activision.com/modern-warfare") "\n"
			"\n"
			"Error Code: 0xC0000005 (0) N\n"
			"Signature: 3A2AB5BF-65678C69-C37034BB-3E8EBA04\n"
			"Location: 0x0000000143E911A7 (15332815)\n"
			"Executable: game_dx12_ship.exe"));

		dialog::set_dialog_scan_button_text(nullptr);
		*/

		dialog::set_dialog_close_button_text(TEXT("Close"));

		return TRUE;
	});

	dialog::register_procedure_callback(WM_COMMAND, [](HWND h_dlg, WPARAM w_param, LPARAM) {
		if (HIWORD(w_param) != BN_CLICKED) {
			return FALSE;
		}

		switch (LOWORD(w_param)) {
		case IDC_EX_DLG_SCAN_BTN:
			utils::string::set_clipboard_text(dialog::target_clipboard_text);
			MessageBoxA(h_dlg, "Copied to clipboard.", "Crash handler", MB_ICONINFORMATION | MB_OK);
			return TRUE;
		case IDC_EX_DLG_CLOSE_BTN:
			EndDialog(h_dlg, 0);
			return TRUE;
		}

		return FALSE;
	});

	INT_PTR result = DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_EX_DLG), nullptr, dialog::dialog_proc, 0);
	if (result == -1) {
		DWORD err = GetLastError();
		MessageBoxA(nullptr, utils::string::va("Failed to open dialog with error code %lu.", err), "Crash handler - Crash handler", MB_OK | MB_ICONERROR);
		return 1;
	}
	return 0;
}

int __stdcall WinMain(HINSTANCE h_inst, HINSTANCE, PSTR, int) {
	std::vector<std::string> c_args{};
	LPWSTR cmd_line = GetCommandLineW();

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(cmd_line, &argc);
	if (!argv) {
		MessageBoxW(nullptr, L"Failed to parse command line.", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	for (auto x = 0; x < argc; x++) {
		c_args.push_back(utils::string::convert(argv[x]));
	}

	if (c_args.size() != 2) {
		MessageBoxW(nullptr, L"Invalid command line.", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	return i_main(h_inst, c_args);
}
