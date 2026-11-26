#include "common.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace scheduler {
	namespace {
		functions::R_EndFrameT* R_EndFrame{};

		utils::hook::detour r_end_frame_hook;
		utils::hook::detour main_frame_hook;

		void r_end_frame_stub() {
			execute<pipeline::client>();
			return r_end_frame_hook.invoke<void>();
		}

		void g_clear_vehicle_inputs_stub() {
			//game::G_ClearVehicleInputs();
			return execute<pipeline::server>();
		}

		void main_frame_stub() {
			main_frame_hook.invoke<void>();
			return execute<pipeline::main>();
		}

		std::jthread async_thread;
	}

	struct component final : generic_component {
		void post_load() override {
			async_thread = init_async();
		}

		void find_signatures(memory::signature_store& batch) override {
			if (identification::game::is_less_or_eq("1.10.0")) {
				batch.add(SETUP_POINTER(R_EndFrame), "48 83 EC ? E8 ? ? ? ? 48 8B 15 ? ? ? ? 45 33 D2");
			}
			else {
				batch.add(SETUP_POINTER(R_EndFrame), "48 8B 15 ? ? ? ? 45 33 D2 4C 8B 0D");
			}
		}

		void post_unpack() override {
			if (!game::is_server()) {
				r_end_frame_hook.create(R_EndFrame, r_end_frame_stub);
			}

			// Com_Frame_Try_Block_Function
			//main_frame_hook.create(game::select(0x1420F8E00, 0x1405020E0), main_frame_stub);
			//utils::hook::call(game::select(0x14225522E, 0x140538427), g_clear_vehicle_inputs_stub);
		}

		void pre_destroy() override {
			if (async_thread.joinable() && async_thread.request_stop()) {
				async_thread.join();
			}
		}
	};
}

REGISTER_COMPONENT(scheduler::component)
