#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

#include <engine/msvc/thread_name_info.hpp>

namespace patches::nasty_exception {
	namespace {
		utils::hook::iat_detour raise_exception_hook;

		void raise_exception_stub(std::uint32_t exception_code_in, std::uint32_t exception_flags, std::uint32_t number_of_arguments,
			const std::uint64_t* arguments)
		{
			LOG("Component/Patches/NastyException", DEBUG, "Raising exception: 0x{:08X} (fl:0x{:08X}, na:0x{:08X}) and returning to {}",
				exception_code_in, exception_flags, number_of_arguments, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())));

			if (exception_code_in == 0x1337 || exception_code_in == 0x1338) {
				LOG("Component/Patches/NastyException", DEBUG, "DebugException -> Ignoring 0x{:08X} exception so we're chill", exception_code_in);
				return;
			}

			if (exception_code_in == 0x406D1388) {
				auto thread_name_info_in = PTR_AS(const msvc::thread_name_info*, arguments);
				LOG("Component/Patches", DEBUG, "SetThreadName -> Setting name of thread with ID {} to {}",
					thread_name_info_in->thread_id_, thread_name_info_in->name_ ? thread_name_info_in->name_ : "<null>");
			}

			return raise_exception_hook.invoke<void>(exception_code_in, exception_flags, number_of_arguments, arguments);
		}
	}

	struct component final : generic_component {
		void post_load() override {
			raise_exception_hook.create("kernel32.dll", "RaiseException", raise_exception_stub);
		}
	};
}

REGISTER_COMPONENT(patches::nasty_exception::component)
