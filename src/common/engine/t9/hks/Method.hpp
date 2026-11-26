#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hksInstruction.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"
#include "engine/t9/hks/InternString.hpp"

namespace t9::hks {
	class Method : public ChunkHeader {
	public:
		template <typename T>
		class Array {
		public:
			hksUint32 size_;			// 0x0000
		private:
			char pad_0004[0x0004];		// 0x0004
		public:
			T* data_;					// 0x0008
		};

		class LocalInfo {
		public:
			hks::InternString* name_;	// 0x0000
			hksInt32 start_pc_;			// 0x0008
			hksInt32 end_pc_;			// 0x000C
		};

		using Children = Array<Method*>;
		using Constants = Array<HksObject>;
		using Instructions = Array<hksInstruction>;
		using LineInfo = Array<hksUint32>;
		using Locals = Array<LocalInfo>;
		using UpValueInfo = Array<InternString*>;

		class DebugInfo {
		public:
			hksUint32 line_defined_;
			hksUint32 last_line_defined_;
			LineInfo line_info_;
			UpValueInfo upval_info_;
			InternString* source_;
			InternString* name_;
			Locals local_info_;
		};

		hksUint32 hash_;
		hksUshort16 num_upvals_;
		hksUshort16 num_registers_;
		hksByte num_params_;
		hksByte flags_;
		Instructions instructions_;
		Constants constants_;
		Children children_;
		DebugInfo* debug_;
	};
}
