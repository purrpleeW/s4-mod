#include "common_core.hpp"
#include "loader/component_loader.hpp"

#include "component_loader.hpp"
#include "identification/utility.hpp"
#include "identification/game.hpp"
#include "utils/nt.hpp"
#include <memory/signature_store.hpp>
#include "ci/ci.hpp"
#include "utils/concurrency.hpp"
#include "utils/string.hpp"

namespace component_loader {
	namespace {
		bool post_load_phase_complete = false;
		bool post_unpack_phase_complete = false;

		std::vector<std::unique_ptr<generic_component>>& get_components() {
			using component_vector = std::vector<std::unique_ptr<generic_component>>;
			using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

			static component_vector_container components(new component_vector, [](const component_vector* component_vector) {
				pre_destroy();
				delete component_vector;
			});

			return *components;
		}

		std::vector<std::pair<registration_functor, component_type>>& get_registration_functors() {
			static std::vector<std::pair<registration_functor, component_type>> functors;
			return functors;
		}

		void activate_component(std::unique_ptr<generic_component> component) {
			auto& components = get_components();
			components.push_back(std::move(component));

			std::ranges::stable_sort(components, [](const std::unique_ptr<generic_component>& a, const std::unique_ptr<generic_component>& b) {
				return a->priority() > b->priority();
			});
		}
	}

	void register_component(registration_functor functor, component_type type) {
		if (!get_components().empty()) {
			throw std::runtime_error("Registration is too late");
		}

		get_registration_functors().emplace_back(std::move(functor), type);
	}

	void ensure_load(bool ignore_iden) {
		if (ignore_iden) {
			return;
		}

		static auto info = identification::game::get_target_info_game();
		if (!identification::game::is_target_game()) {
			ci::end_test_run(-127 /* Catastrophic Failure on Startup */);
			const auto btn_press = MessageBoxA(nullptr,
				"This client has been launched on an unknown/unsupported game/game version. Try to load anyway?",
				utils::string::va("%s - %s", info.game_name.c_str(), info.client_name.c_str()), MB_YESNO | MB_ICONWARNING
			);

			if (btn_press != IDYES) {
				TerminateProcess(GetCurrentProcess(), 0);
			}
		}
	}

	bool activate(bool server) {
		static auto res = [server] {
			try {
				for (auto& functor : get_registration_functors()) {
					if (functor.second == component_type::any || server == (functor.second == component_type::server)) {
						activate_component(functor.first());
					}
				}
			}
			catch (premature_shutdown_trigger&) {
				return false;
			}
			catch (const std::exception& e) {
				utils::nt::show_error(e.what());
				return false;
			}

			return true;
		}();

		return res;
	}

	bool post_load() {
		static auto res = [] {
			try {
				for (const auto& component : get_components()) {
					if (!component->should_enable()) {
						continue;
					}
					component->post_load();
				}
			}
			catch (premature_shutdown_trigger&) {
				return false;
			}
			catch (const std::exception& e) {
				utils::nt::show_error(e.what());
				return false;
			}

			post_load_phase_complete = true;
			return true;
		}();

		return res;
	}

	void post_unpack() {
		static auto res = [] {
			try {
				while (!component_loader::is_post_load_phase_complete() || !finished_auth()) {
					Sleep(100);
				}

				for (const auto& component : get_components()) {
					if (!component->should_enable() || !component->should_activate()) {
						continue;
					}

					if (identification::game::is_platform(identification::game::Steam)
						&& !component->should_activate_steam()
					) { continue; }
					else if (identification::game::is_platform(identification::game::BattleNet)
						&& !component->should_activate_bnet()
					) { continue; }
					else if (identification::game::is_platform(identification::game::MsStore)
						&& !component->should_activate_msstore()
					) { continue; }

					// prints vtable of component for debugging...
					// LOG("Test", DEBUG, "Running component post unpack: {}", and_rel(DEREF_PTR_AS(std::uintptr_t, component.get())));
					component->post_unpack();
				}
			}
			catch (const std::exception& e) {
				utils::nt::show_error(e.what());
				return false;
			}

			post_unpack_phase_complete = true;
			return true;
		}();

		if (!res) {
			TerminateProcess(GetCurrentProcess(), 1);
		}
	}

	void find_signatures(memory::signature_store& batch) {
		static auto res = [&] {
			try {
				for (const auto& component : get_components()) {
					if (!component->should_enable()) {
						continue;
					}
					component->find_signatures(batch);

					if (identification::game::is_platform(identification::game::Steam)) {
						component->find_signatures_steam(batch);
					} else if (identification::game::is_platform(identification::game::BattleNet)) {
						component->find_signatures_bnet(batch);
					} else if (identification::game::is_platform(identification::game::MsStore)) {
						component->find_signatures_msstore(batch);
					}
				}
			}
			catch (const std::exception& e) {
				utils::nt::show_error(e.what());
				return false;
			}

			post_unpack_phase_complete = true;
			return true;
		}();

		if (!res) {
			TerminateProcess(GetCurrentProcess(), 1);
		}
	}

	void pre_destroy() {
		static auto res = [] {
			try {
				for (const auto& component : get_components()) {
					if (!component->should_enable()) {
						continue;
					}
					component->pre_destroy();
				}
			}
			catch (const std::exception& e) {
				utils::nt::show_error(e.what());
				return false;
			}

			return true;
		}();

		if (!res) {
			TerminateProcess(GetCurrentProcess(), 1);
		}
	}

	bool is_post_load_phase_complete() {
		return post_load_phase_complete;
	}

	bool is_post_unpack_phase_complete() {
		return post_unpack_phase_complete;
	}

	void trigger_premature_shutdown() {
		throw premature_shutdown_trigger();
	}
}