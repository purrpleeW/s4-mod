#include "common.hpp"

#include "component/assets.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/properties.hpp>

namespace assets::splash {
	namespace {
		utils::hook::iat_detour load_image_a_hook;

		bool save_handle_to_bmp(HBITMAP bitmap_handle, std::filesystem::path target_path) {
			if (bitmap_handle == nullptr) {
				return false;
			}

			auto path_parent = target_path.parent_path();
			std::filesystem::create_directories(path_parent);

			BITMAP bm;
			if (!GetObjectA(bitmap_handle, sizeof(BITMAP), &bm)) {
				return false;
			}

			BITMAPINFOHEADER bi{};
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = bm.bmWidth;
			bi.biHeight = bm.bmHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 32;	// force 32-bit
			bi.biCompression = BI_RGB;

			const DWORD imageSize = bm.bmWidth * bm.bmHeight * 4;
			std::vector<BYTE> pixels(imageSize);

			HDC hdc = GetDC(nullptr);
			if (!GetDIBits(hdc, bitmap_handle, 0, bm.bmHeight, pixels.data(), PTR_AS(BITMAPINFO*, &bi), DIB_RGB_COLORS)) {
				ReleaseDC(nullptr, hdc);
				return false;
			}
			ReleaseDC(nullptr, hdc);

			BITMAPFILEHEADER bf{};
			bf.bfType = 0x4D42;	// "BM"
			bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			bf.bfSize = bf.bfOffBits + imageSize;

			std::ofstream file(target_path, std::ios::binary);
			if (!file) {
				return false;
			}

			file.write(PTR_AS(char*, &bf), sizeof(bf));
			file.write(PTR_AS(char*, &bi), sizeof(bi));
			file.write(PTR_AS(char*, pixels.data()), imageSize);

			return true;
		}

		HANDLE load_image_a_stub(HINSTANCE h_inst, LPCSTR name, UINT type, int c_x, int c_y, UINT fu_load) {
			auto res = load_image_a_hook.invoke<HANDLE>(h_inst, name, type, c_x, c_y, fu_load);

			if (RSC_VAL(name) == 0x64 /*Resource Id: 100*/) {
				while (!component_loader::is_post_unpack_phase_complete()) {
					Sleep(100);
				}

				auto splash_path = assets::get_content_path("splash.bmp");
				if (std::filesystem::exists(splash_path)) {
					return load_image_a_hook.invoke<HANDLE>(nullptr, splash_path.string().c_str(), IMAGE_BITMAP, c_x, c_y, LR_LOADFROMFILE);
				}
				else if (!save_handle_to_bmp(PTR_AS(HBITMAP, res), splash_path)) {
					LOG("Component/Assets/Splash", WARN, "Failed to extract current splash screen.");
				}
			}

			return res;
		}
	}

	struct component final : client_component {
		void post_load() override {
			load_image_a_hook.create("user32.dll", "LoadImageA", load_image_a_stub);
		}
	};
}

REGISTER_COMPONENT(assets::splash::component)
