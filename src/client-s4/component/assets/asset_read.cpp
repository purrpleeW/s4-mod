#include "common.hpp"
#include "component/assets/asset_read.hpp"

#include "component/assets.hpp"
#include "game/game.hpp"

#include <algorithm>
#include <atomic>
#include <fstream>
#include <unordered_map>

namespace assets::reader {
	namespace {
		constexpr std::size_t max_string_length = 4096;
		constexpr std::size_t page_size = 4096;
		constexpr std::size_t string_chunk_size = 64;
		constexpr std::streamoff max_json_size = 64 * 1024;

		std::atomic<const void*> camo_table{};

		const std::string* read_table_cell(
			std::uint16_t index, const std::vector<const char*>& strings, std::unordered_map<std::uint16_t, std::string>& cache) {
			if (index >= strings.size()) {
				return nullptr;
			}

			if (const auto found = cache.find(index); found != cache.end()) {
				return &found->second;
			}

			const auto value = read_string(strings[index]);
			if (!value) {
				return nullptr;
			}

			return &cache.emplace(index, *value).first->second;
		}

		bool same_table_storage(const s4::StringTable& before, const s4::StringTable& after) {
			return after.cell_indices_ == before.cell_indices_ && after.strings_ == before.strings_ &&
				after.row_count_ == before.row_count_ && after.column_count_ == before.column_count_ &&
				after.unique_cell_count_ == before.unique_cell_count_ && after.name_ == before.name_;
		}
	}

	bool read_memory(std::uintptr_t address, void* output, std::size_t size) {
		if (!address || address > UINTPTR_MAX - size) {
			return false;
		}

		SIZE_T copied{};
		return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(address), output, size, &copied) && copied == size;
	}

	std::optional<std::string> read_string(const char* pointer) {
		if (!pointer) {
			return {};
		}

		std::string result;
		const auto address = reinterpret_cast<std::uintptr_t>(pointer);
		while (result.size() < max_string_length) {
			if (address > UINTPTR_MAX - result.size()) {
				return {};
			}

			const auto cursor = address + result.size();
			std::array<char, string_chunk_size> chunk;
			const auto page_remaining = page_size - (cursor & (page_size - 1));
			const auto count = std::min({ chunk.size(), page_remaining, max_string_length - result.size() });
			if (!read_memory(cursor, chunk.data(), count)) {
				return {};
			}

			const auto end = std::find(chunk.begin(), chunk.begin() + count, '\0');
			result.append(chunk.data(), static_cast<std::size_t>(end - chunk.begin()));
			if (end != chunk.begin() + count) {
				return result;
			}
		}

		return {};
	}

	std::optional<nlohmann::json> read_json(const std::filesystem::path& file) {
		std::error_code error;
		if (!std::filesystem::is_regular_file(file, error)) {
			return {};
		}

		std::ifstream stream(file, std::ios::binary | std::ios::ate);
		const auto size = stream ? stream.tellg() : std::streampos(-1);
		if (size <= 0 || size > max_json_size) {
			LOG("Component/Assets", WARN, "Rejected '{}', unreadable or over 64kb", file.string());
			return {};
		}

		std::string bytes(static_cast<std::size_t>(size), '\0');
		stream.seekg(0);
		if (!stream.read(bytes.data(), static_cast<std::streamsize>(bytes.size())) || stream.peek() != std::ifstream::traits_type::eof()) {
			LOG("Component/Assets", WARN, "Rejected '{}', file changed during read?", file.string());
			return {};
		}

		auto value = nlohmann::json::parse(bytes, nullptr, false);
		if (value.is_discarded()) {
			LOG("Component/Assets", WARN, "Rejected '{}', invalid JSON formatting?", file.string());
			return {};
		}

		return value;
	}

	std::vector<std::vector<std::string>> read_table(const char* name) {
		const auto* address = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_STRINGTABLE, name, 0).string_table_;
		const auto table = read_object(address);
		if (!table || read_string(table->name_) != std::optional<std::string>(name)) {
			return {};
		}

		if (table->column_count_ <= 0 || table->column_count_ > 64 || table->row_count_ <= 0 || table->row_count_ > 8192 ||
			table->unique_cell_count_ <= 0 || table->unique_cell_count_ > 65536) {
			return {};
		}

		const auto count = static_cast<std::size_t>(table->column_count_) * table->row_count_;
		std::vector<std::uint16_t> indices(count);
		std::vector<const char*> strings(table->unique_cell_count_);
		if (!read_memory(reinterpret_cast<std::uintptr_t>(table->cell_indices_), indices.data(), count * sizeof(indices[0])) ||
			!read_memory(reinterpret_cast<std::uintptr_t>(table->strings_), strings.data(), strings.size() * sizeof(strings[0]))) {
			return {};
		}

		std::unordered_map<std::uint16_t, std::string> cache;
		std::vector<std::vector<std::string>> rows;
		for (int row = 0; row < table->row_count_; ++row) {
			std::vector<std::string> cells(table->column_count_);
			for (const int column : { 0, 1, 3, 7, 8 }) {
				if (column >= table->column_count_) {
					continue;
				}

				const auto index = indices[static_cast<std::size_t>(row) * table->column_count_ + column];
				const auto* value = read_table_cell(index, strings, cache);
				if (!value) {
					return {};
				}

				cells[column] = *value;
			}

			rows.push_back(std::move(cells));
		}

		const auto after = read_object(address);
		if (!after || !same_table_storage(*table, *after)) {
			return {};
		}

		if (std::string_view(name) == "mp/camotable.csv") {
			camo_table = address;
		}

		return rows;
	}

	bool is_camo_table_current(const void* table) {
		return table && table == camo_table.load();
	}
}