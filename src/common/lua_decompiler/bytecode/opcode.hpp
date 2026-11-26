#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	enum class bytecode::opcode {
		op_islt,		// if A<VAR> < D<VAR> then JMP
		op_isge,		// if not (A<VAR> < D<VAR>) then JMP
		op_isle,		// if A<VAR> <= D<VAR> then JMP
		op_isgt,		// if not (A<VAR> <= D<VAR>) then JMP
		op_iseqv,		// if A<VAR> == D<VAR> then JMP
		op_isnev,		// if A<VAR> ~= D<VAR> then JMP
		op_iseqs,		// if A<VAR> == D<STR> then JMP
		op_isnes,		// if A<VAR> ~= D<STR> then JMP
		op_iseqn,		// if A<VAR> == D<NUM> then JMP
		op_isnen,		// if A<VAR> ~= D<NUM> then JMP
		op_iseqp,		// if A<VAR> == D<PRI> then JMP
		op_isnep,		// if A<VAR> ~= D<PRI> then JMP
		op_istc,		// if D<VAR> then A<DST> = D and JMP
		op_isfc,		// if not D<VAR> then A<DST> = D and JMP
		op_ist,			// if D<VAR> then JMP
		op_isf,			// if not D<VAR> then JMP
		op_istype,		// unsupported
		op_isnum,		// unsupported
		op_mov,			// A<DST> = D<VAR>
		op_not,			// A<DST> = not D<VAR>
		op_unm,			// A<DST> = -D<VAR>
		op_len,			// A<DST> = #D<VAR>
		op_addvn,		// A<DST> = B<VAR> + C<NUM>
		op_subvn,		// A<DST> = B<VAR> - C<NUM>
		op_mulvn,		// A<DST> = B<VAR> * C<NUM>
		op_divvn,		// A<DST> = B<VAR> / C<NUM>
		op_modvn,		// A<DST> = B<VAR> % C<NUM>
		op_addnv,		// A<DST> = C<NUM> + B<VAR>
		op_subnv,		// A<DST> = C<NUM> - B<VAR>
		op_mulnv,		// A<DST> = C<NUM> * B<VAR>
		op_divnv,		// A<DST> = C<NUM> / B<VAR>
		op_modnv,		// A<DST> = C<NUM> % B<VAR>
		op_addvv,		// A<DST> = B<VAR> + C<VAR>
		op_subvv,		// A<DST> = B<VAR> - C<VAR>
		op_mulvv,		// A<DST> = B<VAR> * C<VAR>
		op_divvv,		// A<DST> = B<VAR> / C<VAR>
		op_modvv,		// A<DST> = B<VAR> % C<VAR>
		op_pow,			// A<DST> = B<VAR> ^ C<VAR>
		op_cat,			// A<DST> = B<RBASE> .. B++ -> C<RBASE>
		op_kstr,		// A<DST> = D<STR>
		op_khash,		// A<DST> = D<HASH>
		op_kcdata,		// A<DST> = D<CDATA>
		op_kshort,		// A<DST> = D<LITS>
		op_knum,		// A<DST> = D<NUM>
		op_kpri,		// A<DST> = D<PRI>
		op_knil,		// A<BASE>, A++ -> D<BASE> = nil
		op_uget,		// A<DST> = D<UV>
		op_usetv,		// A<UV> = D<VAR>
		op_usets,		// A<UV> = D<STR>
		op_usetn,		// A<UV> = D<NUM>
		op_usetp,		// A<UV> = D<PRI>
		op_uclo,		// upvalue close for A<RBASE>, A++ -> framesize; goto D<JUMP>
		op_fnew,		// A<DST> = D<FUNC>
		op_tnew,		// A<DST> = {}
		op_tdup,		// A<DST> = D<TAB>
		op_gget,		// A<DST> = _G.D<STR>
		op_gset,		// _G.D<STR> = A<VAR>
		op_v84_unk1,	// unsupported
		op_v84_unk2,	// unsupported
		op_tgetv,		// A<DST> = B<VAR>[C<VAR>]
		op_tgets,		// A<DST> = B<VAR>[C<STR>]
		op_tgetb,		// A<DST> = B<VAR>[C<LIT>]
		op_tgetr,		// unsupported
		op_tsetv,		// B<VAR>[C<VAR>] = A<VAR>
		op_tsets,		// B<VAR>[C<STR>] = A<VAR>
		op_tsetb,		// B<VAR>[C<LIT>] = A<VAR>
		op_tsetm,		// A-1<BASE>[D&0xFFFFFFFF<NUM>] <- A (<- multres)
		op_tsetr,		// unsupported
		op_callm,		// if B<LIT> == 0 then A<BASE> (<- multres) <- A(A+FR2?2:1, A++ -> for C<LIT>, A++ (<- multres)) else A, A++ -> for B-1 = A(A+FR2?2:1, A++ -> for C, A++ (<- multres))
		op_call,		// if B<LIT> == 0 then A<BASE> (<- multres) <- A(A+FR2?2:1, A++ -> for C-1<LIT>) else A, A++ -> for B-1 = A(A+FR2?2:1, A++ -> for C-1)
		op_callmt,		// return A<BASE>(A+FR2?2:1, A++ -> for D<LIT>, A++ (<- multres))
		op_callt,		// return A<BASE>(A+FR2?2:1, A++ -> for D-1<LIT>)
		op_iterc,		// for A<BASE>, A++ -> for B-1<LIT> in A-3, A-2, A-1 do
		op_itern,		// for A<BASE>, A++ -> for B-1<LIT> in A-3, A-2, A-1 do
		op_varg,		// if B<LIT> == 0 then A<BASE> (<- multres) <- ... else A, A++ -> for B-1 = ...
		op_isnext,		// goto ITERN at D<JUMP>
		op_retm,		// return A<BASE>, A++ -> for D<LIT>, A++ (<- multres)
		op_ret,			// return A<RBASE>, A++ -> for D-1<LIT>
		op_ret0,		// return
		op_ret1,		// return A<RBASE>
		op_fori,		// for A+3<BASE> = A, A+1, A+2 do; exit at D<JUMP>
		op_jfori,		// unsupported
		op_forl,		// end of numeric for loop; start at D<JUMP>
		op_iforl,		// unsupported
		op_jforl,		// unsupported
		op_iterl,		// end of generic for loop; start at D<JUMP>
		op_iiterl,		// unsupported
		op_jiterl,		// unsupported
		op_loop,		// if D<JUMP> == 32767 then goto loop else while/repeat loop; exit at D
		op_iloop,		// unsupported
		op_jloop,		// unsupported
		op_jmp,			// goto D<JUMP> or if true then JMP or goto ITERC at D
		op_funcf,		// unsupported
		op_ifuncf,		// unsupported
		op_jfuncf,		// unsupported
		op_funcv,		// unsupported
		op_ifuncv,		// unsupported
		op_jfuncv,		// unsupported
		op_funcc,		// unsupported
		op_funccw,		// unsupported
		op_count,
		op_invalid = op_count
	};
}
