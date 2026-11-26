#pragma once
#include "common.hpp"
#include "game/function_types.hpp"

#include <component/command/command_base.hpp>

namespace command {

	using params_cl = detail::params<detail::cl_cmd_args_provider, s4::CmdArgs>;
	using params_sv = detail::params<detail::sv_cmd_args_provider, s4::CmdArgs>;
	using client_info = detail::client_info_provider<s4::SvClient>;

	namespace detail {
		inline cl_cmd_bind_provider<params_cl> cl_cmd_bind;
		inline sv_cmd_bind_provider<params_sv, client_info> sv_cmd_bind;
	}

	inline void add(const char* name, const std::function<void(const params_cl&)>& callback) {
		detail::cl_cmd_bind.add(name, callback);
	}

	inline void add(const char* name, const std::function<void()>& callback) {
		detail::cl_cmd_bind.add(name, callback);
	}

	inline void add_sv(const char* name, std::function<void(const client_info&, const params_sv&)> callback) {
		detail::sv_cmd_bind.add_sv(name, callback);
	}

	inline void remove(const char* name) {
		detail::cl_cmd_bind.remove(name);
	}

	inline void remove_sv(const char* name) {
		detail::sv_cmd_bind.remove_sv(name);
	}

	s4::gclient_s* get_client(int client_num);
	s4::gentity_s* get_entity(int client_num);
	s4::SvClient* get_sv_client(int client_num);
	bool check_cheats(int client_num);
	bool check_running();

	void execute(std::string command, bool sync = false);
}
