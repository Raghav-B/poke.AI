#include "../common/System.h"
#include "GBA.h"
#include "GBAGlobals.h"
#include "GBAinline.h"
#include "GBACpu.h"

#ifdef PROFILING
#include "../prof/prof.h"
#endif

#ifdef _MSC_VER
// Disable "empty statement" warnings
 #pragma warning(disable: 4390)
// Visual C's inline assembler treats "offset" as a reserved word, so we
// tell it otherwise.  If you want to use it, write "OFFSET" in capitals.
 #define offset offset_
#endif

// Common macros //////////////////////////////////////////////////////////

#ifdef BKPT_SUPPORT
#define CONSOLE_OUTPUT(a, b) \
    do { \
		if ((opcode == 0xe0000000) && (reg[0].I == 0xC0DED00D)) {   \
			dbgOutput((a), (b));                                    \
		} while (0)
#else
#define CONSOLE_OUTPUT(a, b)  /* nothing */
#endif

#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)

// The following macros are used for optimization; any not defined for a
// particular compiler/CPU combination default to the C core versions.
//
//    ALU_INIT_C:   Used at the beginning of ALU instructions (AND/EOR/...).
//    (ALU_INIT_NC) Can consist of variable declarations, like the C core,
//                  or the start of a continued assembly block, like the
//                  x86-optimized version.  The _C version is used when the
//                  carry flag from the shift operation is needed (logical
//                  operations that set condition codes, like ANDS); the
//                  _NC version is used when the carry result is ignored.
//    VALUE_XXX: Retrieve the second operand's value for an ALU instruction.
//               The _C and _NC versions are used the same way as ALU_INIT.
//    OP_XXX: ALU operations.  XXX is the instruction name.
//    ALU_FINISH: Appended to all ALU instructions.  Usually empty, but if
//                ALU_INIT started a block ALU_FINISH can be used to end it
//                (as with the asm(...) statement in the x86 core).
//    SETCOND_NONE: Used in multiply instructions in place of SETCOND_MUL
//                  when the condition codes are not set.  Usually empty.
//    SETCOND_MUL: Used in multiply instructions to set the condition codes.
//    ROR_IMM_MSR: Used to rotate the immediate operand for MSR.
//    ROR_OFFSET: Used to rotate the `offset' parameter for LDR and STR
//                instructions.
//    RRX_OFFSET: Used to rotate (RRX) the `offset' parameter for LDR and
//                STR instructions.

#ifndef C_CORE

#if 0  // definitions have changed
//#ifdef __POWERPC__
			#define OP_SUBS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("subco. %0, %2, %3\n"              \
		              "mcrxr cr1\n"                       \
		              "mfcr %1\n"                         \
					  : "=r" (Result),                    \
		              "=r" (Flags)                      \
					  : "r" (reg[base].I),                \
		              "r" (value)                       \
		              );                                  \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_RSBS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("subfco. %0, %2, %3\n"             \
		              "mcrxr cr1\n"                       \
		              "mfcr %1\n"                         \
					  : "=r" (Result),                    \
		              "=r" (Flags)                      \
					  : "r" (reg[base].I),                \
		              "r" (value)                       \
		              );                                  \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_ADDS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("addco. %0, %2, %3\n"              \
		              "mcrxr cr1\n"                       \
		              "mfcr %1\n"                         \
					  : "=r" (Result),                    \
		              "=r" (Flags)                      \
					  : "r" (reg[base].I),                \
		              "r" (value)                       \
		              );                                  \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_ADCS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("mtspr xer, %4\n"                  \
		              "addeo. %0, %2, %3\n"              \
		              "mcrxr cr1\n"                      \
		              "mfcr      %1\n"                   \
					  : "=r" (Result),                   \
		              "=r" (Flags)                     \
					  : "r" (reg[base].I),               \
		              "r" (value),                     \
		              "r" (C_FLAG << 29)               \
		              );                                 \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_SBCS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("mtspr xer, %4\n"                  \
		              "subfeo. %0, %3, %2\n"             \
		              "mcrxr cr1\n"                      \
		              "mfcr      %1\n"                   \
					  : "=r" (Result),                   \
		              "=r" (Flags)                     \
					  : "r" (reg[base].I),               \
		              "r" (value),                     \
		              "r" (C_FLAG << 29)               \
		              );                                 \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_RSCS \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("mtspr xer, %4\n"                  \
		              "subfeo. %0, %2, %3\n"             \
		              "mcrxr cr1\n"                      \
		              "mfcr      %1\n"                   \
					  : "=r" (Result),                   \
		              "=r" (Flags)                     \
					  : "r" (reg[base].I),               \
		              "r" (value),                     \
		              "r" (C_FLAG << 29)               \
		              );                                 \
		reg[dest].I = Result;                           \
		Z_FLAG		= (Flags >> 29) & 1;                     \
		N_FLAG		= (Flags >> 31) & 1;                     \
		C_FLAG		= (Flags >> 25) & 1;                     \
		V_FLAG		= (Flags >> 26) & 1;                     \
	}
			#define OP_CMP \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("subco. %0, %2, %3\n"              \
		              "mcrxr cr1\n"                       \
		              "mfcr %1\n"                         \
					  : "=r" (Result),                    \
		              "=r" (Flags)                      \
					  : "r" (reg[base].I),                \
		              "r" (value)                       \
		              );                                  \
		Z_FLAG = (Flags >> 29) & 1;                     \
		N_FLAG = (Flags >> 31) & 1;                     \
		C_FLAG = (Flags >> 25) & 1;                     \
		V_FLAG = (Flags >> 26) & 1;                     \
	}
			#define OP_CMN \
	{ \
		register int Flags;                             \
		register int Result;                            \
		asm volatile ("addco. %0, %2, %3\n"              \
		              "mcrxr cr1\n"                       \
		              "mfcr %1\n"                         \
					  : "=r" (Result),                    \
		              "=r" (Flags)                      \
					  : "r" (reg[base].I),                \
		              "r" (value)                       \
		              );                                  \
		Z_FLAG = (Flags >> 29) & 1;                     \
		N_FLAG = (Flags >> 31) & 1;                     \
		C_FLAG = (Flags >> 25) & 1;                     \
		V_FLAG = (Flags >> 26) & 1;                     \
	}

