#pragma once
#include "common_core.hpp"
#include "logger/logger.hpp"
#include "memory/scanned_result.hpp"
#include "utils/nt.hpp"

namespace memory {
	struct masked_signature {
		std::vector<std::uint8_t> data_{};
		std::vector<std::uint8_t> mask_{};
	};

	inline masked_signature masked_signature_from_string(const std::string& pattern) {
		const auto astroul = [](char c) -> std::uint8_t {
			if (c >= '0' && c <= '9') {
				return c - '0';
			}

			if (c >= 'A' && c <= 'F') {
				return 10 + (c - 'A');
			}

			if (c >= 'a' && c <= 'f') {
				return 10 + (c - 'a');
			}

			return 0;
		};

		masked_signature sig{};
		for (std::size_t ix = 0, len = pattern.length(); ix < len; ix++) {
			if (pattern[ix] == ' ') {
				continue;
			}

			char h1 = pattern[ix];
			char h2 = (ix + 1 < len && pattern[ix + 1] != ' ') ? pattern[ix + 1] : '?';

			if (h1 == '?' && h2 == '?') {
				sig.data_.push_back(0x00);
				sig.mask_.push_back(0x00); // full wildcard
			}
			else if (h1 != '?' && h2 == '?') {
				sig.data_.push_back(astroul(h1) << 4);
				sig.mask_.push_back(0xF0); // high nibble exact, low wildcard
			}
			else if (h1 == '?' && h2 != '?') {
				sig.data_.push_back(astroul(h2));
				sig.mask_.push_back(0x0F); // low nibble exact, high wildcard
			}
			else if (h1 != '?' && h2 != '?') {
				sig.data_.push_back((astroul(h1) << 4) | astroul(h2));
				sig.mask_.push_back(0xFF); // exact match
			}

			ix++; // do the funny
		}
		return sig;
	}

	inline bool matches(void* data, const masked_signature& signature) {
		auto size = signature.data_.size();
		if (size != signature.mask_.size()) {
			return false;	// size discrepency
		}
		for (std::size_t jx = 0; jx < size; jx++) {
			auto byte = DEREF_PTR_AS(std::uint8_t, PTR_AS(std::uintptr_t, data) + jx);
			if ((byte & signature.mask_[jx]) != signature.data_[jx]) {
				return false;
			}
		}
		return true;
	}

	inline bool matches(std::uintptr_t address, const masked_signature& signature) {
		return matches(PTR_AS(void*, address), signature);
	}

	inline bool matches(void* data, const std::string& pattern) {
		return matches(data, masked_signature_from_string(pattern));
	}

	inline bool matches(std::uintptr_t address, const std::string& pattern) {
		return matches(address, masked_signature_from_string(pattern));
	}

	template <typename T = void*>
	std::enable_if_t<std::is_pointer_v<T>, std::vector<scanned_result<std::remove_pointer_t<T>>>> masked_vectored_sig_scan(utils::nt::library library,
		masked_signature sig, std::size_t limit)
	{
		std::vector<scanned_result<std::remove_pointer_t<T>>> results{};
		for (auto page : library.get_accessible_pages()) {
			for (auto addr = page.begin_; addr < page.end_ - sig.mask_.size(); addr++) {
				if (!matches(addr, sig)) {
					continue;
				}
				results.push_back({ addr });
				if (results.size() >= limit) {
					return results;
				}
			}
		}
		return results;
	}

	template <typename T = void*>
	std::enable_if_t<std::is_pointer_v<T>, scanned_result<std::remove_pointer_t<T>>> masked_sig_scan(utils::nt::library library, masked_signature sig) {
		std::vector<scanned_result<std::remove_pointer_t<T>>> result = masked_vectored_sig_scan<T>(library, sig, 1);
		if (result.size() > 0) {
			return result.at(0);
		}
		return scanned_result<std::remove_pointer_t<T>>(nullptr);
	}

	template <typename T = void*>
	std::enable_if_t<std::is_pointer_v<T>, std::vector<scanned_result<std::remove_pointer_t<T>>>> vectored_sig_scan(utils::nt::library library,
		std::string pattern, std::size_t limit, std::string name = "", bool print_fail = true)
	{
		std::vector<scanned_result<std::remove_pointer_t<T>>> res = masked_vectored_sig_scan<T>(library, masked_signature_from_string(pattern), limit);
		if (!name.empty()) {
			if (res.size() > 0) {
				LOG("Scanner", DEBUG, "Found '{}' {}+0x{:X} [{} total]", name, library.get_name(), res.at(0).template as<std::uintptr_t>()
					- PTR_AS(std::uintptr_t, library.get_info().lpBaseOfDll), res.size());
			}
			else if (print_fail) {
				LOG("Scanner", WARN, "Failed to find '{}' in {} ({})", name, library.get_name(), pattern);
			}
		}
		return res;
	}

	template <typename T = void*>
	std::enable_if_t<std::is_pointer_v<T>, scanned_result<std::remove_pointer_t<T>>> sig_scan(utils::nt::library library, std::string pattern,
		std::string name = "", bool print_fail = true)
	{
		scanned_result<std::remove_pointer_t<T>> res = masked_sig_scan<T>(library, masked_signature_from_string(pattern));
		if (!name.empty()) {
			if (res) {
				LOG("Scanner", DEBUG, "Found '{}' {}+0x{:X}", name, library.get_name(), res.template as<std::uintptr_t>()
					- PTR_AS(std::uintptr_t, library.get_info().lpBaseOfDll));
			}
			else if (print_fail) {
				LOG("Scanner", WARN, "Failed to find '{}' in {} ({})", name, library.get_name(), pattern);
			}
		}
		return res;
	}

	template <typename T = void*>
	std::enable_if_t<std::is_pointer_v<T>, scanned_result<std::remove_pointer_t<T>>> get_export(const std::string& module_name, const std::string& export_name) {
		utils::nt::library library(module_name);
		if (!library.is_valid()) {
			LOG("Scanner", WARN, "Failed to find module '{}' while trying to find '{}'", export_name, module_name);
			return scanned_result<std::remove_pointer_t<T>>(nullptr);
		}

		scanned_result<std::remove_pointer_t<T>> res = scanned_result<std::remove_pointer_t<T>>(library.get_proc<void*>(export_name));
		if (res) {
			LOG("Scanner", DEBUG, "Found '{}' {}+0x{:X}", export_name, module_name, res.template as<std::uintptr_t>()
				- PTR_AS(std::uintptr_t, library.get_info().lpBaseOfDll));
		}
		else {
			LOG("Scanner", WARN, "Failed to find '{}' in {}", export_name, module_name);
		}
		return res;
	}
}