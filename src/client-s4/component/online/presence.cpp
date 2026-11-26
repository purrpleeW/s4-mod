#include "common.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace online::presence {
	namespace {
		functions::SetPresenceT* SetPresence{};

		constexpr std::size_t set_presence_scan_size = 0x800;

		void force_local_presence_status() {
			const auto code = PTR_AS(std::uint8_t*, SetPresence);
			const std::uint8_t owner_compare[] = { 0x3B, 0xC3, 0x48, 0x8B, 0x9C, 0x24 };

			for (std::size_t offset = 0; offset < set_presence_scan_size; offset++) {
				if (std::memcmp(code + offset, owner_compare, sizeof(owner_compare)) != 0) {
					continue;
				}

				auto branch = code + offset + 2;
				while (branch[0] == 0x48 && branch[1] == 0x8B && branch[3] == 0x24) {
					branch += 8;
				}

				if (branch[0] == 0x0F && branch[1] == 0x84) {
					utils::hook::set<std::uint8_t>(branch, 0x90);
					utils::hook::set<std::uint8_t>(branch + 1, 0xE9);
				}
				
				else if (branch[0] == 0x74) {
					utils::hook::set<std::uint8_t>(branch, 0xEB);
				}

				return;
			}
		}
	}

	struct component final : client_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(SetPresence), "39 44 CB ? 7D ? 8B CE E8 ? ? ? ? 8B CE E8 ? ? ? ? 84 C0 74 ? 8B CE E8", SETUP_MOD(add(9).rip()));
		}

		bool should_activate() override {
			return SetPresence != nullptr;
		}

		void post_unpack() override {
			force_local_presence_status();

			scheduler::loop<scheduler::pipeline::client>([] {
				SetPresence(0);
			}, 5000ms);
		}
	};
}

REGISTER_COMPONENT(online::presence::component) // in case this component isn't clear, this is a Discord RPC presence, not anything actually in-game