#else  // !__POWERPC__

// Macros to emit instructions in the format used by the particular compiler.
// We use GNU assembler syntax: "op src, dest" rather than "op dest, src"

#ifdef __GNUC__
 #define ALU_HEADER           asm ("mov %%ecx, %%edi; "
 #define ALU_TRAILER          : "=D" (opcode) : "c" (opcode) : "eax", "ebx", "edx", "esi")
 #define EMIT0(op)            # op "; "
 #define EMIT1(op, arg)        # op " " arg "; "
 #define EMIT2(op, src, dest)   # op " " src ", " dest "; "
 #define CONST(val)           "$" # val
 #define ASMVAR(cvar)         ASMVAR2(__USER_LABEL_PREFIX__, cvar)
 #define ASMVAR2(prefix, cvar) STRING(prefix) cvar
 #define STRING(x)            # x
 #define VAR(var)             ASMVAR(# var)
 #define VARL(var)            ASMVAR(# var)
 #define REGREF1(index)       ASMVAR("reg(" index ")")
 #define REGREF2(index, scale) ASMVAR("reg(," index "," # scale ")")
 #define LABEL(n)             # n ": "
 #define LABELREF(n, dir)      # n # dir
 #define al "%%al"
 #define ah "%%ah"
 #define eax "%%eax"
 #define bl "%%bl"
 #define bh "%%bh"
 #define ebx "%%ebx"
 #define cl "%%cl"
 #define ch "%%ch"
 #define ecx "%%ecx"
 #define dl "%%dl"
 #define dh "%%dh"
 #define edx "%%edx"
 #define esp "%%esp"
 #define ebp "%%ebp"
 #define esi "%%esi"
 #define edi "%%edi"
 #define movzx movzb
