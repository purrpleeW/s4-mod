#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/msg_t.hpp"
#include "utils/string.hpp"

namespace iw8 {
	class XUID {
	public:
		XUID(std::uint64_t id = 0)
			: id_(id)
		{}

		std::uint64_t id_;

		void apply_xuid(std::uint64_t* ptr) {
			if (ptr == nullptr) {
				return;
			}
			*ptr = this->get_universal_id();
		}

		XUID* from_string(const char* xuid_string, int base = 10) {
			auto atoui64 = [](const std::string& str, int base) -> std::uint64_t {
				try {
					return std::stoull(str, nullptr, base);
				}
				catch (...) {
					return 0;
				}
			};

			this->id_ = atoui64(xuid_string, base);
			return this;
		}

		std::uint64_t get_universal_id() {
			return this->id_;
		};

		std::uint64_t get_proper_id() {
			return 0x11CB1243B8D7C31E | (this->id_ * this->id_);
		};

		const char* to_hex_string() {
			return utils::string::va("%zx", this->id_);
		}

		const char* to_string() {
			return utils::string::va("%zu", this->id_);
		}

		bool operator !=(const XUID* xuid) {
			return this->id_ != xuid->id_;
		}

		XUID* operator =(const XUID* xuid) {
			this->id_ = xuid->id_;
			return this;
		}

		bool operator ==(const XUID* xuid) {
			return this->id_ == xuid->id_;
		}
	};
}
