#include "common.hpp"
#include "component/assets/asset_image.hpp"
#include "component/assets/image_validation.hpp"
#include "game/game.hpp"

#include <engine/s4/Image_SetupParams.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/image.hpp>
#include <stb_image.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace assets::image {
	namespace {
		using Image_SetupInternalT = void(s4::GfxImage*, s4::Image_SetupParams*, s4::Image_SetupData*);
		using DB_GetGfxImageIndexT = std::uint32_t(s4::GfxImage*);

		Image_SetupInternalT* Image_SetupInternal{};
		DB_GetGfxImageIndexT* DB_GetGfxImageIndex{};
		utils::hook::detour image_index_hook;

		std::mutex index_mutex;
		std::unordered_map<s4::GfxImage*, s4::GfxImage*> image_origins;
		bool enabled = false;

		using namespace validation;
		constexpr std::size_t max_retained_size = 512 * 1024 * 1024;
		constexpr std::size_t max_textures = 1024;
		constexpr std::uint32_t single_2d_map_mask = 0x38000;
		constexpr std::uint32_t unstreamed_image_flags = 3; // NOPICMIP | NOMIPMAPS.
		constexpr std::uint32_t rgba8_format = 6;
		constexpr std::uint32_t rgba8_srgb_format = 7;
		constexpr std::uint32_t native_resource_flags = 0x8C0;

		std::size_t retained_size = 0;
		std::size_t texture_count = 0;

		struct uploaded_image {
			std::string name;
			std::string source;
			utils::image::image pixels;
			std::vector<s4::GfxImage> frames;
			std::vector<std::chrono::milliseconds> frame_ends;
			s4::GfxImage display{};
			std::chrono::steady_clock::time_point start;
			bool linear{};
			bool active{};
			bool ready{};
		};

		using image_versions = std::vector<std::unique_ptr<uploaded_image>>;

		std::unordered_map<std::string, image_versions> images;

		bool read_source(const std::filesystem::path& file, std::string& bytes) {
			std::ifstream stream(file, std::ios::binary | std::ios::ate);
			if (!stream) {
				return false;
			}

			const auto size = stream.tellg();
			if (size <= 0 || size > static_cast<std::streamoff>(max_file_size)) {
				return false;
			}

			bytes.resize(static_cast<std::size_t>(size));
			stream.seekg(0);
			// reject files without allocating their size
			return static_cast<bool>(stream.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))) &&
				stream.peek() == std::ifstream::traits_type::eof();
		}

		s4::GfxImage* resolve_original_image(s4::GfxImage* image) {
			std::lock_guard lock(index_mutex);
			while (true) {
				const auto it = image_origins.find(image);
				if (it == image_origins.end()) {
					return image;
				}

				image = it->second;
			}
		}

		std::uint32_t image_index_stub(s4::GfxImage* image) {
			return image_index_hook.invoke<std::uint32_t>(resolve_original_image(image));
		}

		bool validate_source(const std::string& bytes, bool gif) {
			int width{}, height{}, channels{};
			if (bytes.empty() || bytes.size() > max_file_size ||
				!stbi_info_from_memory(
					reinterpret_cast<const unsigned char*>(bytes.data()), static_cast<int>(bytes.size()), &width, &height, &channels) ||
				!rgba_frame_size(width, height)) {
				return false;
			}

			const auto frame_size = static_cast<std::size_t>(width) * height * 4;
			return !gif || validate_gif(bytes, frame_size);
		}

		bool is_pooled_image(const s4::GfxImage* image) {
			if (!game::s_assetPools) {
				return false;
			}

			const auto& pool = game::s_assetPools[ENUM_UNDER(s4::XAssetType::ASSET_TYPE_IMAGE)];
			const auto address = reinterpret_cast<std::uintptr_t>(image);
			const auto begin = reinterpret_cast<std::uintptr_t>(pool.entries_);
			if (!begin || pool.element_size_ != sizeof(s4::GfxImage) || address < begin ||
				address - begin >= static_cast<std::size_t>(pool.pool_size_) * pool.element_size_ ||
				(address - begin) % pool.element_size_) {
				return false;
			}

			return image->name_ != nullptr;
		}

		void deactivate_versions(image_versions& versions) {
			for (auto& version : versions) {
				version->active = false;
			}
		}

		uploaded_image* find_cached_upload(image_versions& versions, const std::string& source, bool linear, const s4::GfxImage& original) {
			for (auto& cached : versions) {
				if (!cached->ready || cached->source != source || cached->linear != linear ||
					cached->display.semantic_category_ != original.semantic_category_) {
					continue;
				}

				return cached.get();
			}

			return nullptr;
		}

		bool has_upload_budget(const utils::image::image& decoded, std::size_t source_size) {
			const auto frame_size = decoded.width * decoded.height * 4;
			return frame_size && decoded.frames > 0 && decoded.frames <= max_frames && decoded.data.size() == frame_size * decoded.frames &&
				decoded.data.size() <= max_decoded_size && decoded.frames <= max_textures - texture_count &&
				decoded.data.size() + source_size <= max_retained_size - retained_size;
		}

		uploaded_image& retain_upload(image_versions& versions, std::unique_ptr<uploaded_image> record, s4::GfxImage* original) {
			versions.reserve(versions.size() + 1);
			{
				std::lock_guard lock(index_mutex);
				image_origins.emplace(&record->display, original);
			}

			deactivate_versions(versions);
			versions.push_back(std::move(record));

			auto& uploaded = *versions.back();
			retained_size += uploaded.pixels.data.size() + uploaded.source.size();
			texture_count += uploaded.pixels.frames;
			return uploaded;
		}

		s4::Image_SetupParams make_setup_params(const uploaded_image& record) {
			s4::Image_SetupParams params{};
			params.width_ = static_cast<int>(record.pixels.width);
			params.height_ = static_cast<int>(record.pixels.height);
			params.depth_ = 1;
			params.num_elements_ = 1;
			params.max_level_count_ = 1;
			params.flags_ = unstreamed_image_flags;
			params.format_ = record.linear ? rgba8_format : rgba8_srgb_format;
			params.resource_flags_ = native_resource_flags;
			return params;
		}

		void upload(uploaded_image& record, const s4::GfxImage& original) {
			const auto frame_size = record.pixels.width * record.pixels.height * 4;
			auto data = std::make_unique<s4::Image_SetupData>();
			record.frames.resize(record.pixels.frames);
			record.frame_ends.reserve(record.frames.size());
			std::chrono::milliseconds elapsed{};
			for (std::size_t index = 0; index < record.frames.size(); ++index) {
				auto& frame = record.frames[index];
				frame.name_ = record.name.c_str();
				frame.semantic_category_ = original.semantic_category_;

				auto params = make_setup_params(record);
				data->data_[0][0] = reinterpret_cast<const std::uint8_t*>(record.pixels.data.data() + index * frame_size);
				Image_SetupInternal(&frame, &params, data.get());
				const auto delay = index < record.pixels.frame_delays.size() ? record.pixels.frame_delays[index] : 100;
				elapsed += std::chrono::milliseconds(std::max(delay, 10));
				record.frame_ends.push_back(elapsed);
			}

			record.display = record.frames.front();
			record.start = std::chrono::steady_clock::now();
			record.active = true;
			record.ready = true;
		}

		s4::GfxImage* load_texture(s4::GfxImage* original, const std::string& name, const std::filesystem::path& file, bool linear) {
			std::string source;
			if (!read_source(file, source)) {
				LOG("Component/Assets/Image", WARN, "Couldn't read '{}' (empty or changed?)", file.string());
				return nullptr;
			}

			auto& versions = images[name];
			if (auto* cached = find_cached_upload(versions, source, linear, *original)) {
				deactivate_versions(versions);
				cached->active = true;

				std::lock_guard lock(index_mutex);
				image_origins[&cached->display] = original;
				return &cached->display;
			}

			const bool gif = validation::is_gif(source);
			if (!validate_source(source, gif)) {
				LOG("Component/Assets/Image", WARN, "Rejected image '{}', invalid image or size/frame limits were exceeded", file.string());
				return nullptr;
			}

			auto record = std::make_unique<uploaded_image>();
			record->pixels = utils::image::load_image(source, gif);
			if (!has_upload_budget(record->pixels, source.size())) {
				LOG("Component/Assets/Image", WARN, "Image budget exceeded for '{}', restart the game to fix?",
					file.string());
				return nullptr;
			}

			record->name = name;
			record->source = std::move(source);
			record->linear = linear;

			auto& uploaded = retain_upload(versions, std::move(record), original);
			upload(uploaded, *original);

			return &uploaded.display;
		}

		void animate_upload(uploaded_image& image, std::chrono::steady_clock::time_point now) {
			if (!image.active || image.frames.size() < 2) {
				return;
			}

			const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - image.start);
			const auto phase = std::chrono::milliseconds(elapsed.count() % image.frame_ends.back().count());
			const auto frame = std::upper_bound(image.frame_ends.begin(), image.frame_ends.end(), phase) - image.frame_ends.begin();
			image.display.texture_id_ = image.frames[frame].texture_id_;
		}
	}

	bool available() {
		return enabled;
	}

	bool find_image_file(const std::filesystem::path& stem, std::filesystem::path& result) {
		for (const auto* ext : supported_exts) {
			auto path = stem;
			path += std::format(".{}", ext);
			std::error_code error;
			if (std::filesystem::is_regular_file(path, error)) {
				result = std::move(path);
				return true;
			}
		}

		return false;
	}

	s4::GfxImage* load_custom_texture(s4::GfxImage* original, const std::string& name, const std::filesystem::path& file, bool linear) {
		if (!enabled || !original) {
			return nullptr;
		}

		original = resolve_original_image(original);
		if (!is_pooled_image(original)) {
			return nullptr;
		}

		if ((original->flags_ & single_2d_map_mask) || original->depth_ != 1 || original->num_elements_ != 1) {
			LOG("Component/Assets/Image", WARN, "Skipping '{}', original image is not a single 2D texture!", file.string());
			return nullptr;
		}

		try {
			return load_texture(original, name, file, linear);
		}
		catch (const std::exception& error) {
			LOG("Component/Assets/Image", WARN, "Failed to load '{}', error: {}", file.string(), error.what());
			return nullptr;
		}
	}

	void stop_animations() {
		for (auto& [name, versions] : images) {
			deactivate_versions(versions);
		}
	}

	void animate() {
		const auto now = std::chrono::steady_clock::now();
		for (auto& [name, versions] : images) {
			for (auto& image : versions) {
				animate_upload(*image, now);
			}
		}
	}

	class component final : public client_component {
	public:
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Image_SetupInternal),
				"48 83 EC 68 C5 FC 10 02 C5 F8 10 4A 20 C5 FC 11 44 24 20 C5 FB 10 42 30 C5 FB 11 44 24 50 C5 F8 "
					"11 4C 24 40 48 8D 54 24 20 C5 F8 77 E8", SETUP_MOD(add(45).rip()));

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(DB_GetGfxImageIndex), "E8 ? ? ? ? 48 8B CF 8B F0 E8 ? ? ? ? 44 8B 47 14 41 C1 E8 06", SETUP_MOD(add(1).rip()));
			}
			else {
				batch.add(SETUP_POINTER(DB_GetGfxImageIndex), "E8 ? ? ? ? 48 8B 43 40 48 8B 08 48 8B 49 20", SETUP_MOD(add(1).rip()));
			}
		}

		void post_unpack() override {
			if (!Image_SetupInternal || !DB_GetGfxImageIndex) {
				LOG("Component/Assets/Image", WARN, "Custom images disabled, missing sigs!");
				return;
			}

			image_index_hook.create(DB_GetGfxImageIndex, image_index_stub);
			enabled = true;
		}
	};
}

REGISTER_COMPONENT(assets::image::component)