#else
 #define ALU_HEADER           __asm { __asm mov ecx, opcode
 #define ALU_TRAILER          }
 #define EMIT0(op)            __asm op
 #define EMIT1(op, arg)        __asm op arg
 #define EMIT2(op, src, dest)   __asm op dest, src
 #define CONST(val)           val
 #define VAR(var)             var
 #define VARL(var)            dword ptr var
 #define REGREF1(index)       reg[index]
 #define REGREF2(index, scale) reg[index * scale]
 #define LABEL(n)             __asm l ## n :
 #define LABELREF(n, dir)      l ## n
#endif

//X//#ifndef _MSC_VER
// ALU op register usage:
//    EAX -> 2nd operand value, result (RSB/RSC)
//    EBX -> C_OUT (carry flag from shift/rotate)
//    ECX -> opcode (input), shift/rotate count
//    EDX -> Rn (base) value, result (all except RSB/RSC)
//    ESI -> Rd (destination) index * 4

// Helper macros for loading value / shift count
#define VALUE_LOAD_IMM \
    EMIT2(and, CONST(0x0F), eax)            \
    EMIT2(mov, REGREF2(eax, 4), eax)         \
    EMIT2(shr, CONST(7), ecx)               \
    EMIT2(and, CONST(0x1F), ecx)
#define VALUE_LOAD_REG \
    EMIT2(and, CONST(0x0F), eax)            \
    EMIT2(mov, REGREF2(eax, 4), eax)         \
    EMIT2(movzx, ch, ecx)                   \
    EMIT2(and, CONST(0x0F), ecx)            \
    EMIT2(mov, REGREF2(ecx, 4), ecx)

// Helper macros for setting flags
#define SETCOND_LOGICAL \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT2(mov, bl, VAR(C_FLAG))
#define SETCOND_ADD \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT1(seto, VAR(V_FLAG))            \
    EMIT1(setc, VAR(C_FLAG))
#define SETCOND_SUB \
    EMIT1(sets, VAR(N_FLAG))            \
    EMIT1(setz, VAR(Z_FLAG))            \
    EMIT1(seto, VAR(V_FLAG))            \
    EMIT1(setnc, VAR(C_FLAG))

// ALU initialization
#define ALU_INIT(LOAD_C_FLAG) \
    ALU_HEADER                          \
    LOAD_C_FLAG                         \
    EMIT2(mov, ecx, edx)                \
    EMIT2(shr, CONST(14), edx)          \
    EMIT2(mov, ecx, eax)                \
    EMIT2(mov, ecx, esi)                \
    EMIT2(shr, CONST(10), esi)          \
    EMIT2(and, CONST(0x3C), edx)        \
    EMIT2(mov, REGREF1(edx), edx)       \
    EMIT2(and, CONST(0x3C), esi)

#define LOAD_C_FLAG_YES EMIT2(mov, VAR(C_FLAG), bl)
#define LOAD_C_FLAG_NO  /*nothing*/
#define ALU_INIT_C ALU_INIT(LOAD_C_FLAG_YES)
#define ALU_INIT_NC ALU_INIT(LOAD_C_FLAG_NO)

// Macros to load the value operand for an ALU op; these all set N/Z
// according to the value

// OP Rd,Rb,Rm LSL #
#define VALUE_LSL_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jnz, LABELREF(1, f))           \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(shl, cl, eax)                 \
    EMIT1(setc, bl)                     \
    LABEL(0)
#define VALUE_LSL_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT2(shl, cl, eax)

