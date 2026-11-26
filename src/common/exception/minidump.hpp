#pragma once
#include "utils/nt.hpp"

namespace exception {
	std::string create_minidump(LPEXCEPTION_POINTERS exception_info);
	void write_minidump(const LPEXCEPTION_POINTERS exception_info);
}
