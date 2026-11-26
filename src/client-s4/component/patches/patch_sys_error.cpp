#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::sys_error {
	namespace {
		functions::Sys_ErrorT* Sys_Error{};

		void sys_error_stub(const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);

			char buffer[0x800];
			vsnprintf(buffer, sizeof(buffer), fmt, args);

			va_end(args);

			LOG("Component/Patches/SysError", WARN, "[from {}]: {}", and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())), buffer);
			ci::end_test_run(0 /* Exited Successfully */);
		}
	}

	struct component final : generic_component {
		void find_signatures_bnet(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Sys_Error), "E8 ? ? ? ? CC 48 8D 54 24 ? B9", SETUP_MOD(add(1).rip()));
		}

		void find_signatures_steam(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Sys_Error), "E8 ? ? ? ? CC 45 33 C0 48 8B CD", SETUP_MOD(add(1).rip()));
		}

		bool should_activate() override {
			return Sys_Error != nullptr;
		}

		void post_unpack() override {
			utils::hook::jump(Sys_Error, sys_error_stub);
		}
	};
}

REGISTER_COMPONENT(patches::sys_error::component)