// OP Rd,Rb,Rm LSL Rs
#define VALUE_LSL_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0, f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(je, LABELREF(1, f))            \
    EMIT1(ja, LABELREF(2, f))            \
    EMIT2(shl, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(test, CONST(1), al)           \
    EMIT1(setnz, bl)                    \
    EMIT2(xor, eax, eax)                \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(2)                            \
    EMIT2(xor, ebx, ebx)                \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSL_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1, f))           \
    EMIT2(shl, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm LSR #
#define VALUE_LSR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(test, eax, eax)               \
    EMIT1(sets, bl)                     \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm LSR Rs
#define VALUE_LSR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0, f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(je, LABELREF(1, f))            \
    EMIT1(ja, LABELREF(2, f))            \
    EMIT2(shr, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(test, eax, eax)               \
    EMIT1(sets, bl)                     \
    EMIT2(xor, eax, eax)                \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(2)                            \
    EMIT2(xor, ebx, ebx)                \
    EMIT2(xor, eax, eax)                \
    LABEL(0)
#define VALUE_LSR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1, f))           \
    EMIT2(shr, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(xor, eax, eax)                \
    LABEL(0)

// OP Rd,Rb,Rm ASR #
#define VALUE_ASR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(sar, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    EMIT1(sets, bl)                     \
    LABEL(0)
#define VALUE_ASR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(sar, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    LABEL(0)

// OP Rd,Rb,Rm ASR Rs
#define VALUE_ASR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(test, cl, cl)                 \
    EMIT1(jz, LABELREF(0, f))            \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1, f))           \
    EMIT2(sar, cl, eax)                 \
    EMIT1(setc, bl)                     \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    EMIT1(sets, bl)                     \
    LABEL(0)
#define VALUE_ASR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(cmp, CONST(0x20), cl)         \
    EMIT1(jae, LABELREF(1, f))           \
    EMIT2(sar, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(sar, CONST(31), eax)          \
    LABEL(0)

// OP Rd,Rb,Rm ROR #
#define VALUE_ROR_IMM_C \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(rcr, CONST(1), eax)           \
    LABEL(0)                            \
    EMIT1(setc, bl)
#define VALUE_ROR_IMM_NC \
    VALUE_LOAD_IMM                      \
    EMIT1(jz, LABELREF(1, f))            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(jmp, LABELREF(0, f))           \
    LABEL(1)                            \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT2(rcr, CONST(1), eax)           \
    LABEL(0)

// OP Rd,Rb,Rm ROR Rs
#define VALUE_ROR_REG_C \
    VALUE_LOAD_REG                      \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(setc, bl)
#define VALUE_ROR_REG_NC \
    VALUE_LOAD_REG                      \
    EMIT2(ror, cl, eax)

// OP Rd,Rb,# ROR #
#define VALUE_IMM_C \
    EMIT2(movzx, ch, ecx)               \
    EMIT2(add, ecx, ecx)                \
    EMIT2(movzx, al, eax)               \
    EMIT2(bt, CONST(0), ebx)            \
    EMIT2(ror, cl, eax)                 \
    EMIT1(setc, bl)
#define VALUE_IMM_NC \
    EMIT2(movzx, ch, ecx)               \
    EMIT2(add, ecx, ecx)                \
    EMIT2(movzx, al, eax)               \
    EMIT2(ror, cl, eax)

// Macros to perform ALU ops

// Set condition codes iff the destination register is not R15 (PC)
#define CHECK_PC(OP, SETCOND) \
    EMIT2(cmp, CONST(0x3C), esi)        \
    EMIT1(je, LABELREF(8, f))            \
    OP SETCOND                          \
    EMIT1(jmp, LABELREF(9, f))           \
    LABEL(8)                            \
    OP                                  \
    LABEL(9)

#define OP_AND \
    EMIT2(and, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ANDS   CHECK_PC(OP_AND, SETCOND_LOGICAL)
#define OP_EOR \
    EMIT2(xor, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_EORS   CHECK_PC(OP_EOR, SETCOND_LOGICAL)
#define OP_SUB \
    EMIT2(sub, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_SUBS   CHECK_PC(OP_SUB, SETCOND_SUB)
#define OP_RSB \
    EMIT2(sub, edx, eax)                \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_RSBS   CHECK_PC(OP_RSB, SETCOND_SUB)
#define OP_ADD \
    EMIT2(add, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ADDS   CHECK_PC(OP_ADD, SETCOND_ADD)
#define OP_ADC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT2(adc, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ADCS   CHECK_PC(OP_ADC, SETCOND_ADD)
#define OP_SBC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT0(cmc)                          \
    EMIT2(sbb, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_SBCS   CHECK_PC(OP_SBC, SETCOND_SUB)
#define OP_RSC \
    EMIT2(bt, CONST(0), VARL(C_FLAG))   \
    EMIT0(cmc)                          \
    EMIT2(sbb, edx, eax)                \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_RSCS   CHECK_PC(OP_RSC, SETCOND_SUB)
#define OP_TST \
    EMIT2(and, eax, edx)                \
    SETCOND_LOGICAL
#define OP_TEQ \
    EMIT2(xor, eax, edx)                \
    SETCOND_LOGICAL
#define OP_CMP \
    EMIT2(sub, eax, edx)                \
    SETCOND_SUB
#define OP_CMN \
    EMIT2(add, eax, edx)                \
    SETCOND_ADD
#define OP_ORR \
    EMIT2(or, eax, edx)                 \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_ORRS   CHECK_PC(OP_ORR, SETCOND_LOGICAL)
#define OP_MOV \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_MOVS   CHECK_PC(EMIT2(test, eax, eax) EMIT2(mov, eax, REGREF1(esi)), SETCOND_LOGICAL)
#define OP_BIC \
    EMIT1(not, eax)                     \
    EMIT2(and, eax, edx)                \
    EMIT2(mov, edx, REGREF1(esi))
#define OP_BICS   CHECK_PC(OP_BIC, SETCOND_LOGICAL)
#define OP_MVN \
    EMIT1(not, eax)                     \
    EMIT2(mov, eax, REGREF1(esi))
#define OP_MVNS   CHECK_PC(OP_MVN EMIT2(test, eax, eax), SETCOND_LOGICAL)

// ALU cleanup macro
#define ALU_FINISH  ALU_TRAILER

// End of ALU macros
//X//#endif //_MSC_VER

#ifdef __GNUC__

#define ROR_IMM_MSR \
    asm ("ror %%cl, %%eax;"             \
		 : "=a" (value)                 \
		 : "a" (opcode & 0xFF), "c" (shift));

#define ROR_OFFSET \
    asm ("ror %%cl, %0"                  \
		 : "=r" (offset)                 \
		 : "0" (offset), "c" (shift));

#define RRX_OFFSET \
    asm (EMIT2(btl, CONST(0), VAR(C_FLAG)) \
         "rcr $1, %0"                    \
		 : "=r" (offset)                 \
		 : "0" (offset));

#else  // !__GNUC__, i.e. Visual C++

#define ROR_IMM_MSR \
    __asm {                             \
        __asm mov ecx, shift            \
        __asm ror value, cl             \
	}

#define ROR_OFFSET \
    __asm {                             \
        __asm mov ecx, shift            \
        __asm ror offset, cl            \
	}

#define RRX_OFFSET \
    __asm {                             \
        __asm bt dword ptr C_FLAG, 0    \
        __asm rcr offset, 1             \
	}

#endif  // !__GNUC__

#endif  // !__POWERPC__
#endif  // !C_CORE

// C core

#define C_SETCOND_LOGICAL \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    C_FLAG = C_OUT;
#define C_SETCOND_ADD \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    V_FLAG = ((NEG(lhs) & NEG(rhs) & POS(res)) |        \
              (POS(lhs) & POS(rhs) & NEG(res))) ? true : false; \
    C_FLAG = ((NEG(lhs) & NEG(rhs)) |                   \
              (NEG(lhs) & POS(res)) |                   \
              (NEG(rhs) & POS(res))) ? true : false;
#define C_SETCOND_SUB \
    N_FLAG = ((s32)res < 0) ? true : false;             \
    Z_FLAG = (res == 0) ? true : false;                 \
    V_FLAG = ((NEG(lhs) & POS(rhs) & POS(res)) |        \
              (POS(lhs) & NEG(rhs) & NEG(res))) ? true : false; \
    C_FLAG = ((NEG(lhs) & POS(rhs)) |                   \
              (NEG(lhs) & POS(res)) |                   \
              (POS(rhs) & POS(res))) ? true : false;

#ifndef ALU_INIT_C
 #define ALU_INIT_C \
    int dest = (opcode >> 12) & 15;                       \
    bool C_OUT = C_FLAG;                                \
    u32 value;
#endif
// OP Rd,Rb,Rm LSL #
#ifndef VALUE_LSL_IMM_C
 #define VALUE_LSL_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(!shift)) {  /* LSL #0 most common? */    \
		value = reg[opcode & 0x0F].I;                   \
	} else {                                            \
		u32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v >> (32 - shift)) & 1 ? true : false; \
		value = v << shift;                             \
	}
#endif
// OP Rd,Rb,Rm LSL Rs
#ifndef VALUE_LSL_REG_C
 #define VALUE_LSL_REG_C \
    unsigned int shift = reg[(opcode >> 8) & 15].B.B0;    \
    if (LIKELY(shift)) {                                \
		if (shift == 32) {                              \
			value = 0;                                  \
			C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false); \
		} else if (LIKELY(shift < 32)) {                \
			u32 v = reg[opcode & 0x0F].I;               \
			C_OUT = (v >> (32 - shift)) & 1 ? true : false; \
			value = v << shift;                         \
		} else {                                        \
			value = 0;                                  \
			C_OUT = false;                              \
		}                                               \
	} else {                                            \
		value = reg[opcode & 0x0F].I;                   \
	}
#endif
// OP Rd,Rb,Rm LSR #
#ifndef VALUE_LSR_IMM_C
 #define VALUE_LSR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
		u32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
		value = v >> shift;                             \
	} else {                                            \
		value = 0;                                      \
		C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false; \
	}
#endif
// OP Rd,Rb,Rm LSR Rs
#ifndef VALUE_LSR_REG_C
 #define VALUE_LSR_REG_C \
    unsigned int shift = reg[(opcode >> 8) & 15].B.B0;    \
    if (LIKELY(shift)) {                                \
		if (shift == 32) {                              \
			value = 0;                                  \
			C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false); \
		} else if (LIKELY(shift < 32)) {                \
			u32 v = reg[opcode & 0x0F].I;               \
			C_OUT = (v >> (shift - 1)) & 1 ? true : false; \
			value = v >> shift;                         \
		} else {                                        \
			value = 0;                                  \
			C_OUT = false;                              \
		}                                               \
	} else {                                            \
		value = reg[opcode & 0x0F].I;                   \
	}
#endif
// OP Rd,Rb,Rm ASR #
#ifndef VALUE_ASR_IMM_C
 #define VALUE_ASR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
		/* VC++ BUG: u32 v; (s32)v>>n is optimized to shr! */ \
		s32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v >> (int)(shift - 1)) & 1 ? true : false; \
		value = v >> (int)shift;                        \
	} else {                                            \
		if (reg[opcode & 0x0F].I & 0x80000000) {        \
			value = 0xFFFFFFFF;                         \
			C_OUT = true;                               \
		} else {                                        \
			value = 0;                                  \
			C_OUT = false;                              \
		}                                               \
	}
