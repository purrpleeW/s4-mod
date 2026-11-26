#include "common.hpp"
#include "component/online/ui/cod_points_state.hpp"

#include "component/command.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/properties.hpp>
#include <utils/safe_release.hpp>

namespace online::ui::cod_points {
	namespace {
		using publish_currency_models_t = void(int controller_index);
		using get_controller_model_t = std::uint16_t(int controller_index);

		publish_currency_models_t* unk_publish_currency_models{};
		get_controller_model_t* LUI_Model_GetModelForController{};
		void* currency_model_set_int_call{};
		functions::LUI_Model_SetIntT* currency_model_set_int{};
		utils::hook::detour publish_currency_models_hook;
		display_balance balance;
		constexpr const char* property_name = "s4DisplayCodPoints";

		struct currency_publication {
			int setting;
			bool saw_cod_points = false;
		};
		thread_local currency_publication* current_publication = nullptr;

		void set_currency_model_stub(std::uint16_t model, int native_balance, int currency) {
			if (current_publication) {
				if (currency == currency_id) {
					current_publication->saw_cod_points = true;
				}
				native_balance = resolve_display_balance(current_publication->setting, currency, native_balance);
			}
			currency_model_set_int(model, native_balance);
		}

		void publish_currency_models_stub(int controller_index) {
			currency_publication publication{ balance.get() };
			auto* previous = std::exchange(current_publication, &publication);
			auto restore_context = [previous] {
				current_publication = previous;
			};
			const auto restore_publication = utils::safe_release(std::move(restore_context));

			publish_currency_models_hook.invoke<void>(controller_index);
			if (publication.saw_cod_points) {
				return;
			}

			// we can add to the visual balance without actually adding currency
			const auto controller_model = LUI_Model_GetModelForController(controller_index);
			if (!controller_model) {
				return;
			}
			const auto model = game::LUI_Model_CreateModelFromPath(controller_model, model_path);
			if (model) {
				currency_model_set_int(model, resolve_display_balance(publication.setting, currency_id, 0));
			}
		}

		void refresh_display_balance() {
			if (!game::Com_FrontEnd_IsInFrontEnd() || !LUI_Model_GetModelForController(0)) {
				return;
			}
			unk_publish_currency_models(0);
		}

		void display_balance_command(const command::params_cl& params) {
			if (params.arg_count() == 0) {
				return;
			}
			const auto value = params.arg_count() == 1 ? parse_display_balance(params[1]) : std::nullopt;
			if (!value || !balance.set(*value)) {
				LOG("Component/CodPoints", INFO, "Usage: cp_displayBalance <0 to 2147483647 or 'native'>");
				return;
			}

			utils::properties::store(property_name, *value == native_display ? "native" : std::to_string(*value));
		}

		void load_balance() {
			const auto stored = utils::properties::load(property_name);
			if (!stored) {
				return;
			}
			const auto value = parse_display_balance(*stored);
			if (value) {
				balance.set(*value);
				return;
			}
			LOG("Component/CodPoints", WARN, "Invalid saved balance, using {}", default_balance);
		}
	}

	struct component final : client_component {
		void post_load() override {
			load_balance();
		}

		void find_signatures(memory::signature_store& batch) override {
			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(unk_publish_currency_models), "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 8B F1 E8 ? ? ? ? 44 0F B7 F0 E8 ? ? ? ? 8B D6 48 8B C8 E8 ? ? ? ? 33 FF 8B E8 85 C0");

				batch.add(SETUP_POINTER(LUI_Model_GetModelForController), "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 8B F1 E8 ? ? ? ? 44 0F B7 F0 "
					"E8 ? ? ? ? 8B D6 48 8B C8 E8 ? ? ? ? 33 FF 8B E8 85 C0", SETUP_MOD(add(24).rip()));

				batch.add(SETUP_POINTER(currency_model_set_int_call), "48 8B D0 41 0F B7 CE E8 ? ? ? ? 8B 53 08 0F B7 C8 E8 ? ? ? ? FF C7 3B FD", SETUP_MOD(add(18)));

				batch.add(SETUP_POINTER(currency_model_set_int), "48 8B D0 41 0F B7 CE E8 ? ? ? ? 8B 53 08 0F B7 C8 E8 ? ? ? ? FF C7 3B FD", SETUP_MOD(add(19).rip()));
			}
			else {
				batch.add(SETUP_POINTER(unk_publish_currency_models), "48 89 5C 24 ? 55 56 41 56 48 83 EC ? 8B F1 E8 ? ? ? ? 44 0F B7 F0 E8 ? ? ? ? 8B D6 48 8B C8 E8 ? ? ? ? 33 DB 8B E8 85 C0");

				batch.add(SETUP_POINTER(LUI_Model_GetModelForController), "48 89 5C 24 ? 55 56 41 56 48 83 EC ? 8B F1 E8 ? ? ? ? 44 0F B7 F0 E8 ? ? ? ? 8B D6 48 "
					"8B C8 E8 ? ? ? ? 33 DB 8B E8 85 C0", SETUP_MOD(add(16).rip()));

				batch.add(SETUP_POINTER(currency_model_set_int_call), "48 8B D0 41 0F B7 CE E8 ? ? ? ? 8B 57 08 0F B7 C8 E8 ? ? ? ? FF C3 3B DD", SETUP_MOD(add(18)));

				batch.add(SETUP_POINTER(currency_model_set_int), "48 8B D0 41 0F B7 CE E8 ? ? ? ? 8B 57 08 0F B7 C8 E8 ? ? ? ? FF C3 3B DD", SETUP_MOD(add(19).rip()));
			}
		}

		void post_unpack() override {
			if (!unk_publish_currency_models || !LUI_Model_GetModelForController || !currency_model_set_int_call || !currency_model_set_int
				|| currency_model_set_int != game::LUI_Model_SetInt
				|| !game::LUI_Model_CreateModelFromPath || !game::Com_FrontEnd_IsInFrontEnd) {
				return;
			}

			// we can use RDI (RBX on the beta) to see that it points to the currency entry, then we see that the first
			// byte is the currency ID. all we have to do is pass that as an arg
			const auto currency_entry = identification::game::is_config(identification::game::Beta) ? rbx : rdi;
			const auto [stub, stub_size] = utils::hook::assemble([currency_entry](utils::hook::assembler& assembler) {
				assembler.movzx(r8d, byte_ptr(currency_entry));
				assembler.jmp(set_currency_model_stub);
			});
			if (!stub) {
				LOG("Component/CodPoints", WARN, "Unable to create display stub for cod points!");
				return;
			}
			utils::hook::call(currency_model_set_int_call, stub);
			publish_currency_models_hook.create(unk_publish_currency_models, publish_currency_models_stub);

			command::add("cp_displayBalance", display_balance_command);
			scheduler::loop<scheduler::pipeline::client>(refresh_display_balance, 100ms);
		}
	};
}

REGISTER_COMPONENT(online::ui::cod_points::component)
