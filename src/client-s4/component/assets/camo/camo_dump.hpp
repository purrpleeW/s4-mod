#pragma once

#include "component/assets/camo/camo_menu.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace asset_dumper::camo {

	inline std::optional<std::vector<std::size_t>> occupied_slots(
		std::span<const std::byte> bytes, std::uintptr_t base, std::size_t stride, std::uintptr_t free_head) {
		if (!base || stride < sizeof(std::uintptr_t) || bytes.empty() || bytes.size() % stride ||
			base > std::numeric_limits<std::uintptr_t>::max() - bytes.size()) {
			return {};
		}

		std::vector<bool> free(bytes.size() / stride, false);
		while (free_head) {
			if (free_head < base) {
				return {};
			}

			const auto offset = free_head - base;
			if (offset >= bytes.size() || offset % stride || free[offset / stride]) {
				return {};
			}

			free[offset / stride] = true;
			std::memcpy(&free_head, bytes.data() + offset, sizeof(free_head));
		}

		std::vector<std::size_t> result;
		for (std::size_t index = 0; index < free.size(); ++index) {
			if (!free[index]) {
				result.push_back(index);
			}
		}

		return result;
	}

	using assets::camo::menu::numeric_id;
	using assets::camo::menu::parse_row;
	using assets::camo::menu::table_entry;

	inline bool encrypted_label_is_bounded(std::string_view value) {
		return value.size() >= 3 && (static_cast<unsigned char>(value[0]) & 0xC0) == 0x80 &&
			static_cast<unsigned char>(value[2]) <= value.size() - 2;
	}
}