#endif
// OP Rd,Rb,Rm ASR Rs
#ifndef VALUE_ASR_REG_C
 #define VALUE_ASR_REG_C \
    unsigned int shift = reg[(opcode >> 8) & 15].B.B0;    \
    if (LIKELY(shift < 32)) {                           \
		if (LIKELY(shift)) {                            \
			s32 v = reg[opcode & 0x0F].I;               \
			C_OUT = (v >> (int)(shift - 1)) & 1 ? true : false; \
			value = v >> (int)shift;                    \
		} else {                                        \
			value = reg[opcode & 0x0F].I;               \
		}                                               \
	} else {                                            \
		if (reg[opcode & 0x0F].I & 0x80000000) {        \
			value = 0xFFFFFFFF;                         \
			C_OUT = true;                               \
		} else {                                        \
			value = 0;                                  \
			C_OUT = false;                              \
		}                                               \
	}
#endif
// OP Rd,Rb,Rm ROR #
#ifndef VALUE_ROR_IMM_C
 #define VALUE_ROR_IMM_C \
    unsigned int shift = (opcode >> 7) & 0x1F;          \
    if (LIKELY(shift)) {                                \
		u32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
		value = ((v << (32 - shift)) |                  \
		         (v >> shift));                         \
	} else {                                            \
		u32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v & 1) ? true : false;                 \
		value = ((v >> 1) |                             \
		         (C_FLAG << 31));                       \
	}
