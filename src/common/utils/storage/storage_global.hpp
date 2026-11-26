#pragma once

#include "storage_generic_bson.hpp"

namespace utils::storage {
	namespace detail {
		struct storage_global : storage_generic_bson {
			void init() override {
				this->storage_path_ = get_appdata_path() / "storage.bson";
				this->load();
			}
		};
	}

	inline detail::storage_global global_storage;
}