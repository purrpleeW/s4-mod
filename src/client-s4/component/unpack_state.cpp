#include "common.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace unpack_state {
	namespace {
		utils::hook::iat_detour create_file_a_hook;
		utils::hook::iat_detour exit_process_hook;

		HANDLE create_file_a_stub(LPCSTR lp_file_name, DWORD dw_desired_access, DWORD dw_share_mode, LPSECURITY_ATTRIBUTES lp_security_attributes,
			DWORD dw_creation_disposition, DWORD dw_flags_and_attributes, HANDLE h_template_file)
		{
			static bool scanned = false;
			if (!scanned) {
				static memory::signature_store batch;
				static utils::nt::library game{};
				game.unprotect();	// fixes non-Arxan executables :)
				component_loader::find_signatures(batch);
				batch.scan_all();
				//batch.dump();		// for debugging
				scanned = true;

				ci::record_test_state(ci::Started, ci::Hook, "client-s4/Component/UnpackState", "post_unpack()");
				component_loader::post_unpack();
				ci::record_test_state(ci::Ended, ci::Hook, "client-s4/Component/UnpackState", "post_unpack()");
			}

			return create_file_a_hook.invoke<HANDLE>(lp_file_name, dw_desired_access, dw_share_mode, lp_security_attributes, dw_creation_disposition,
				dw_flags_and_attributes, h_template_file);
		}

		void exit_process_stub(UINT u_exit_code) {
			component_loader::pre_destroy();
			exit_process_hook.invoke<void>(u_exit_code);
		}
	}

	struct component final : generic_component {
		void post_load() override {
			create_file_a_hook.create("kernel32.dll", "CreateFileA", create_file_a_stub);
			exit_process_hook.create("kernel32.dll", "ExitProcess", exit_process_stub);
		}
	};
}

REGISTER_COMPONENT(unpack_state::component)
