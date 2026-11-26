#include "common_core.hpp"
#include "kiero/kiero.hpp"
#include "utils/hook.hpp"

#if KIERO_INCLUDE_D3D9
#	include <d3d9.h>
#endif

#if KIERO_INCLUDE_D3D10
#	include <dxgi.h>
#	include <d3d10_1.h>
#	include <d3d10.h>
#endif

#if KIERO_INCLUDE_D3D11
#	include <dxgi.h>
#	include <d3d11.h>
#endif

#if KIERO_INCLUDE_D3D12
#	include <dxgi.h>
#	include <d3d12.h>
#endif

#if KIERO_INCLUDE_OPENGL
#	include <gl/GL.h>
#endif

#if KIERO_INCLUDE_VULKAN
#	include <vulkan/vulkan.h>
#endif

#ifdef _UNICODE
#	define KIERO_TEXT(text) L##text
#else
#	define KIERO_TEXT(text) text
#endif

#define KIERO_ARRAY_SIZE(arr) ((std::size_t)(sizeof(arr)/sizeof(arr[0])))

static kiero::render_type cur_render_type = kiero::render_type::none;
static std::uintptr_t* methods_table = nullptr;

static std::unordered_map<std::uintptr_t, utils::hook::detour&> detour_map;

kiero::status kiero::init(kiero::render_type _render_type) {
	if (cur_render_type != render_type::none) {
		return status::already_initialized_error;
	}

	if (_render_type != render_type::none) {
		if (_render_type >= render_type::d3d9 && _render_type <= render_type::d3d12) {
			WNDCLASSEX window_class;
			window_class.cbSize = sizeof(WNDCLASSEX);
			window_class.style = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc = DefWindowProc;
			window_class.cbClsExtra = 0;
			window_class.cbWndExtra = 0;
			window_class.hInstance = GetModuleHandle(nullptr);
			window_class.hIcon = nullptr;
			window_class.hCursor = nullptr;
			window_class.hbrBackground = nullptr;
			window_class.lpszMenuName = nullptr;
			window_class.lpszClassName = KIERO_TEXT("BNDNBANANHAHA");
			window_class.hIconSm = nullptr;
			::RegisterClassEx(&window_class);
			HWND window = ::CreateWindow(window_class.lpszClassName, KIERO_TEXT("GADHAHAEHA"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr,
				window_class.hInstance, nullptr);
			if (_render_type == render_type::d3d9) {
#				if KIERO_INCLUDE_D3D9
					using Direct3DCreate9T = LPDIRECT3D9(__stdcall*)(std::uint32_t);

					HMODULE lib_d3d9;
					if ((lib_d3d9 = ::GetModuleHandle(KIERO_TEXT("d3d9.dll"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::module_not_found_error;
					}
					Direct3DCreate9T func_direct_3d_create_9;
					if ((func_direct_3d_create_9 = PTR_AS(Direct3DCreate9T, ::GetProcAddress(lib_d3d9, "Direct3DCreate9"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					LPDIRECT3D9 direct_3d_9;
					if ((direct_3d_9 = func_direct_3d_create_9(D3D_SDK_VERSION)) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3DDISPLAYMODE display_mode;
					if (direct_3d_9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3DPRESENT_PARAMETERS params;
					params.BackBufferWidth = 0;
					params.BackBufferHeight = 0;
					params.BackBufferFormat = display_mode.Format;
					params.BackBufferCount = 0;
					params.MultiSampleType = D3DMULTISAMPLE_NONE;
					params.MultiSampleQuality = NULL;
					params.SwapEffect = D3DSWAPEFFECT_DISCARD;
					params.hDeviceWindow = window;
					params.Windowed = 1;
					params.EnableAutoDepthStencil = 0;
					params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
					params.Flags = NULL;
					params.FullScreen_RefreshRateInHz = 0;
					params.PresentationInterval = 0;
					LPDIRECT3DDEVICE9 device;
					if (direct_3d_9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &device) < 0)
					{
						direct_3d_9->Release();
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					methods_table = PTR_AS(std::uintptr_t*, ::calloc(119, sizeof(std::uintptr_t)));
					::memcpy(methods_table, DEREF_PTR_AS(std::uintptr_t*, device), 119 * sizeof(std::uintptr_t));
					direct_3d_9->Release();
					direct_3d_9 = nullptr;
					device->Release();
					device = nullptr;
					cur_render_type = render_type::d3d9;
					::DestroyWindow(window);
					::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
					return status::success;
#				endif
			}
			else if (_render_type == render_type::d3d10) {
#				if KIERO_INCLUDE_D3D10
					using CreateDXGIFactoryT = long(__stdcall*)(const IID&, void**);
					using D3D10CreateDeviceAndSwapChainT = long(__stdcall*)(IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC*,
						IDXGISwapChain**, ID3D10Device**);

					HMODULE lib_dxgi;
					HMODULE lib_d3d10;
					if ((lib_dxgi = ::GetModuleHandle(KIERO_TEXT("dxgi.dll"))) == nullptr
						|| (lib_d3d10 = ::GetModuleHandle(KIERO_TEXT("d3d10.dll"))) == nullptr)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::module_not_found_error;
					}
					CreateDXGIFactoryT func_create_dxgi_factory;
					if ((func_create_dxgi_factory = PTR_AS(CreateDXGIFactoryT, ::GetProcAddress(lib_dxgi, "CreateDXGIFactory"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					IDXGIFactory* factory;
					if (func_create_dxgi_factory(IID_PPV_ARGS(&factory)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					IDXGIAdapter* adapter;
					if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3D10CreateDeviceAndSwapChainT func_d3d10_create_device_and_swap_chain;
					if ((func_d3d10_create_device_and_swap_chain = PTR_AS(D3D10CreateDeviceAndSwapChainT,
						::GetProcAddress(lib_d3d10, "D3D10CreateDeviceAndSwapChain"))) == nullptr)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					DXGI_RATIONAL refresh_rate;
					refresh_rate.Numerator = 60;
					refresh_rate.Denominator = 1;
					DXGI_MODE_DESC buffer_desc;
					buffer_desc.Width = 100;
					buffer_desc.Height = 100;
					buffer_desc.RefreshRate = refresh_rate;
					buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
					buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					DXGI_SAMPLE_DESC sample_desc;
					sample_desc.Count = 1;
					sample_desc.Quality = 0;
					DXGI_SWAP_CHAIN_DESC swap_chain_desc;
					swap_chain_desc.BufferDesc = buffer_desc;
					swap_chain_desc.SampleDesc = sample_desc;
					swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					swap_chain_desc.BufferCount = 1;
					swap_chain_desc.OutputWindow = window;
					swap_chain_desc.Windowed = 1;
					swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
					swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
					IDXGISwapChain* swap_chain;
					ID3D10Device* device;
					if (func_d3d10_create_device_and_swap_chain(adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D10_SDK_VERSION, &swap_chain_desc,
						&swap_chain, &device) < 0)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					methods_table = PTR_AS(std::uintptr_t*, ::calloc(116, sizeof(std::uintptr_t)));
					::memcpy(methods_table, DEREF_PTR_AS(std::uintptr_t*, swap_chain), 18 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 18, DEREF_PTR_AS(std::uintptr_t*, device), 98 * sizeof(std::uintptr_t));
					swap_chain->Release();
					swap_chain = nullptr;
					device->Release();
					device = nullptr;
					::DestroyWindow(window);
					::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
					cur_render_type = render_type::d3d10;
					return status::success;
#				endif
			}
			else if (_render_type == render_type::d3d11) {
#				if KIERO_INCLUDE_D3D11
					using D3D11CreateDeviceAndSwapChainT = long(__stdcall*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT,
						UINT, const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);

					HMODULE lib_d3d11;
					if ((lib_d3d11 = ::GetModuleHandle(KIERO_TEXT("d3d11.dll"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::module_not_found_error;
					}
					D3D11CreateDeviceAndSwapChainT func_d3d11_create_device_and_swap_chain;
					if ((func_d3d11_create_device_and_swap_chain = PTR_AS(D3D11CreateDeviceAndSwapChainT,
						::GetProcAddress(lib_d3d11, "D3D11CreateDeviceAndSwapChain"))) == nullptr)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3D_FEATURE_LEVEL feature_level;
					const D3D_FEATURE_LEVEL feature_levels[] = {
						D3D_FEATURE_LEVEL_10_1,
						D3D_FEATURE_LEVEL_11_0
					};
					DXGI_RATIONAL refresh_rate;
					refresh_rate.Numerator = 60;
					refresh_rate.Denominator = 1;
					DXGI_MODE_DESC buffer_desc;
					buffer_desc.Width = 100;
					buffer_desc.Height = 100;
					buffer_desc.RefreshRate = refresh_rate;
					buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
					buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					DXGI_SAMPLE_DESC sample_desc;
					sample_desc.Count = 1;
					sample_desc.Quality = 0;
					DXGI_SWAP_CHAIN_DESC swap_chain_desc;
					swap_chain_desc.BufferDesc = buffer_desc;
					swap_chain_desc.SampleDesc = sample_desc;
					swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					swap_chain_desc.BufferCount = 1;
					swap_chain_desc.OutputWindow = window;
					swap_chain_desc.Windowed = 1;
					swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
					swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
					IDXGISwapChain* swap_chain;
					ID3D11Device* device;
					ID3D11DeviceContext* context;
					if (func_d3d11_create_device_and_swap_chain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, feature_levels, 1, D3D11_SDK_VERSION,
						&swap_chain_desc, &swap_chain, &device, &feature_level, &context) < 0)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					methods_table = PTR_AS(std::uintptr_t*, ::calloc(205, sizeof(std::uintptr_t)));
					::memcpy(methods_table, DEREF_PTR_AS(std::uintptr_t*, swap_chain), 18 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 18, DEREF_PTR_AS(std::uintptr_t*, device), 43 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 18 + 43, DEREF_PTR_AS(std::uintptr_t*, context), 144 * sizeof(std::uintptr_t));
					swap_chain->Release();
					swap_chain = nullptr;
					device->Release();
					device = nullptr;
					context->Release();
					context = nullptr;
					::DestroyWindow(window);
					::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
					cur_render_type = render_type::d3d11;
					return status::success;
#				endif
			}
			else if (_render_type == render_type::d3d12) {
#				if KIERO_INCLUDE_D3D12
					using CreateDXGIFactoryT = long(__stdcall*)(const IID&, void**);
					using D3D12CreateDeviceT = long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**);

					HMODULE lib_dxgi;
					HMODULE lib_d3d12;
					if ((lib_dxgi = ::GetModuleHandle(KIERO_TEXT("dxgi.dll"))) == nullptr
						|| (lib_d3d12 = ::GetModuleHandle(KIERO_TEXT("d3d12.dll"))) == nullptr)
					{
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::module_not_found_error;
					}
					CreateDXGIFactoryT func_create_dxgi_factory;
					if ((func_create_dxgi_factory = PTR_AS(CreateDXGIFactoryT, ::GetProcAddress(lib_dxgi, "CreateDXGIFactory"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					IDXGIFactory* factory;
					if (func_create_dxgi_factory(IID_PPV_ARGS(&factory)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					IDXGIAdapter* adapter;
					if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3D12CreateDeviceT D3D12CreateDevice;
					if ((D3D12CreateDevice = PTR_AS(D3D12CreateDeviceT, ::GetProcAddress(lib_d3d12, "D3D12CreateDevice"))) == nullptr) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					ID3D12Device* device;
					if (D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					D3D12_COMMAND_QUEUE_DESC queueDesc;
					queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
					queueDesc.Priority = 0;
					queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
					queueDesc.NodeMask = 0;
					ID3D12CommandQueue* command_queue;
					if (device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					ID3D12CommandAllocator* command_allocator;
					if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					ID3D12GraphicsCommandList* command_list;
					if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr, IID_PPV_ARGS(&command_list)) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					DXGI_RATIONAL refresh_rate;
					refresh_rate.Numerator = 60;
					refresh_rate.Denominator = 1;
					DXGI_MODE_DESC buffer_desc;
					buffer_desc.Width = 100;
					buffer_desc.Height = 100;
					buffer_desc.RefreshRate = refresh_rate;
					buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
					buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					DXGI_SAMPLE_DESC sample_desc;
					sample_desc.Count = 1;
					sample_desc.Quality = 0;
					DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
					swap_chain_desc.BufferDesc = buffer_desc;
					swap_chain_desc.SampleDesc = sample_desc;
					swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					swap_chain_desc.BufferCount = 2;
					swap_chain_desc.OutputWindow = window;
					swap_chain_desc.Windowed = 1;
					swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
					swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
					IDXGISwapChain* swap_chain;
					if (factory->CreateSwapChain(command_queue, &swap_chain_desc, &swap_chain) < 0) {
						::DestroyWindow(window);
						::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
						return status::unknown_error;
					}
					methods_table = PTR_AS(std::uintptr_t*, ::calloc(150, sizeof(std::uintptr_t)));
					::memcpy(methods_table, DEREF_PTR_AS(std::uintptr_t*, device), 44 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 44, DEREF_PTR_AS(std::uintptr_t*, command_queue), 19 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 44 + 19, DEREF_PTR_AS(std::uintptr_t*, command_allocator), 9 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 44 + 19 + 9, DEREF_PTR_AS(std::uintptr_t*, command_list), 60 * sizeof(std::uintptr_t));
					::memcpy(methods_table + 44 + 19 + 9 + 60, DEREF_PTR_AS(std::uintptr_t*, swap_chain), 18 * sizeof(std::uintptr_t));
					cur_render_type = render_type::d3d12;
					return status::success;
#				endif
			}
			::DestroyWindow(window);
			::UnregisterClass(window_class.lpszClassName, window_class.hInstance);
			return status::not_supported_error;
		}
		else if (_render_type == render_type::open_gl) {
#			if KIERO_INCLUDE_OPENGL
				HMODULE lib_open_gl_32;
				if ((lib_open_gl_32 = ::GetModuleHandle(KIERO_TEXT("opengl32.dll"))) == nullptr) {
					return status::module_not_found_error;
				}
				std::size_t size = KIERO_ARRAY_SIZE(gl_funcs);
				methods_table = PTR_AS(std::uintptr_t*, ::calloc(size, sizeof(std::uintptr_t)));
				for (auto x = 0; x < size; x++) {
					methods_table[x] = PTR_AS(std::uintptr_t, ::GetProcAddress(lib_open_gl_32, gl_funcs[x]));
				}
				cur_render_type = render_type::open_gl;
				return status::success;
#			endif
		}
		else if (_render_type == render_type::vulkan) {
#			if KIERO_INCLUDE_VULKAN
				HMODULE lib_vulkan;
				if ((lib_vulkan = GetModuleHandle(KIERO_TEXT("vulkan-1.dll"))) == nullptr) {
					return status::module_not_found_error;
				}
				std::size_t size = KIERO_ARRAY_SIZE(vulkan_funcs);
				methods_table = PTR_AS(std::uintptr_t*, ::calloc(size, sizeof(std::uintptr_t)));
				for (auto x = 0; x < size; x++) {
					methods_table[x] = PTR_AS(std::uintptr_t, ::GetProcAddress(lib_vulkan, vulkan_funcs[x]));
				}
				cur_render_type = render_type::vulkan;
				return status::success;
#			endif
		}
		else if (_render_type == render_type::auto_select) {
			render_type type = render_type::none;
			if (::GetModuleHandle(KIERO_TEXT("d3d9.dll")) != nullptr) {
				type = render_type::d3d9;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d10.dll")) != nullptr) {
				type = render_type::d3d10;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d11.dll")) != nullptr) {
				type = render_type::d3d11;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d12.dll")) != nullptr) {
				type = render_type::d3d12;
			}
			else if (::GetModuleHandle(KIERO_TEXT("opengl32.dll")) != nullptr) {
				type = render_type::open_gl;
			}
			else if (::GetModuleHandle(KIERO_TEXT("vulkan-1.dll")) != nullptr) {
				type = render_type::vulkan;
			}
			return init(type);
		}
	}
	return status::success;
}

void kiero::shutdown() {
	if (cur_render_type != render_type::none) {
		for (auto& [target, detour] : detour_map) {
			detour.clear();
		}
		detour_map.clear();
		::free(methods_table);
		methods_table = nullptr;
		cur_render_type = render_type::none;
	}
}

kiero::status kiero::bind(std::uint16_t _index, void** _original, void* _function) {
	assert(_index >= 0 && _original != nullptr && _function != nullptr);
	if (cur_render_type != render_type::none) {
		unbind(_index);
		utils::hook::detour new_detour;
		new_detour.create(methods_table[_index], _function);
		if (_original != nullptr) {
			*_original = new_detour.get_original();
		}
		detour_map.emplace(methods_table[_index], new_detour);
		return status::success;
	}
	return status::not_initialized_error;
}

void kiero::unbind(std::uint16_t _index) {
	assert(_index >= 0);
	if (cur_render_type != render_type::none) {
		auto it = detour_map.find(methods_table[_index]);
		if (it == detour_map.end()) {
			return;
		}
		it->second.clear();
		detour_map.erase(it);
	}
}

kiero::render_type kiero::get_render_type() {
	return cur_render_type;
}

std::uintptr_t* kiero::get_methods_table() {
	return methods_table;
}
