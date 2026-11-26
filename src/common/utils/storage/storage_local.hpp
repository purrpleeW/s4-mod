#pragma once

#include "storage_generic_bson.hpp"

namespace utils::storage {
	namespace detail {
		struct storage_local : storage_generic_bson {
			void init() override {
				this->storage_path_ = get_game_mod_path() / "storage.bson";
				this->load();
			}
		};
	}

	inline detail::storage_local local_storage;
}