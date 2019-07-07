#include "../common/System.h"
#include "GBA.h"
#include "GBAGlobals.h"
#include "GBAinline.h"
#include "GBACpu.h"

#ifdef PROFILING
#include "../prof/prof.h"
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

typedef INSN_REGPARM void (*insnfunc_t)(u32 opcode);

#define thumbUI thumbUnknownInsn
#ifdef BKPT_SUPPORT
 #define thumbBP thumbBreakpoint
#else
 #define thumbBP thumbUnknownInsn
#endif

// Common macros //////////////////////////////////////////////////////////

#ifdef BKPT_SUPPORT
# define THUMB_CONSOLE_OUTPUT(a, b) \
    do {                     \
		if ((opcode == 0x4000) && (reg[0].I == 0xC0DED00D)) {   \
			dbgOutput((a), (b));                                  \
		}                                                       \
	} while (0)
# define UPDATE_OLDREG \
    do {                                 \
		if (debugger_last) {                                    \
			snprintf(oldbuffer, sizeof(oldbuffer), "%08X",      \
			         armState ? reg[15].I - 4 : reg[15].I - 2); \
			int i;                          \
			for (i = 0; i < 18; i++) {                          \
				oldreg[i] = reg[i].I;                           \
			}                                                   \
		}                                                       \
	} while (0)
#else
# define THUMB_CONSOLE_OUTPUT(a, b)
# define UPDATE_OLDREG
#endif

#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)

