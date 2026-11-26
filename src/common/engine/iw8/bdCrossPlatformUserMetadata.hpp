#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdJSONDeserializer.hpp"
#include "engine/iw8/bdJSONSerializer.hpp"
#include "engine/iw8/bdJSONValue.hpp"

namespace iw8 {
	class bdCrossPlatformUserMetadata {
	public:
		virtual ~bdCrossPlatformUserMetadata() {}
		virtual bool serialize(const char* metadata_key, bdJSONSerializer* serializer) { return false; }
		virtual bool deserialize(const char* metadata_key, bdJSONDeserializer* deserializer) { return false; }
		virtual bool validate_size() { return false; }
		virtual bool validate_key(const char* key) { return false; }
		virtual bool validate_string(const char* value) { return false; }

		bdJSONValue json_value_;
	};
	ENGINE_ASSERT_SZ(bdCrossPlatformUserMetadata, 0x0018);
}
