#include "common.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <memory/signature_store.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

#include <magic_enum/magic_enum.hpp>

namespace patches::dlog {
	namespace {
		functions::DLog_RecordErrorEventT* DLog_RecordErrorEvent{};
		functions::DLog_StringT* DLog_String{};

		utils::hook::detour dlog_record_error_event_hook;
		utils::hook::detour dlog_string_hook;

		void dlog_record_error_event_stub(s4::DLogErrorCode code, const char* message, const char* stack_trace) {
			const auto remove_new_lines = [](const std::string& input) -> std::string {
				std::string output = input;
				while (output.starts_with("\n")) {
					output = output.substr(1);
				}
				while (output.ends_with("\n")) {
					output = output.substr(0, output.size() - 1);
				}
				return output;
			};

			const auto message_str = remove_new_lines(message ? message : "<null>");
			const auto stack_trace_str = remove_new_lines(stack_trace ? stack_trace : "<null>");

			LOG("Component/Patches/DLog", ERROR, "[RecordErrorEvent] Code {} [{}]: {} - Stacktrace: {}",
				ENUM_UNDER(code), magic_enum::enum_name(code), message_str, stack_trace_str);
			return dlog_record_error_event_hook.invoke<void>(code, message, stack_trace);
		}

		bool dlog_string_stub(s4::DLogContext* context, const char* name, const char* value, bool coerce) {
			if (context == nullptr || name == nullptr || value == nullptr) {
				// prevents a crash upon trying to enter an online custom game due to there
				// being no available playlists when offline
				return false;
			}
			return dlog_string_hook.invoke<bool>(context, name, value, coerce);
		}
	}

	struct component final : generic_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(DLog_RecordErrorEvent), "E8 ? ? ? ? B1 ? C6 05 ? ? ? ? ? E8 ? ? ? ? E8", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(DLog_String), "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 83 79 ? ? 45 0F B6 F1");
		}

		void post_unpack() override {
			dlog_record_error_event_hook.create(DLog_RecordErrorEvent, dlog_record_error_event_stub);
			dlog_string_hook.create(DLog_String, dlog_string_stub);
		}
	};
}

REGISTER_COMPONENT(patches::dlog::component)