#endif
// OP Rd,Rb,Rm ROR Rs
#ifndef VALUE_ROR_REG_C
 #define VALUE_ROR_REG_C \
    unsigned int shift = reg[(opcode >> 8) & 15].B.B0;    \
    if (LIKELY(shift & 0x1F)) {                         \
		u32 v = reg[opcode & 0x0F].I;                   \
		C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
		value = ((v << (32 - shift)) |                  \
		         (v >> shift));                         \
	} else {                                            \
		value = reg[opcode & 0x0F].I;                   \
		if (shift)                                      \
			C_OUT = (value & 0x80000000 ? true : false); \
	}
#endif
// OP Rd,Rb,# ROR #
#ifndef VALUE_IMM_C
 #define VALUE_IMM_C \
    int shift = (opcode & 0xF00) >> 7;                  \
    if (UNLIKELY(shift)) {                              \
		u32 v = opcode & 0xFF;                          \
		C_OUT = (v >> (shift - 1)) & 1 ? true : false;  \
		value = ((v << (32 - shift)) |                  \
		         (v >> shift));                         \
	} else {                                            \
		value = opcode & 0xFF;                          \
	}
#endif

// Make the non-carry versions default to the carry versions
// (this is fine for C--the compiler will optimize the dead code out)
#ifndef ALU_INIT_NC
 #define ALU_INIT_NC ALU_INIT_C
