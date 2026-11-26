#pragma once

#include "common_core.hpp"
#include <logger/logger.hpp>

#include <component/arxan/arxan_detail.hpp>
#include <component/arxan/arxan_code_healing.hpp>
#include <component/arxan/arxan_integrity_checks.hpp>

namespace arxan {
	bool is_set_to_load();

	namespace detail {
		inline std::atomic_bool should_notify_on_exception = true;

		inline void patch_code_healers() {
			auto patched_al_basic = code_healing::patch_code_healer_block(code_healer_type::al_basic);
			auto patched_al_lea = code_healing::patch_code_healer_block(code_healer_type::al_lea);
			auto patched_al_jmp = code_healing::patch_code_healer_block(code_healer_type::al_jmp);
			auto patched_eax_basic = code_healing::patch_code_healer_block(code_healer_type::eax_basic);
			auto patched_eax_lea = code_healing::patch_code_healer_block(code_healer_type::eax_lea);
			auto patched_eax_jmp = code_healing::patch_code_healer_block(code_healer_type::eax_jmp);
			auto total = patched_al_basic + patched_al_jmp + patched_eax_basic + patched_eax_jmp;

			LOG("Component/Arxan", INFO, "Patched {} code healers. [{} AB, {} AL, {} AJ, {} EB, {} EL, {} EJ]", total,
				patched_al_basic, patched_al_lea, patched_al_jmp, patched_eax_basic, patched_eax_lea, patched_eax_jmp);
		}

		inline void patch_integrity_checks() {
			integrity_checks::assign_integrity_checks(potential_integrity_checks);

			auto patched_intact_basic = integrity_checks::patch_integrity_check_block(integrity_check_type::intact_basic);
			auto patched_intact_big = integrity_checks::patch_integrity_check_block(integrity_check_type::intact_big);
			auto patched_split_basic = integrity_checks::patch_integrity_check_block(integrity_check_type::split_basic);
			auto patched_split_big = integrity_checks::patch_integrity_check_block(integrity_check_type::split_big);
			auto total = patched_intact_basic + patched_intact_big + patched_split_basic + patched_split_big;

			LOG("Component/Arxan", INFO, "Patched {} integrity checks. [{} Intact Basic, {} Intact Big, {} Split Basic, {} Split Big]", total,
				patched_intact_basic, patched_intact_big, patched_split_basic, patched_split_big);
		}
	}

	inline bool should_notify_on_exception() {
		return detail::should_notify_on_exception;
	}

	inline void allow_notification_on_exception(bool state) {
		detail::should_notify_on_exception = state;
	}

	inline void init_arxan_thread() {
		using namespace std::chrono;

		const auto timepoint = steady_clock::now();
		{
			detail::patch_integrity_checks();
			detail::patch_code_healers();
		}
		const auto elapsed_time = duration_cast<milliseconds>(steady_clock::now() - timepoint).count();

		LOG("Component/Arxan", DEBUG, "Created inline hooks for checksums in {}ms.", elapsed_time);
	}
}