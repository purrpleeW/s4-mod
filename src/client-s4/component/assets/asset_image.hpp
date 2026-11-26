#pragma once
#include "engine/s4/GfxImage.hpp"

#include <filesystem>
#include <string>

namespace assets::image {
	inline constexpr const char* supported_exts[] = { "gif", "jpg", "png", "jpeg" };

	bool available();
	s4::GfxImage* load_custom_texture(s4::GfxImage* original, const std::string& name, const std::filesystem::path& file, bool linear = false);
	void animate();
	void stop_animations();
	bool find_image_file(const std::filesystem::path& stem, std::filesystem::path& result);
}