#endif
#ifndef VALUE_LSL_IMM_NC
 #define VALUE_LSL_IMM_NC VALUE_LSL_IMM_C
#endif
#ifndef VALUE_LSL_REG_NC
 #define VALUE_LSL_REG_NC VALUE_LSL_REG_C
#endif
#ifndef VALUE_LSR_IMM_NC
 #define VALUE_LSR_IMM_NC VALUE_LSR_IMM_C
#endif
#ifndef VALUE_LSR_REG_NC
 #define VALUE_LSR_REG_NC VALUE_LSR_REG_C
#endif
#ifndef VALUE_ASR_IMM_NC
 #define VALUE_ASR_IMM_NC VALUE_ASR_IMM_C
#endif
#ifndef VALUE_ASR_REG_NC
 #define VALUE_ASR_REG_NC VALUE_ASR_REG_C
#endif
#ifndef VALUE_ROR_IMM_NC
 #define VALUE_ROR_IMM_NC VALUE_ROR_IMM_C
#endif
#ifndef VALUE_ROR_REG_NC
 #define VALUE_ROR_REG_NC VALUE_ROR_REG_C
#endif
#ifndef VALUE_IMM_NC
 #define VALUE_IMM_NC VALUE_IMM_C
#endif

#define C_CHECK_PC(SETCOND) if (LIKELY(dest != 15)) { SETCOND }
#ifndef OP_AND
 #define OP_AND \
    u32 res		= reg[(opcode >> 16) & 15].I & value;           \
    reg[dest].I = res;
#endif
#ifndef OP_ANDS
 #define OP_ANDS   OP_AND C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_EOR
 #define OP_EOR \
    u32 res		= reg[(opcode >> 16) & 15].I ^ value;           \
    reg[dest].I = res;
#endif
#ifndef OP_EORS
 #define OP_EORS   OP_EOR C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_SUB
 #define OP_SUB \
    u32 lhs = reg[(opcode >> 16) & 15].I;               \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_SUBS
 #define OP_SUBS   OP_SUB C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_RSB
 #define OP_RSB \
    u32 lhs = value;                                    \
    u32 rhs = reg[(opcode >> 16) & 15].I;               \
    u32 res = lhs - rhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_RSBS
 #define OP_RSBS   OP_RSB C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_ADD
 #define OP_ADD \
    u32 lhs = reg[(opcode >> 16) & 15].I;               \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs;                                \
    reg[dest].I = res;
#endif
#ifndef OP_ADDS
 #define OP_ADDS   OP_ADD C_CHECK_PC(C_SETCOND_ADD)
