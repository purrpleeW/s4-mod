#pragma once
#include "engine/engine_common.hpp"

namespace ntl {
	template <std::size_t Bits, std::size_t Offset, typename Word>
	class bitset {
	public:
		static constexpr std::size_t bits_per_word = sizeof(Word) * 8;
		static constexpr std::size_t word_count = (Bits + bits_per_word - 1) / bits_per_word;

		Word data_[word_count];
	};
}
