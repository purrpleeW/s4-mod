#pragma once
#include "common_core.hpp"
#include <bitset>

namespace utils {
	//$ https://voithos.io/articles/type-safe-enum-class-bit-flags/

	template <typename T>
	class bit_flags {
		using underlying_t = std::underlying_type_t<T>;

	public:
		constexpr bit_flags() : flags_(static_cast<underlying_t>(0)) {}
		constexpr explicit bit_flags(T v) : flags_(to_underlying(v)) {}
		constexpr bit_flags(std::initializer_list<T> vs) : bit_flags() {
			for (T v : vs) {
				flags_ |= to_underlying(v);
			}
		}

		constexpr bool is_set(T v) const {
			return (flags_ & to_underlying(v)) == to_underlying(v);
		}

		constexpr void set(T v) {
			flags_ |= to_underlying(v);
		}

		constexpr void unset(T v) {
			flags_ &= ~to_underlying(v);
		}

		constexpr void clear() {
			flags_ = static_cast<underlying_t>(0);
		}

		constexpr operator bool() const {
			return flags_ != static_cast<underlying_t>(0);
		}

		friend constexpr bit_flags operator|(bit_flags lhs, T rhs) {
			return bit_flags(lhs.flags_ | to_underlying(rhs));
		}
		friend constexpr bit_flags operator|(bit_flags lhs, bit_flags rhs) {
			return bit_flags(lhs.flags_ | rhs.flags_);
		}
		friend constexpr bit_flags operator&(bit_flags lhs, T rhs) {
			return bit_flags(lhs.flags_ & to_underlying(rhs));
		}
		friend constexpr bit_flags operator&(bit_flags lhs, bit_flags rhs) {
			return bit_flags(lhs.flags_ & rhs.flags_);
		}
		friend constexpr bit_flags operator^(bit_flags lhs, T rhs) {
			return bit_flags(lhs.flags_ ^ to_underlying(rhs));
		}
		friend constexpr bit_flags operator^(bit_flags lhs, bit_flags rhs) {
			return bit_flags(lhs.flags_ ^ rhs.flags_);
		}

		friend constexpr bit_flags& operator|=(bit_flags& lhs, T rhs) {
			lhs.flags_ |= to_underlying(rhs);
			return lhs;
		}
		friend constexpr bit_flags& operator|=(bit_flags& lhs, bit_flags rhs) {
			lhs.flags_ |= rhs.flags_;
			return lhs;
		}
		friend constexpr bit_flags& operator&=(bit_flags& lhs, T rhs) {
			lhs.flags_ &= to_underlying(rhs);
			return lhs;
		}
		friend constexpr bit_flags& operator&=(bit_flags& lhs, bit_flags rhs) {
			lhs.flags_ &= rhs.flags_;
			return lhs;
		}
		friend constexpr bit_flags& operator^=(bit_flags& lhs, T rhs) {
			lhs.flags_ ^= to_underlying(rhs);
			return lhs;
		}
		friend constexpr bit_flags& operator^=(bit_flags& lhs, bit_flags rhs) {
			lhs.flags_ ^= rhs.flags_;
			return lhs;
		}

		friend constexpr bit_flags operator~(const bit_flags& bf) {
			return bit_flags(~bf.flags_);
		}

		friend constexpr bool operator==(const bit_flags& lhs, const bit_flags& rhs) {
			return lhs.flags_ == rhs.flags_;
		}
		friend constexpr bool operator!=(const bit_flags& lhs, const bit_flags& rhs) {
			return lhs.flags_ != rhs.flags_;
		}

		// Stream output operator for debugging.
		friend std::ostream& operator<<(std::ostream& os, const bit_flags& bf) {
			os << std::bitset<sizeof(underlying_t) * 8>(bf.flags_);
			return os;
		}

		// Construct BitFlags from raw values.
		static constexpr bit_flags from_raw(underlying_t flags) {
			return bit_flags(flags);
		}

		// Retrieve the raw underlying flags.
		constexpr underlying_t to_raw() const { return flags_; }

	private:
		constexpr explicit bit_flags(underlying_t flags) : flags_(flags) {}
		static constexpr underlying_t to_underlying(T v) { return static_cast<underlying_t>(v); }
		underlying_t flags_;
	};
}