#endif
#ifndef OP_ADC
 #define OP_ADC \
    u32 lhs = reg[(opcode >> 16) & 15].I;               \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs + (u32)C_FLAG;                  \
    reg[dest].I = res;
#endif
#ifndef OP_ADCS
 #define OP_ADCS   OP_ADC C_CHECK_PC(C_SETCOND_ADD)
#endif
#ifndef OP_SBC
 #define OP_SBC \
    u32 lhs = reg[(opcode >> 16) & 15].I;               \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs - !((u32)C_FLAG);               \
    reg[dest].I = res;
#endif
#ifndef OP_SBCS
 #define OP_SBCS   OP_SBC C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_RSC
 #define OP_RSC \
    u32 lhs = value;                                    \
    u32 rhs = reg[(opcode >> 16) & 15].I;               \
    u32 res = lhs - rhs - !((u32)C_FLAG);               \
    reg[dest].I = res;
#endif
#ifndef OP_RSCS
 #define OP_RSCS   OP_RSC C_CHECK_PC(C_SETCOND_SUB)
#endif
#ifndef OP_TST
 #define OP_TST \
    u32 res = reg[(opcode >> 16) & 0x0F].I & value;     \
    C_SETCOND_LOGICAL;
#endif
#ifndef OP_TEQ
 #define OP_TEQ \
    u32 res = reg[(opcode >> 16) & 0x0F].I ^ value;     \
    C_SETCOND_LOGICAL;
#endif
#ifndef OP_CMP
 #define OP_CMP \
    u32 lhs = reg[(opcode >> 16) & 15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs - rhs;                                \
    C_SETCOND_SUB;
#endif
#ifndef OP_CMN
 #define OP_CMN \
    u32 lhs = reg[(opcode >> 16) & 15].I;                   \
    u32 rhs = value;                                    \
    u32 res = lhs + rhs;                                \
    C_SETCOND_ADD;
#endif
#ifndef OP_ORR
 #define OP_ORR \
    u32 res		= reg[(opcode >> 16) & 0x0F].I | value;     \
    reg[dest].I = res;
#endif
#ifndef OP_ORRS
 #define OP_ORRS   OP_ORR C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_MOV
 #define OP_MOV \
    u32 res		= value;                                    \
    reg[dest].I = res;
#endif
#ifndef OP_MOVS
 #define OP_MOVS   OP_MOV C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_BIC
 #define OP_BIC \
    u32 res		= reg[(opcode >> 16) & 0x0F].I & (~value);  \
    reg[dest].I = res;
#endif
#ifndef OP_BICS
 #define OP_BICS   OP_BIC C_CHECK_PC(C_SETCOND_LOGICAL)
#endif
#ifndef OP_MVN
 #define OP_MVN \
    u32 res		= ~value;                                   \
    reg[dest].I = res;
#endif
#ifndef OP_MVNS
 #define OP_MVNS   OP_MVN C_CHECK_PC(C_SETCOND_LOGICAL)
#endif

#ifndef SETCOND_NONE
 #define SETCOND_NONE /*nothing*/
#endif
#ifndef SETCOND_MUL
 #define SETCOND_MUL \
    N_FLAG = ((s32)reg[dest].I < 0) ? true : false;    \
    Z_FLAG = reg[dest].I ? false : true;
#endif
#ifndef SETCOND_MULL
 #define SETCOND_MULL \
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false; \
    Z_FLAG = reg[dest].I || reg[acc].I ? false : true;
#endif

#ifndef ALU_FINISH
 #define ALU_FINISH /*nothing*/
#endif

#ifndef ROR_IMM_MSR
 #define ROR_IMM_MSR \
    u32 v = opcode & 0xff;                              \
    value = ((v << (32 - shift)) | (v >> shift));
#endif
#ifndef ROR_OFFSET
 #define ROR_OFFSET \
    offset = ((offset << (32 - shift)) | (offset >> shift));
#endif
#ifndef RRX_OFFSET
 #define RRX_OFFSET \
    offset = ((offset >> 1) | ((int)C_FLAG << 31));
#endif
