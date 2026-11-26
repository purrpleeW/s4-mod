#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAtomic.hpp"

namespace iw8 {
	template <typename KeyType, typename ValueType, typename HashClassType>
	class bdHashMap {
	public:
		class Node {
			ValueType data_;
			const KeyType key_;
			Node* next_;
		};

		std::uint32_t size_;
		std::uint32_t capacity_;
		float load_factor_;
		std::uint32_t threshold_;
		Node** map_;
		HashClassType hash_class_;
		bdAtomic<std::uint32_t> num_iterators_;
	};
}
