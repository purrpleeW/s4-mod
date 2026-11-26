#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/enums/EResult.hpp"

namespace steam {
	class ISteamUnifiedMessages {
	public:
		static const ClientUnifiedMessageHandle k_InvalidUnifiedMessageHandle = 0;

		virtual ClientUnifiedMessageHandle SendMethod(const char* service_method, const void* request_buffer, std::uint32_t request_buffer_size,
			std::uint64_t context) = 0;
		virtual bool GetMethodResponseInfo(ClientUnifiedMessageHandle handle, std::uint32_t* response_size, EResult* result) = 0;
		virtual bool GetMethodResponseData(ClientUnifiedMessageHandle handle, void* response_buffer, std::uint32_t response_buffer_size,
			bool auto_release) = 0;
		virtual bool ReleaseMethod(ClientUnifiedMessageHandle handle) = 0;
		virtual bool SendNotification(const char* service_notification, const void* notification_buffer, std::uint32_t notification_buffer_size) = 0;
	};
	ENGINE_ASSERT_SZ(ISteamUnifiedMessages, 0x0008);
}