#ifndef C_CORE
#ifdef __GNUC__
#ifdef __POWERPC__
  #define ADD_RD_RS_RN(N) \
	{                                       \
		register int Flags;                 \
		register int Result;                \
		asm volatile ("addco. %0, %2, %3\n"  \
		              "mcrxr cr1\n"           \
		              "mfcr %1\n"             \
					  : "=r" (Result),        \
		              "=r" (Flags)          \
					  : "r" (reg[source].I),  \
		              "r" (reg[N].I)        \
		              );                      \
		reg[dest].I = Result;               \
		Z_FLAG		= (Flags >> 29) & 1;         \
		N_FLAG		= (Flags >> 31) & 1;         \
		C_FLAG		= (Flags >> 25) & 1;         \
		V_FLAG		= (Flags >> 26) & 1;         \
	}
  #define ADD_RD_RS_O3(N) \
	{                                       \
		register int Flags;                 \
		register int Result;                \
		asm volatile ("addco. %0, %2, %3\n"  \
		              "mcrxr cr1\n"           \
		              "mfcr %1\n"             \
					  : "=r" (Result),        \
		              "=r" (Flags)          \
					  : "r" (reg[source].I),  \
		              "r" (N)               \
		              );                      \
		reg[dest].I = Result;               \
		Z_FLAG		= (Flags >> 29) & 1;         \
		N_FLAG		= (Flags >> 31) & 1;         \
		C_FLAG		= (Flags >> 25) & 1;         \
		V_FLAG		= (Flags >> 26) & 1;         \
	}
  #define ADD_RD_RS_O3_0 ADD_RD_RS_O3
  #define ADD_RN_O8(d) \
	{ \
		register int Flags;                 \
		register int Result;                \
		asm volatile ("addco. %0, %2, %3\n"  \
		              "mcrxr cr1\n"           \
		              "mfcr %1\n"             \
					  : "=r" (Result),        \
		              "=r" (Flags)          \
					  : "r" (reg[(d)].I),     \
		              "r" (opcode & 255)    \
		              );                      \
		reg[(d)].I = Result;                \
		Z_FLAG	   = (Flags >> 29) & 1;         \
		N_FLAG	   = (Flags >> 31) & 1;         \
		C_FLAG	   = (Flags >> 25) & 1;         \
		V_FLAG	   = (Flags >> 26) & 1;         \
	}
  #define CMN_RD_RS \
	{ \
		register int Flags;                 \
		register int Result;                \
		asm volatile ("addco. %0, %2, %3\n"  \
		              "mcrxr cr1\n"           \
		              "mfcr %1\n"             \
					  : "=r" (Result),        \
		              "=r" (Flags)          \
					  : "r" (reg[dest].I),    \
		              "r" (value)           \
		              );                      \
		Z_FLAG = (Flags >> 29) & 1;         \
		N_FLAG = (Flags >> 31) & 1;         \
		C_FLAG = (Flags >> 25) & 1;         \
		V_FLAG = (Flags >> 26) & 1;         \
	}
  #define ADC_RD_RS \
	{ \
		register int Flags;                 \
		register int Result;                \
		asm volatile ("mtspr 1, %4\n"        \       /* reg 1 is xer */
"addeo. %0, %2, %3\n"  \
"mcrxr cr1\n"          \
"mfcr	%1\n"            \
	: "=r" (Result),       \
    "=r" (Flags)         \
	: "r" (reg[dest].I),   \
    "r" (value),         \
    "r" (C_FLAG << 29)   \
         );                     \
    reg[dest].I = Result;               \
    Z_FLAG = (Flags >> 29) & 1;         \
    N_FLAG = (Flags >> 31) & 1;         \
    C_FLAG = (Flags >> 25) & 1;         \
    V_FLAG = (Flags >> 26) & 1;         \
	}
  #define SUB_RD_RS_RN(N) \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subco. %0, %2, %3\n"  \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[source].I),  \
                      "r" (reg[N].I)        \
                      );                      \
        reg[dest].I = Result;               \
        Z_FLAG		= (Flags >> 29) & 1;         \
        N_FLAG		= (Flags >> 31) & 1;         \
        C_FLAG		= (Flags >> 25) & 1;         \
        V_FLAG		= (Flags >> 26) & 1;         \
	}
  #define SUB_RD_RS_O3(N) \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subco. %0, %2, %3\n"  \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[source].I),  \
                      "r" (N)               \
                      );                      \
        reg[dest].I = Result;               \
        Z_FLAG		= (Flags >> 29) & 1;         \
        N_FLAG		= (Flags >> 31) & 1;         \
        C_FLAG		= (Flags >> 25) & 1;         \
        V_FLAG		= (Flags >> 26) & 1;         \
	}
  #define SUB_RD_RS_O3_0 SUB_RD_RS_O3
  #define SUB_RN_O8(d) \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subco. %0, %2, %3\n"  \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[(d)].I),     \
                      "r" (opcode & 255)    \
                      );                      \
        reg[(d)].I = Result;                \
        Z_FLAG	   = (Flags >> 29) & 1;         \
        N_FLAG	   = (Flags >> 31) & 1;         \
        C_FLAG	   = (Flags >> 25) & 1;         \
        V_FLAG	   = (Flags >> 26) & 1;         \
	}
  #define CMP_RN_O8(d) \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subco. %0, %2, %3\n"  \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[(d)].I),     \
                      "r" (opcode & 255)    \
                      );                      \
        Z_FLAG = (Flags >> 29) & 1;         \
        N_FLAG = (Flags >> 31) & 1;         \
        C_FLAG = (Flags >> 25) & 1;         \
        V_FLAG = (Flags >> 26) & 1;         \
	}
  #define SBC_RD_RS \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("mtspr 1, %4\n"        \       /* reg 1 is xer */
"subfeo. %0, %3, %2\n" \
"mcrxr cr1\n"          \
"mfcr	%1\n"            \
	: "=r" (Result),       \
    "=r" (Flags)         \
	: "r" (reg[dest].I),   \
    "r" (value),         \
    "r" (C_FLAG << 29)   \
         );                     \
    reg[dest].I = Result;               \
    Z_FLAG = (Flags >> 29) & 1;         \
    N_FLAG = (Flags >> 31) & 1;         \
    C_FLAG = (Flags >> 25) & 1;         \
    V_FLAG = (Flags >> 26) & 1;         \
	}
  #define NEG_RD_RS \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subfco. %0, %2, %3\n" \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[source].I),  \
                      "r" (0)               \
                      );                      \
        reg[dest].I = Result;               \
        Z_FLAG		= (Flags >> 29) & 1;         \
        N_FLAG		= (Flags >> 31) & 1;         \
        C_FLAG		= (Flags >> 25) & 1;         \
        V_FLAG		= (Flags >> 26) & 1;         \
	}
  #define CMP_RD_RS \
    { \
        register int Flags;                 \
        register int Result;                \
        asm volatile ("subco. %0, %2, %3\n"  \
                      "mcrxr cr1\n"           \
                      "mfcr %1\n"             \
					  : "=r" (Result),        \
                      "=r" (Flags)          \
					  : "r" (reg[dest].I),    \
                      "r" (value)           \
                      );                      \
        Z_FLAG = (Flags >> 29) & 1;         \
        N_FLAG = (Flags >> 31) & 1;         \
        C_FLAG = (Flags >> 25) & 1;         \
        V_FLAG = (Flags >> 26) & 1;         \
	}
