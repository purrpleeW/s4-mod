#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

namespace assets::reader {
	bool read_memory(std::uintptr_t address, void* output, std::size_t size);
	std::optional<std::string> read_string(const char* pointer);
	std::optional<nlohmann::json> read_json(const std::filesystem::path& file);
	bool is_camo_table_current(const void* table);

	template <typename T>
	std::optional<T> read_object(const T* address) {
		static_assert(std::is_trivially_copyable_v<T>);
		std::array<std::byte, sizeof(T)> bytes;
		if (!read_memory(reinterpret_cast<std::uintptr_t>(address), bytes.data(), bytes.size())) {
			return {};
		}

		return std::bit_cast<T>(bytes);
	}

	std::vector<std::vector<std::string>> read_table(const char* name);
}