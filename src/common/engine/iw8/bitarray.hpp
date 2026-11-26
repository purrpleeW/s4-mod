#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	template <std::size_t BitCount>
	class bitarray {
	public:
		std::uint32_t array_[BitCount >> 5];

		void copy_bit_array(const bitarray<BitCount>& other_bit_set) {
			std::memcpy(this, &other_bit_set, sizeof(bitarray<BitCount>));
		}

		void reset_all() {
			std::memset(this->array_, 0x00, sizeof(this->array_));
		}

		void reset_bit(std::uint32_t pos) {
			this->array_[pos >> 5] &= ~((1u << 0x1F) >> (pos & 0x1F));
		}

		void set_all() {
			std::memset(this->array_, 0xFF, sizeof(this->array_));
		}

		void set_bit(std::uint32_t pos) {
			this->array_[pos >> 5] |= ((1u << 0x1F) >> (pos & 0x1F));
		}

		void set_bit_to(std::uint32_t pos, bool value) {
			if (value) {
				this->set_bit(pos);
			}
			else {
				this->reset_bit(pos);
			}
		}

		bool test_any() {
			for (auto& i : this->array_) {
				if (i != 0x00000000) {
					return true;
				}
			}
			return false;
		}

		bool test_not_any() {
			for (auto& i : this->array_) {
				if (i != 0xFFFFFFFF) {
					return true;
				}
			}
			return false;
		}

		bool test_bit(std::uint32_t pos) {
			return (this->array_[pos >> 5] & ((1u << 0x1F) >> (pos & 0x1F))) != 0;
		}
	};
}