#else
  #define EMIT1(op, arg)        # op " " arg "; "
  #define EMIT2(op, src, dest)   # op " " src ", " dest "; "
  #define CONST(val)           "$" # val
  #define ASMVAR(cvar)         ASMVAR2(__USER_LABEL_PREFIX__, cvar)
  #define ASMVAR2(prefix, cvar) STRING(prefix) cvar
  #define STRING(x)            # x
  #define VAR(var)             ASMVAR(# var)
  #define REGREF1(index)       ASMVAR("reg(" index ")")
  #define REGREF2(index, scale) ASMVAR("reg(," index "," # scale ")")
  #define eax "%%eax"
  #define ecx "%%ecx"
  #define edx "%%edx"
  #define ADD_RN_O8(d) \
    asm ("andl $0xFF, %%eax;" \
         "addl %%eax, %0;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setcb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
			 : "=m" (reg[(d)].I));
  #define CMN_RD_RS \
    asm ("add %0, %1;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setcb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
            \
			 : : "r" (value), "r" (reg[dest].I) : "1");
  #define ADC_RD_RS \
    asm (EMIT2(bt, CONST(0), VAR(C_FLAG)) \
         "adc %1, %%ebx;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setcb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
			 : "=b" (reg[dest].I) \
			   : "r" (value), "b" (reg[dest].I));
  #define SUB_RN_O8(d) \
    asm ("andl $0xFF, %%eax;" \
         "subl %%eax, %0;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setncb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
			 : "=m" (reg[(d)].I));
  #define MOV_RN_O8(d) \
    asm ("andl $0xFF, %%eax;" \
         EMIT2(movb, CONST(0), VAR(N_FLAG)) \
         "movl %%eax, %0;" \
         EMIT1(setzb, VAR(Z_FLAG)) \
			 : "=m" (reg[(d)].I));
  #define CMP_RN_O8(d) \
    asm ("andl $0xFF, %%eax;" \
         "cmpl %%eax, %0;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setncb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
            \
			 : : "m" (reg[(d)].I));
  #define SBC_RD_RS \
    asm volatile (EMIT2(bt, CONST(0), VAR(C_FLAG)) \
                  "cmc;" \
                  "sbb %1, %%ebx;" \
                  EMIT1(setsb, VAR(N_FLAG)) \
                  EMIT1(setzb, VAR(Z_FLAG)) \
                  EMIT1(setncb, VAR(C_FLAG)) \
                  EMIT1(setob, VAR(V_FLAG)) \
					  : "=b" (reg[dest].I) \
						: "r" (value), "b" (reg[dest].I) : "cc", "memory");
  #define LSL_RD_RS \
    asm ("shl %%cl, %%eax;" \
         EMIT1(setcb, VAR(C_FLAG)) \
			 : "=a" (value) \
			   : "a" (reg[dest].I), "c" (value));
  #define LSR_RD_RS \
    asm ("shr %%cl, %%eax;" \
         EMIT1(setcb, VAR(C_FLAG)) \
			 : "=a" (value) \
			   : "a" (reg[dest].I), "c" (value));
  #define ASR_RD_RS \
    asm ("sar %%cl, %%eax;" \
         EMIT1(setcb, VAR(C_FLAG)) \
			 : "=a" (value) \
			   : "a" (reg[dest].I), "c" (value));
  #define ROR_RD_RS \
    asm ("ror %%cl, %%eax;" \
         EMIT1(setcb, VAR(C_FLAG)) \
			 : "=a" (value) \
			   : "a" (reg[dest].I), "c" (value));
  #define NEG_RD_RS \
    asm ("neg %%ebx;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setncb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
			 : "=b" (reg[dest].I) \
			   : "b" (reg[source].I));
  #define CMP_RD_RS \
    asm ("sub %0, %1;" \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT1(setncb, VAR(C_FLAG)) \
         EMIT1(setob, VAR(V_FLAG)) \
            \
			 : : "r" (value), "r" (reg[dest].I) : "1");
  #define IMM5_INSN(OP, N) \
    asm ("movl %%eax,%%ecx;"         \
         "shrl $1,%%eax;"            \
         "andl $7,%%ecx;"            \
         "andl $0x1C,%%eax;"         \
         EMIT2(movl, REGREF1(eax), edx) \
         OP                          \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT2(movl, edx, REGREF2(ecx, 4)) \
			 : : "i" (N))
  #define IMM5_INSN_0(OP)            \
    asm ("movl %%eax,%%ecx;"         \
         "shrl $1,%%eax;"            \
         "andl $7,%%ecx;"            \
         "andl $0x1C,%%eax;"         \
         EMIT2(movl, REGREF1(eax), edx) \
         OP                          \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT2(movl, edx, REGREF2(ecx, 4)) \
			 : :)
  #define IMM5_LSL \
    "shll %0,%%edx;" \
    EMIT1(setcb, VAR(C_FLAG))
  #define IMM5_LSL_0 \
    "testl %%edx,%%edx;"
  #define IMM5_LSR \
    "shrl %0,%%edx;" \
    EMIT1(setcb, VAR(C_FLAG))
  #define IMM5_LSR_0 \
    "testl %%edx,%%edx;" \
    EMIT1(setsb, VAR(C_FLAG)) \
    "xorl %%edx,%%edx;"
  #define IMM5_ASR \
    "sarl %0,%%edx;" \
    EMIT1(setcb, VAR(C_FLAG))
  #define IMM5_ASR_0 \
    "sarl $31,%%edx;" \
    EMIT1(setsb, VAR(C_FLAG))
  #define THREEARG_INSN(OP, N) \
    asm ("movl %%eax,%%edx;"       \
         "shrl $1,%%edx;"          \
         "andl $0x1C,%%edx;"       \
         "andl $7,%%eax;"          \
         EMIT2(movl, REGREF1(edx), ecx) \
         OP(N)                     \
         EMIT1(setsb, VAR(N_FLAG)) \
         EMIT1(setzb, VAR(Z_FLAG)) \
         EMIT2(movl, ecx, REGREF2(eax, 4)) \
			 : :)
  #define ADD_RD_RS_RN(N)          \
    EMIT2(add, VAR(reg) "+" # N "*4", ecx) \
    EMIT1(setcb, VAR(C_FLAG)) \
    EMIT1(setob, VAR(V_FLAG))
  #define ADD_RD_RS_O3(N)          \
    "add $" # N ",%%ecx;"        \
    EMIT1(setcb, VAR(C_FLAG)) \
    EMIT1(setob, VAR(V_FLAG))
  #define ADD_RD_RS_O3_0(N)        \
    EMIT2(movb, CONST(0), VAR(C_FLAG)) \
    "add $0,%%ecx;"           \
    EMIT2(movb, CONST(0), VAR(V_FLAG))
  #define SUB_RD_RS_RN(N) \
    EMIT2(sub, VAR(reg) "+" # N "*4", ecx) \
    EMIT1(setncb, VAR(C_FLAG)) \
    EMIT1(setob, VAR(V_FLAG))
  #define SUB_RD_RS_O3(N) \
    "sub $" # N ",%%ecx;"        \
    EMIT1(setncb, VAR(C_FLAG)) \
    EMIT1(setob, VAR(V_FLAG))
  #define SUB_RD_RS_O3_0(N)        \
    EMIT2(movb, CONST(1), VAR(C_FLAG)) \
    "sub $0,%%ecx;"           \
    EMIT2(movb, CONST(0), VAR(V_FLAG))
#endif
#else // !__GNUC__
  #define ADD_RD_RS_RN(N) \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm add ebx, dword ptr [OFFSET reg + 4 * N] \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define ADD_RD_RS_O3(N) \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm add ebx, N \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define ADD_RD_RS_O3_0 \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm add ebx, 0 \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm mov byte ptr C_FLAG, 0 \
		__asm mov byte ptr V_FLAG, 0 \
	}
  #define ADD_RN_O8(d) \
	{ \
		__asm mov ebx, opcode \
		          __asm and ebx, 255 \
		__asm add dword ptr [OFFSET reg + 4 * (d)], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define CMN_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm add ebx, value \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define ADC_RD_RS \
	{ \
		__asm mov ebx, dest \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * ebx] \
		__asm bt word ptr C_FLAG, 0 \
		__asm adc ebx, value \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define SUB_RD_RS_RN(N) \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm sub ebx, dword ptr [OFFSET reg + 4 * N] \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define SUB_RD_RS_O3(N) \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm sub ebx, N \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define SUB_RD_RS_O3_0 \
	{ \
		__asm mov eax, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm sub ebx, 0 \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm mov byte ptr C_FLAG, 1 \
		__asm mov byte ptr V_FLAG, 0 \
	}
  #define SUB_RN_O8(d) \
	{ \
		__asm mov ebx, opcode \
		__asm and ebx, 255 \
		__asm sub dword ptr [OFFSET reg + 4 * (d)], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define MOV_RN_O8(d) \
	{ \
		__asm mov eax, opcode \
		__asm and eax, 255 \
		__asm mov dword ptr [OFFSET reg + 4 * (d)], eax \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
	}
  #define CMP_RN_O8(d) \
	{ \
		__asm mov eax, dword ptr [OFFSET reg + 4 * (d)] \
		__asm mov ebx, opcode \
		__asm and ebx, 255 \
		__asm sub eax, ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define SBC_RD_RS \
	{ \
		__asm mov ebx, dest \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * ebx] \
		__asm mov eax, value \
		__asm bt word ptr C_FLAG, 0 \
		__asm cmc \
		__asm sbb ebx, eax \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define LSL_RD_RM_I5 \
	{ \
		__asm mov eax, source \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr shift \
		__asm shl eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define LSL_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr value \
		__asm shl eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define LSR_RD_RM_I5 \
	{ \
		__asm mov eax, source \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr shift \
		__asm shr eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define LSR_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr value \
		__asm shr eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define ASR_RD_RM_I5 \
	{ \
		__asm mov eax, source \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr shift \
		__asm sar eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define ASR_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr value \
		__asm sar eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define ROR_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov eax, dword ptr [OFFSET reg + 4 * eax] \
		__asm mov cl, byte ptr value \
		__asm ror eax, cl \
		__asm mov value, eax \
		__asm setc byte ptr C_FLAG \
	}
  #define NEG_RD_RS \
	{ \
		__asm mov ebx, source \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * ebx] \
		__asm neg ebx \
		__asm mov eax, dest \
		__asm mov dword ptr [OFFSET reg + 4 * eax], ebx \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
  #define CMP_RD_RS \
	{ \
		__asm mov eax, dest \
		__asm mov ebx, dword ptr [OFFSET reg + 4 * eax] \
		__asm sub ebx, value \
		__asm sets byte ptr N_FLAG \
		__asm setz byte ptr Z_FLAG \
		__asm setnc byte ptr C_FLAG \
		__asm seto byte ptr V_FLAG \
	}
#endif
#endif

// C core
#ifndef ADDCARRY
 #define ADDCARRY(a, b, c) \
    C_FLAG = ((NEG(a) & NEG(b)) | \
              (NEG(a) & POS(c)) | \
              (NEG(b) & POS(c))) ? true : false;
#endif
#ifndef ADDOVERFLOW
 #define ADDOVERFLOW(a, b, c) \
    V_FLAG = ((NEG(a) & NEG(b) & POS(c)) | \
              (POS(a) & POS(b) & NEG(c))) ? true : false;
#endif
#ifndef SUBCARRY
 #define SUBCARRY(a, b, c) \
    C_FLAG = ((NEG(a) & POS(b)) | \
              (NEG(a) & POS(c)) | \
              (POS(b) & POS(c))) ? true : false;
#endif
#ifndef SUBOVERFLOW
 #define SUBOVERFLOW(a, b, c) \
    V_FLAG = ((NEG(a) & POS(b) & POS(c)) | \
              (POS(a) & NEG(b) & NEG(c))) ? true : false;
#endif
#ifndef ADD_RD_RS_RN
 #define ADD_RD_RS_RN(N) \
	{ \
		u32 lhs = reg[source].I; \
		u32 rhs = reg[N].I; \
		u32 res = lhs + rhs; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		ADDCARRY(lhs, rhs, res); \
		ADDOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef ADD_RD_RS_O3
 #define ADD_RD_RS_O3(N) \
	{ \
		u32 lhs = reg[source].I; \
		u32 rhs = N; \
		u32 res = lhs + rhs; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		ADDCARRY(lhs, rhs, res); \
		ADDOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef ADD_RD_RS_O3_0
# define ADD_RD_RS_O3_0 ADD_RD_RS_O3
#endif
#ifndef ADD_RN_O8
 #define ADD_RN_O8(d) \
	{ \
		u32 lhs = reg[(d)].I; \
		u32 rhs = (opcode & 255); \
		u32 res = lhs + rhs; \
		reg[(d)].I = res; \
		Z_FLAG	   = (res == 0) ? true : false; \
		N_FLAG	   = NEG(res) ? true : false; \
		ADDCARRY(lhs, rhs, res); \
		ADDOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef CMN_RD_RS
 #define CMN_RD_RS \
	{ \
		u32 lhs = reg[dest].I; \
		u32 rhs = value; \
		u32 res = lhs + rhs; \
		Z_FLAG = (res == 0) ? true : false; \
		N_FLAG = NEG(res) ? true : false; \
		ADDCARRY(lhs, rhs, res); \
		ADDOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef ADC_RD_RS
 #define ADC_RD_RS \
	{ \
		u32 lhs = reg[dest].I; \
		u32 rhs = value; \
		u32 res = lhs + rhs + (u32)C_FLAG; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		ADDCARRY(lhs, rhs, res); \
		ADDOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef SUB_RD_RS_RN
 #define SUB_RD_RS_RN(N) \
	{ \
		u32 lhs = reg[source].I; \
		u32 rhs = reg[N].I; \
		u32 res = lhs - rhs; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef SUB_RD_RS_O3
 #define SUB_RD_RS_O3(N) \
	{ \
		u32 lhs = reg[source].I; \
		u32 rhs = N; \
		u32 res = lhs - rhs; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef SUB_RD_RS_O3_0
# define SUB_RD_RS_O3_0 SUB_RD_RS_O3
#endif
#ifndef SUB_RN_O8
 #define SUB_RN_O8(d) \
	{ \
		u32 lhs = reg[(d)].I; \
		u32 rhs = (opcode & 255); \
		u32 res = lhs - rhs; \
		reg[(d)].I = res; \
		Z_FLAG	   = (res == 0) ? true : false; \
		N_FLAG	   = NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef MOV_RN_O8
 #define MOV_RN_O8(d) \
	{ \
		reg[d].I = opcode & 255; \
		N_FLAG	 = false; \
		Z_FLAG	 = (reg[d].I ? false : true); \
	}
#endif
#ifndef CMP_RN_O8
 #define CMP_RN_O8(d) \
	{ \
		u32 lhs = reg[(d)].I; \
		u32 rhs = (opcode & 255); \
		u32 res = lhs - rhs; \
		Z_FLAG = (res == 0) ? true : false; \
		N_FLAG = NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef SBC_RD_RS
 #define SBC_RD_RS \
	{ \
		u32 lhs = reg[dest].I; \
		u32 rhs = value; \
		u32 res = lhs - rhs - !((u32)C_FLAG); \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef LSL_RD_RM_I5
 #define LSL_RD_RM_I5 \
	{ \
		C_FLAG = (reg[source].I >> (32 - shift)) & 1 ? true : false; \
		value  = reg[source].I << shift; \
	}
#endif
#ifndef LSL_RD_RS
 #define LSL_RD_RS \
	{ \
		C_FLAG = (reg[dest].I >> (32 - value)) & 1 ? true : false; \
		value  = reg[dest].I << value; \
	}
#endif
#ifndef LSR_RD_RM_I5
 #define LSR_RD_RM_I5 \
	{ \
		C_FLAG = (reg[source].I >> (shift - 1)) & 1 ? true : false; \
		value  = reg[source].I >> shift; \
	}
#endif
#ifndef LSR_RD_RS
 #define LSR_RD_RS \
	{ \
		C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false; \
		value  = reg[dest].I >> value; \
	}
#endif
#ifndef ASR_RD_RM_I5
 #define ASR_RD_RM_I5 \
	{ \
		C_FLAG = ((s32)reg[source].I >> (int)(shift - 1)) & 1 ? true : false; \
		value  = (s32)reg[source].I >> (int)shift; \
	}
#endif
#ifndef ASR_RD_RS
 #define ASR_RD_RS \
	{ \
		C_FLAG = ((s32)reg[dest].I >> (int)(value - 1)) & 1 ? true : false; \
		value  = (s32)reg[dest].I >> (int)value; \
	}
#endif
#ifndef ROR_RD_RS
 #define ROR_RD_RS \
	{ \
		C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false; \
		value  = ((reg[dest].I << (32 - value)) | \
		          (reg[dest].I >> value)); \
	}
#endif
#ifndef NEG_RD_RS
 #define NEG_RD_RS \
	{ \
		u32 lhs = reg[source].I; \
		u32 rhs = 0; \
		u32 res = rhs - lhs; \
		reg[dest].I = res; \
		Z_FLAG		= (res == 0) ? true : false; \
		N_FLAG		= NEG(res) ? true : false; \
		SUBCARRY(rhs, lhs, res); \
		SUBOVERFLOW(rhs, lhs, res); \
	}
#endif
#ifndef CMP_RD_RS
 #define CMP_RD_RS \
	{ \
		u32 lhs = reg[dest].I; \
		u32 rhs = value; \
		u32 res = lhs - rhs; \
		Z_FLAG = (res == 0) ? true : false; \
		N_FLAG = NEG(res) ? true : false; \
		SUBCARRY(lhs, rhs, res); \
		SUBOVERFLOW(lhs, rhs, res); \
	}
#endif
#ifndef IMM5_INSN
 #define IMM5_INSN(OP, N) \
    int dest = opcode & 0x07; \
    int source = (opcode >> 3) & 0x07; \
    u32 value; \
    OP(N); \
    reg[dest].I = value; \
    N_FLAG		= (value & 0x80000000 ? true : false); \
    Z_FLAG		= (value ? false : true);
 #define IMM5_INSN_0(OP) \
    int dest = opcode & 0x07; \
    int source = (opcode >> 3) & 0x07; \
    u32 value; \
    OP; \
    reg[dest].I = value; \
    N_FLAG		= (value & 0x80000000 ? true : false); \
    Z_FLAG		= (value ? false : true);
 #define IMM5_LSL(N) \
    int shift = N; \
    LSL_RD_RM_I5;
 #define IMM5_LSL_0 \
    value = reg[source].I;
 #define IMM5_LSR(N) \
    int shift = N; \
    LSR_RD_RM_I5;
 #define IMM5_LSR_0 \
    C_FLAG = reg[source].I & 0x80000000 ? true : false; \
    value  = 0;
 #define IMM5_ASR(N) \
    int shift = N; \
    ASR_RD_RM_I5;
 #define IMM5_ASR_0 \
    if (reg[source].I & 0x80000000) { \
		value  = 0xFFFFFFFF; \
		C_FLAG = true; \
	} else { \
		value  = 0; \
		C_FLAG = false; \
	}
#endif
#ifndef THREEARG_INSN
 #define THREEARG_INSN(OP, N) \
    int dest = opcode & 0x07;          \
    int source = (opcode >> 3) & 0x07; \
    OP(N);
#endif
