#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/vec4_t.hpp"
#include "engine/iw9/GfxColor.hpp"

namespace iw9 {
	union DvarValue {
		bool enabled_;					// 0x0000
		int integer_;					// 0x0000
		std::uint32_t unsigned_int_;	// 0x0000
		std::int64_t integer64_;		// 0x0000
		std::uint64_t unsigned_int64_;	// 0x0000
		float value_;					// 0x0000
		vec4_t vector_;					// 0x0000
		const char* string_;			// 0x0000
		GfxColor color_;				// 0x0000

		// Initializing one union member zero-initializes the storage.
		constexpr DvarValue() noexcept : unsigned_int64_(0) {}

		template <typename T>
		[[nodiscard]] static constexpr inline DvarValue create(const T v) noexcept {
			DvarValue d{};
			d.set(v);
			return d;
		}

		template <typename T>
		constexpr DvarValue& set(T v) noexcept {
			if constexpr (std::is_same_v<T, bool>) {
				this->enabled_ = v;
			}
			else if constexpr (std::is_same_v<T, int>) {
				this->integer_ = v;
			}
			else if constexpr (std::is_same_v<T, std::uint32_t>) {
				this->unsigned_int_ = v;
			}
			else if constexpr (std::is_same_v<T, std::int64_t>) {
				this->integer64_ = v;
			}
			else if constexpr (std::is_same_v<T, std::uint64_t>) {
				this->unsigned_int64_ = v;
			}
			else if constexpr (std::is_same_v<T, float>) {
				this->value_ = v;
			}
			else if constexpr (std::is_same_v<T, vec4_t>) {
				this->vector_ = v;
			}
			else if constexpr (std::is_same_v<T, const char*>) {
				this->string_ = v;
			}
			else if constexpr (std::is_same_v<T, GfxColor>) {
				this->color_ = v;
			}
			else {
				static_assert(sizeof(T) == 0, "Unsupported DvarValue type");
			}

			return *this;
		}

		[[nodiscard]] constexpr DvarValue* get_ptr() noexcept {
			return this;
		}
	};
	ENGINE_ASSERT_SZ(DvarValue, 0x0010);
}
