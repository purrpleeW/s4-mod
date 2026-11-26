#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::frontend {
	namespace {
		functions::unk_FrontEnd_GetZoneNameT* unk_FrontEnd_GetZoneName{};
		utils::hook::detour unk_frontend_get_zone_name_hook;

		constexpr std::uint8_t frontend_mode_mp = 2;

		const char* unk_frontend_get_zone_name_stub(std::uint8_t mode) {
			const auto zone_name = unk_frontend_get_zone_name_hook.invoke<const char*>(mode);
			if (mode == frontend_mode_mp && zone_name != nullptr && std::string_view(zone_name) == "mp_frontend8") {
				return "mp_frontend8_a";
			}

			return zone_name;
		}
	}

	struct component final : generic_component {
		bool should_enable() override {
			return identification::game::is("1.14.0");
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(unk_FrontEnd_GetZoneName), "48 83 EC ? 0F B6 D1 83 EA 02 74 ? 83 FA 01 75 ? 48 8B 05 ? ? ? ? EB");
		}

		bool should_activate() override {
			return unk_FrontEnd_GetZoneName != nullptr;
		}

		void post_unpack() override {
			unk_frontend_get_zone_name_hook.create(unk_FrontEnd_GetZoneName, unk_frontend_get_zone_name_stub);
		}
	};
}

REGISTER_COMPONENT(patches::frontend::component)
