#include "common.hpp"
#include "game/game.hpp"
#include "game/dvars.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace test {
	namespace {

	}

	struct component final : generic_component {
		void post_load() override {
			// scheduler::once<scheduler::pipeline::dvars>([]{
			// 	dvars::register_bool("unattended", true);
			// });
		}
	};
}

REGISTER_COMPONENT(test::component)
