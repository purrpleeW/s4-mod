#pragma once
#include "common.hpp"
#include "component/gsc/lexer/script_value.hpp"

namespace gsc::scripting {
	class function {
	public:
		function(const char*);

		script_value get_raw() const;
		const char* get_pos() const;
		std::string get_name() const;
	private:
		const char* pos_;
	};
}
