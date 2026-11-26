#include "common.hpp"
#include "component/key_catchers.hpp"

#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

namespace key_catchers {
	namespace {
		functions::Sys_QueEventT* Sys_QueEvent{};

		int* keyCatchers{};

		std::vector<ke_typedef> key_event_callbacks;
		std::vector<ch_typedef> char_event_callbacks;

		utils::hook::detour sys_que_event_hook;

		bool on_char_event(const int local_client_num, const s4::keyNum_t key) {
			for (auto& func : char_event_callbacks) {
				if (!func(local_client_num, key)) {
					return false;
				}
			}

			return true;
		}

		bool on_key_event(const int local_client_num, const s4::keyNum_t key, bool down) {
			for (auto& func : key_event_callbacks) {
				if (!func(local_client_num, key, down)) {
					return false;
				}
			}

			return true;
		}

		void sys_que_event_stub(int time, s4::sysEventType_t type, int value, int value2, int ptr_length, void* ptr) {
			if (type == s4::sysEventType_t::SE_KEY) {
				if (!on_key_event(0, static_cast<s4::keyNum_t>(value), (value2 & 1) == 1)) {
					return;
				}
			}
			else if (type == s4::sysEventType_t::SE_CHAR) {
				if (!on_char_event(0, static_cast<s4::keyNum_t>(value))) {
					return;
				}
			}
			sys_que_event_hook.invoke<void>(time, type, value, value2, ptr_length, ptr);
		}
	}

	void add_key_event(ke_typedef&& func) {
		key_event_callbacks.push_back(std::move(func));
	}

	void add_char_event(ch_typedef&& func) {
		char_event_callbacks.push_back(std::move(func));
	}

	bool get_state() {
		return *keyCatchers & 1;
	}

	void toggle_state() {
		*keyCatchers ^= 1;
	}

	void remove_catcher(const int and_mask) {
		if (!keyCatchers) {
			return;
		}

		*keyCatchers &= and_mask;
	}

	class component final : public generic_component {
	public:
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Sys_QueEvent), "E8 ? ? ? ? 45 33 C9 4C 89 74 24 ? 44 8B C6", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(keyCatchers), "81 B4 2F ? ? ? ? 00 02 00 00", SETUP_MOD(add(3).based_rel()));
		}

		bool should_activate() override {
			return Sys_QueEvent && keyCatchers;
		}

		void post_unpack() override {
			sys_que_event_hook.create(Sys_QueEvent, sys_que_event_stub);
		}
	};
}

REGISTER_COMPONENT(key_catchers::component)
