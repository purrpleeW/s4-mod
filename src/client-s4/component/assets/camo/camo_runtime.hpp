#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

namespace assets::camo::runtime {

	inline bool in_pool(std::uintptr_t address, std::uintptr_t base, std::size_t stride, std::size_t count) {
		if (!base || !stride || !count || count > std::numeric_limits<std::size_t>::max() / stride) {
			return false;
		}

		const auto size = stride * count;
		return base <= std::numeric_limits<std::uintptr_t>::max() - size && address >= base && address - base < size &&
			(address - base) % stride == 0;
	}

	template <typename Camo, typename Definitions>
	bool same_asset(const Camo& current, const char* name, Definitions* defs, std::uint32_t count) {
		return current.name_ == name && current.texture_defs_ == defs && current.texture_def_count_ == count;
	}

	inline std::optional<std::size_t> cache_slot(std::uint8_t index) {
		if (index >= 1 && index <= 3) {
			return index - 1;
		}

		if (index >= 9 && index <= 11) {
			return 3 + index - 9;
		}

		if (index >= 25 && index <= 27) {
			return 6 + index - 25;
		}

		return {};
	}

	template <typename Definition>
	auto find_upload_template(const Definition* definitions, std::uint32_t count, std::uint8_t target) {
		decltype(definitions[0].image_) fallback = nullptr;

		for (std::uint32_t index = 0; index < count; ++index) {
			const auto& definition = definitions[index];
			if (definition.index_ == target) {
				return definition.image_ ? definition.image_ : fallback;
			}

			if (!fallback && definition.image_) {
				fallback = definition.image_;
			}
		}

		return fallback;
	}

	template <typename Camo>
	void set_texture_id(Camo& camo, std::size_t slot, std::uint32_t id) {
		id &= 0x3FFFFFFFu;
		if (slot < 3) {
			camo.color_texture_ids_[slot] = id;
		}
		else if (slot < 6) {
			camo.normal_texture_ids_[slot - 3] = id;
		}
		else {
			camo.gray_texture_ids_[slot - 6] = id;
		}
	}

	template <typename Camo, typename Image>
	void overlay(Camo& camo, const std::array<Image*, 9>& images) {
		for (std::size_t slot = 0; slot < images.size(); ++slot) {
			if (images[slot]) {
				set_texture_id(camo, slot, images[slot]->texture_id_);
			}
		}
	}

	template <typename Camo>
	void refresh_native_cache(Camo& camo) {
		for (std::uint32_t index = 0; index < camo.texture_def_count_; ++index) {
			const auto& def = camo.texture_defs_[index];
			if (const auto slot = cache_slot(def.index_); slot && def.image_) {
				set_texture_id(camo, *slot, def.image_->texture_id_);
			}
		}
	}
}