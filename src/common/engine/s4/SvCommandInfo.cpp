#include "SvCommandInfo.hpp"

namespace s4 {
	SvCommandInfo* SvCommandInfo::get_next() {
		return CLASS_VALUE_AT_PTR(this, 0x0040, SvCommandInfo);
	}
}