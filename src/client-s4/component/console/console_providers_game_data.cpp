#include "game/game.hpp"
#include "game/dvars.hpp"

#include "component/command.hpp"
#include "component/key_catchers.hpp"

#include <component/console/console_providers.hpp>
#include <loader/component_loader.hpp>

namespace console::detail {
	bool game_data_provider::get_key_catch_state() {
		return key_catchers::get_state();
	}

	void game_data_provider::toggle_key_catch_state() {
		return key_catchers::toggle_state();
	}

	void game_data_provider::execute_cbuf(const std::string &data) {
		return game::Cbuf_AddText_AutoEncrypt(data);
	}

	std::uint64_t game_data_provider::generate_dvar_checksum(const std::string &data) {
		return game::hash_dvar(data);
	}

	bool game_data_provider::find_dvar_by_name(const std::string &data) {
		return dvars::get_dvar_by_name(data) != nullptr;
	}

	bool game_data_provider::find_dvar_by_hash(std::uint64_t data) {
		return dvars::get_dvar_by_hash(data) != nullptr;
	}

	std::string game_data_provider::get_dvar_display_value(std::uint64_t hash) {
		auto dvar = dvars::get_dvar_by_hash(hash);
		if (!dvar) {
			return "<unavailable>";
		}

		const auto value = game::Dvar_ValueToString(dvar, &dvar->get_current());
		return value ? value : "<unavailable>";
	}

	std::string game_data_provider::get_dvar_default_display_value(std::uint64_t hash) {
		auto dvar = dvars::get_dvar_by_hash(hash);
		if (!dvar) {
			return "<unavailable>";
		}

		const auto value = game::Dvar_ValueToString(dvar, &dvar->get_reset());
		return value ? value : "<unavailable>";
	}

	std::string game_data_provider::get_dvar_domain(std::uint64_t hash) {
		auto dvar = dvars::get_dvar_by_hash(hash);
		if (!dvar) {
			return "Domain is unavailable";
		}

		return dvars::dvar_get_domain(dvar->get_type(), dvar->get_domain());
	}

	int game_data_provider::get_dvar_line_count(std::uint64_t hash) {
		auto dvar = dvars::get_dvar_by_hash(hash);
		if (!dvar) {
			return 2;
		}

		return dvar->get_type() == s4::DvarType::DVAR_TYPE_ENUM
			? dvar->get_domain().enumeration_.string_count_ + 2
			: 2
		;
	}
}
