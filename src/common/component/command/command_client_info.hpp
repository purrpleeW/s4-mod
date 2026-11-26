#pragma once

#include "common_core.hpp"

namespace command::detail {
	template <typename C>
	concept generic_sv_client = requires(C c) {
		{ c.IsLocalClient() } -> std::same_as<bool>;
	};

	template <generic_sv_client C>
	struct cip_clients_provider {
		static void* get_local_client(int client_num);
		static void* get_local_entity(int client_num);
		static C* get_server_client(int client_num);
	};

	template <generic_sv_client C>
	class client_info_provider {
		using traits = cip_clients_provider<C>;
	public:
		client_info_provider() = delete;
		explicit client_info_provider(int client_num) {
			this->num_ = client_num;
			this->client_ = traits::get_local_client(client_num);
			this->entity_ = traits::get_local_entity(client_num);
			this->sv_client_ = traits::get_server_client(client_num);
		}

		bool is_valid() {
			return this->client_ && this->entity_ && this->sv_client_;
		}

		int num_;
		void* client_;
		void* entity_;
		C* sv_client_;
	};
}