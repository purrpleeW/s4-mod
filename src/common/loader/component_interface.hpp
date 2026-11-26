#pragma once

#include "memory/signature_store.hpp"
enum class component_priority {
	lua_hook = -1,
	min = 0,
	name,
	steam_proxy,
	arxan,
	exception_filter,
	call_spoofer,
	pointers,
	ci_test
};

enum class component_type {
	client,
	server,
	any
};

struct generic_component {
	static constexpr component_type type = component_type::any;

	virtual ~generic_component() = default;

	virtual void post_load() {}
	virtual void pre_destroy() {}
	virtual void post_unpack() {}

	virtual void find_signatures(memory::signature_store& batch) {}
	virtual void find_signatures_bnet(memory::signature_store& batch) {}
	virtual void find_signatures_steam(memory::signature_store& batch) {}
	virtual void find_signatures_msstore(memory::signature_store& batch) {}

	virtual bool should_enable() {
		return true;
	}

	virtual bool should_activate() { return true; }
	virtual bool should_activate_bnet() { return true; }
	virtual bool should_activate_steam() { return true; }
	virtual bool should_activate_msstore() { return true; }

	virtual component_priority priority() const {
		return component_priority::min;
	}
};

struct client_component : generic_component {
	static constexpr component_type type = component_type::client;
};

struct server_component : generic_component {
	static constexpr component_type type = component_type::server;
};
