#include "common.hpp"
#include "component/name.hpp"
#include "component/auth.hpp"

#include "component/command.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>
#include <utils/properties.hpp>
#include <utils/string.hpp>

namespace name {
	namespace {
		utils::concurrency::container<std::string> player_name{};

		void store_player_name(const std::string& name) {
			utils::properties::store("playerName", name);
		}

		void activate_player_name(std::string new_name) {
			player_name.access([&](std::string& name) {
				name = std::move(new_name);
			});
		}

		void update_player_name(const std::string& new_name) {
			activate_player_name(new_name);
			store_player_name(new_name);
		}

		void setup_player_name() {
			std::string initial_name = utils::nt::get_user_name();

			if (initial_name.empty()) {
				initial_name = "Unknown Soldier";
			}

			update_player_name(initial_name);
		}

		void load_player_name() {
			const auto stored_name = utils::properties::load("playerName");

			if (stored_name) {
				activate_player_name(*stored_name);
			}
			else {
				setup_player_name();
			}
		}
	}

	const char* get_player_name() {
		const auto name = player_name.copy();
		return utils::string::va("%.*s", static_cast<int>(name.size()), name.data());
	}

	void set_player_name(const std::string& name) {
		update_player_name(name);
		auth::get_local_user_data()->assign_username(name::get_player_name());
	}

	struct component final : client_component {
		component_priority priority() const override {
			return component_priority::name;
		}

		void post_load() override {
			load_player_name();
		}

		void post_unpack() override {
			command::add("name", [](const command::params_cl& params) {
				if (params.arg_count() != 1) {
					return;
				}

				set_player_name(params.get_arg<std::string>(0));
			});
		}
	};
}

REGISTER_COMPONENT(name::component)
