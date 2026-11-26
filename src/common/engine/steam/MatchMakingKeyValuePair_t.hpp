#pragma once
#include "engine/engine_common.hpp"

namespace steam {
	class MatchMakingKeyValuePair_t {
	public:
		MatchMakingKeyValuePair_t() {
			ZeroMemory(this->key_, sizeof(this->key_));
			ZeroMemory(this->value_, sizeof(this->value_));
		}

		MatchMakingKeyValuePair_t(const char* key, const char* value)
			: MatchMakingKeyValuePair_t()
		{
			strncpy_s(this->key_, key, _TRUNCATE);
			strncpy_s(this->value_, value, _TRUNCATE);
		}

		char key_[256];
		char value_[256];
	};
	ENGINE_ASSERT_SZ(MatchMakingKeyValuePair_t, 0x0200);
}
