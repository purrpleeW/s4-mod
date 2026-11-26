#include "common.hpp"
#include "game/game.hpp"

#include <ci/ci.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::com_error {
	namespace {
		utils::hook::detour com_error_impl_hook;

		void com_error_impl_stub(s4::errorParm_t code, const char* fmt, ...) {
			static std::vector<std::string> ignored_list = {
				"DEV ERROR 324",
				"DEV ERROR 1163",
				"EXE/CONFIGSTRINGMISMATCH"
			};

			va_list args;
			va_start(args, fmt);

			char buffer[0x800];
			vsnprintf(buffer, sizeof(buffer), fmt, args);

			va_end(args);

			LOG("Component/Patches/ComError", WARN, "[from {}]: {}", and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())), buffer);

			ci::end_test_run(0 /* Exited Successfully */);

			bool message_in_ignore_list = false;
			for (auto& ignored : ignored_list) {
				if (strstr(buffer, ignored.c_str()) == nullptr) {
					continue;
				}
				message_in_ignore_list = true;
				break;
			}

			if (!message_in_ignore_list) {
				com_error_impl_hook.invoke<void>(code, buffer);
			}
		}
	}

	struct component final : generic_component {
		bool should_activate() override {
			return game::Com_Error_impl != nullptr;
		}

		void post_unpack() override {
			com_error_impl_hook.create(game::Com_Error_impl, com_error_impl_stub);
		}
	};
}

REGISTER_COMPONENT(patches::com_error::component)
