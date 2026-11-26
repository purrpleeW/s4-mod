#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>

namespace pointers {
	struct component final : generic_component {
		component_priority priority() const override {
			return component_priority::pointers;
		}

		void find_signatures(memory::signature_store& batch) override {
			game::init(batch);
		}
	};
}

REGISTER_COMPONENT(pointers::component)
