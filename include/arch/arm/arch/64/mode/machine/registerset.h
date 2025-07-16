/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <config.h>
#include <arch/machine/debug_conf.h>

/* CurrentEL register */
#define PEXPL1                  (1 << 2)
#define PEXPL2                  (1 << 3)

/* PSTATE register */
#define PMODE_FIRQ              (1 << 6)
#define PMODE_IRQ               (1 << 7)
#define PMODE_SERROR            (1 << 8)
#define PMODE_DEBUG             (1 << 9)
#define PMODE_EL0t              0
#define PMODE_EL1t              4
#define PMODE_EL1h              5
#define PMODE_EL2h              9

/* DAIF register */
#define DAIF_FIRQ               (1 << 6)
#define DAIF_IRQ                (1 << 7)
#define DAIF_SERROR             (1 << 8)
#define DAIF_DEBUG              (1 << 9)
#define DAIFSET_MASK            0xf

/* ESR register */
#define ESR_EC_SHIFT            26
#define ESR_EC_LEL_DABT         0x24    // Data abort from a lower EL
#define ESR_EC_CEL_DABT         0x25    // Data abort from the current EL
#define ESR_EC_LEL_IABT         0x20    // Instruction abort from a lower EL
#define ESR_EC_CEL_IABT         0x21    // Instruction abort from the current EL
#define ESR_EC_LEL_SVC64        0x15    // SVC from a lower EL in AArch64 state
#define ESR_EC_LEL_HVC64        0x16    // HVC from EL1 in AArch64 state
#define ESR_EL1_EC_ENFP         0x7     // Access to Advanced SIMD or floating-point registers


/* ID_AA64PFR0_EL1 register */
#define ID_AA64PFR0_EL1_FP      16     // HWCap for Floating Point
#define ID_AA64PFR0_EL1_ASIMD   20     // HWCap for Advanced SIMD

/* CPACR_EL1 register */
#define CPACR_EL1_FPEN          20     // FP registers access

/*
 * We cannot allow async aborts in the verified kernel, but they are useful
 * in identifying invalid memory access bugs so we enable them in debug mode.
 */
#ifdef CONFIG_DEBUG_BUILD
#define PSTATE_EXTRA_FLAGS  0
#else
#define PSTATE_EXTRA_FLAGS  PMODE_SERROR
#endif

#define PSTATE_USER         (PMODE_FIRQ | PMODE_EL0t | PSTATE_EXTRA_FLAGS)

#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
#define PSTATE_IDLETHREAD   (PMODE_FIRQ | PMODE_EL2h | PSTATE_EXTRA_FLAGS)
#else
#define PSTATE_IDLETHREAD   (PMODE_FIRQ | PMODE_EL1h | PSTATE_EXTRA_FLAGS)
#endif

/* Offsets within the user context, these need to match the order in
 * register_t below */
#define PT_LR                       (30 * 8)
#define PT_SP_EL0                   (31 * 8)
#define PT_ELR_EL1                  (32 * 8)
#define PT_SPSR_EL1                 (33 * 8)
#define PT_FaultIP                  (34 * 8)
#define PT_TPIDR_EL0                (35 * 8)

#ifndef __ASSEMBLER__ /* C only definitions */

#include <config.h>
#include <stdint.h>
#include <assert.h>
#include <util.h>
#include <arch/types.h>
#include <sel4/plat/api/constants.h>

/* These are the indices of the registers in the saved thread context.
 * The values are determined by the order in which they're saved in the trap handler. */
enum _register {
    X0                          = 0,    /* 0x00 */
    capRegister                 = 0,
    badgeRegister               = 0,

    X1                          = 1,    /* 0x08 */
    msgInfoRegister             = 1,

    X2                          = 2,    /* 0x10 */
    X3                          = 3,    /* 0x18 */
    X4                          = 4,    /* 0x20 */
    X5                          = 5,    /* 0x28 */
    X6                          = 6,    /* 0x30 */
#ifdef CONFIG_KERNEL_MCS
    replyRegister               = 6,
#endif
    X7                          = 7,    /* 0x38 */
    X8                          = 8,    /* 0x40 */
#ifdef CONFIG_KERNEL_MCS
    nbsendRecvDest              = 8,
#endif
    X9                          = 9,    /* 0x48 */
    X10                         = 10,   /* 0x50 */
    X11                         = 11,   /* 0x58 */
    X12                         = 12,   /* 0x60 */
    X13                         = 13,   /* 0x68 */
    X14                         = 14,   /* 0x70 */
    X15                         = 15,   /* 0x78 */
    X16                         = 16,   /* 0x80 */
    X17                         = 17,   /* 0x88 */
    X18                         = 18,   /* 0x90 */
    X19                         = 19,   /* 0x98 */
    X20                         = 20,   /* 0xa0 */
    X21                         = 21,   /* 0xa8 */
    X22                         = 22,   /* 0xb0 */
    X23                         = 23,   /* 0xb8 */
    X24                         = 24,   /* 0xc0 */
    X25                         = 25,   /* 0xc8 */
    X26                         = 26,   /* 0xd0 */
    X27                         = 27,   /* 0xd8 */
    X28                         = 28,   /* 0xe0 */
    X29                         = 29,   /* 0xe8 */

    X30                         = 30,   /* 0xf0 */
    LR                          = 30,

    /* End of GP registers, the following are additional kernel-saved state. */

    SP_EL0                      = 31,   /* 0xf8 */
    ELR_EL1                     = 32,   /* 0x100 */
    NextIP                      = 32,   /* LR_svc */
    SPSR_EL1                    = 33,   /* 0x108 */

    FaultIP                     = 34,   /* 0x110 */
    /* user readable/writable thread ID register.
     * name comes from the ARM manual */
    TPIDR_EL0                   = 35,
    /* user readonly thread ID register. */
    TPIDRRO_EL0                 = 36,
#ifdef CONFIG_ARM_TLS_REG_TPIDRU
    TLS_BASE = TPIDR_EL0,
#elif defined(CONFIG_ARM_TLS_REG_TPIDRURO)
    TLS_BASE = TPIDRRO_EL0,
#endif

    /* Pointer Authentication Key Registers */
    APDAKeyHi_EL1 = 37,
    APDAKeyLo_EL1 = 38,
    APDBKeyHi_EL1 = 39,
    APDBKeyLo_EL1 = 40,
    APGAKeyHi_EL1 = 41,
    APGAKeyLo_EL1 = 42,
    APIAKeyHi_EL1 = 43,
    APIAKeyLo_EL1 = 44,
    APIBKeyHi_EL1 = 45,
    APIBKeyLo_EL1 = 46,

    n_contextRegisters = 47,
};

#define NEXT_PC_REG ELR_EL1

compile_assert(sp_offset_correct, SP_EL0 *sizeof(word_t) == PT_SP_EL0)
compile_assert(lr_svc_offset_correct, ELR_EL1 *sizeof(word_t) == PT_ELR_EL1)
compile_assert(faultinstruction_offset_correct, FaultIP *sizeof(word_t) == PT_FaultIP)

typedef word_t register_t;

enum messageSizes {
    n_msgRegisters = seL4_FastMessageRegisters,
    n_frameRegisters = 17,
    n_gpRegisters = 19,
    n_exceptionMessage = 3,
    n_syscallMessage = 12,
#ifdef CONFIG_KERNEL_MCS
    n_timeoutMessage = 34,
#endif
#ifdef CONFIG_AARCH64_PTR_AUTH
    n_ptrAuthRegisters = 10,
#else
    n_ptrAuthRegisters = 0,
#endif
};
enum messageSizes n_allUserCtxRegs = n_frameRegisters + n_gpRegisters + n_ptrAuthRegisters;


#define EXCEPTION_MESSAGE \
 {\
    [seL4_UserException_FaultIP] = FaultIP,\
    [seL4_UserException_SP] = SP_EL0,\
    [seL4_UserException_SPSR] = SPSR_EL1\
 }

#define SYSCALL_MESSAGE \
{\
    [seL4_UnknownSyscall_X0] = X0,\
    [seL4_UnknownSyscall_X1] = X1,\
    [seL4_UnknownSyscall_X2] = X2,\
    [seL4_UnknownSyscall_X3] = X3,\
    [seL4_UnknownSyscall_X4] = X4,\
    [seL4_UnknownSyscall_X5] = X5,\
    [seL4_UnknownSyscall_X6] = X6,\
    [seL4_UnknownSyscall_X7] = X7,\
    [seL4_UnknownSyscall_FaultIP] = FaultIP,\
    [seL4_UnknownSyscall_SP] = SP_EL0,\
    [seL4_UnknownSyscall_LR] = ELR_EL1,\
    [seL4_UnknownSyscall_SPSR] = SPSR_EL1\
}

#define TIMEOUT_REPLY_MESSAGE \
{\
    [seL4_TimeoutReply_FaultIP] = FaultIP,\
    [seL4_TimeoutReply_SP] = SP_EL0,\
    [seL4_TimeoutReply_SPSR_EL1] = SPSR_EL1,\
    [seL4_TimeoutReply_X0] = X0,\
    [seL4_TimeoutReply_X1] = X1,\
    [seL4_TimeoutReply_X2] = X2,\
    [seL4_TimeoutReply_X3] = X3,\
    [seL4_TimeoutReply_X4] = X4,\
    [seL4_TimeoutReply_X5] = X5,\
    [seL4_TimeoutReply_X6] = X6,\
    [seL4_TimeoutReply_X7] = X7,\
    [seL4_TimeoutReply_X8] = X8,\
    [seL4_TimeoutReply_X16] = X16,\
    [seL4_TimeoutReply_X17] = X17,\
    [seL4_TimeoutReply_X18] = X18,\
    [seL4_TimeoutReply_X29] = X29,\
    [seL4_TimeoutReply_X30] = X30,\
    [seL4_TimeoutReply_X9] = X9,\
    [seL4_TimeoutReply_X10] = X10,\
    [seL4_TimeoutReply_X11] = X11,\
    [seL4_TimeoutReply_X12] = X12,\
    [seL4_TimeoutReply_X13] = X13,\
    [seL4_TimeoutReply_X14] = X14,\
    [seL4_TimeoutReply_X15] = X15,\
    [seL4_TimeoutReply_X19] = X19,\
    [seL4_TimeoutReply_X20] = X20,\
    [seL4_TimeoutReply_X21] = X21,\
    [seL4_TimeoutReply_X22] = X22,\
    [seL4_TimeoutReply_X23] = X23,\
    [seL4_TimeoutReply_X24] = X24,\
    [seL4_TimeoutReply_X25] = X25,\
    [seL4_TimeoutReply_X26] = X26,\
    [seL4_TimeoutReply_X27] = X27,\
    [seL4_TimeoutReply_X28] = X28,\
}

extern const register_t msgRegisters[];
extern const register_t frameRegisters[];
extern const register_t gpRegisters[];
extern const register_t ptrAuthRegisters[];

#ifdef ARM_BASE_CP14_SAVE_AND_RESTORE
typedef struct debug_register_pair {
    word_t cr, vr;
} debug_register_pair_t;

typedef struct user_breakpoint_state {
    /* We don't use context comparisons */
    debug_register_pair_t breakpoint[seL4_NumExclusiveBreakpoints],
                          watchpoint[seL4_NumExclusiveWatchpoints];
    uint32_t used_breakpoints_bf;
    word_t n_instructions;
    bool_t single_step_enabled;
} user_breakpoint_state_t;
#endif /* ARM_BASE_CP14_SAVE_AND_RESTORE */

#ifdef CONFIG_HAVE_FPU
typedef struct user_fpu_state {
    uint64_t vregs[64];
    uint32_t fpsr;
    uint32_t fpcr;
} user_fpu_state_t;
#endif /* CONFIG_HAVE_FPU */

/* ARM user-code context: size = 72 bytes
 * Or with hardware debug support built in:
 *      72 + sizeof(word_t) * (NUM_BPS + NUM_WPS) * 2
 *
 * The "word_t registers" member of this struct must come first, because in
 * head.S, we assume that an "ldr %0, =ksCurThread" will point to the beginning
 * of the current thread's registers. The assert below should help.
 */
struct user_context {
    word_t registers[n_contextRegisters];
#ifdef ARM_BASE_CP14_SAVE_AND_RESTORE
    user_breakpoint_state_t breakpointState;
#endif /* ARM_BASE_CP14_SAVE_AND_RESTORE */
#ifdef CONFIG_HAVE_FPU
    user_fpu_state_t fpuState;
#endif /* CONFIG_HAVE_FPU */
};
typedef struct user_context user_context_t;

unverified_compile_assert(registers_are_first_member_of_user_context,
                          OFFSETOF(user_context_t, registers) == 0)


#ifdef ARM_BASE_CP14_SAVE_AND_RESTORE
void Arch_initBreakpointContext(user_context_t *context);
#endif

static inline void Arch_initContext(user_context_t *context)
{
    context->registers[SPSR_EL1] = PSTATE_USER;
#ifdef ARM_BASE_CP14_SAVE_AND_RESTORE
    Arch_initBreakpointContext(context);
#endif
}

#define __maybe_unused      __attribute__((__unused__))

/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)

#define __DEFINE_ASM_GPR_NUMS					\
"	.irp	num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30\n" \
"	.equ	.L__gpr_num_x\\num, \\num\n"			\
"	.equ	.L__gpr_num_w\\num, \\num\n"			\
"	.endr\n"						\
"	.equ	.L__gpr_num_xzr, 31\n"				\
"	.equ	.L__gpr_num_wzr, 31\n"

#define __emit_inst(x)			".inst " __stringify((x)) "\n\t"

#define DEFINE_MRS_S						\
	__DEFINE_ASM_GPR_NUMS					\
"	.macro	mrs_s, rt, sreg\n"				\
	__emit_inst(0xd5200000|(\\sreg)|(.L__gpr_num_\\rt))	\
"	.endm\n"

#define DEFINE_MSR_S						\
	__DEFINE_ASM_GPR_NUMS					\
"	.macro	msr_s, sreg, rt\n"				\
	__emit_inst(0xd5000000|(\\sreg)|(.L__gpr_num_\\rt))	\
"	.endm\n"

#define UNDEFINE_MRS_S						\
"	.purgem	mrs_s\n"

#define UNDEFINE_MSR_S						\
"	.purgem	msr_s\n"

#define __mrs_s(v, r)						\
	DEFINE_MRS_S						\
"	mrs_s " v ", " __stringify(r) "\n"			\
	UNDEFINE_MRS_S

#define __msr_s(r, v)						\
	DEFINE_MSR_S						\
"	msr_s " __stringify(r) ", " v "\n"			\
	UNDEFINE_MSR_S

/*
 * ARMv8 ARM reserves the following encoding for system registers:
 * (Ref: ARMv8 ARM, Section: "System instruction class encoding overview",
 *  C5.2, version:ARM DDI 0487A.f)
 *	[20-19] : Op0
 *	[18-16] : Op1
 *	[15-12] : CRn
 *	[11-8]  : CRm
 *	[7-5]   : Op2
 */
#define Op0_shift	19
#define Op0_mask	0x3
#define Op1_shift	16
#define Op1_mask	0x7
#define CRn_shift	12
#define CRn_mask	0xf
#define CRm_shift	8
#define CRm_mask	0xf
#define Op2_shift	5
#define Op2_mask	0x7

/* 
 * This is System register encoding macros.
 * These are used to encode the system register names into a single 64-bit value.
 * (Ref: 《Arm Architecture Reference Manual Armv8》,
 * Section: Chapter D12 AArch64 System Register Encoding
 * Version: ARM DDI 0487F.c)
 */
#define sys_reg_code(op0, op1, crn, crm, op2) \
	(((op0) << Op0_shift) | ((op1) << Op1_shift) | \
	 ((crn) << CRn_shift) | ((crm) << CRm_shift) | \
	 ((op2) << Op2_shift))

#define SYS_ID_AA64ISAR1_EL1    sys_reg_code(3, 0, 0, 6, 1)
#define SYS_ID_AA64ISAR2_EL1    sys_reg_code(3, 0, 0, 6, 2)

#define SYS_APIAKEYLO_EL1		sys_reg_code(3, 0, 2, 1, 0)
#define SYS_APIAKEYHI_EL1		sys_reg_code(3, 0, 2, 1, 1)
#define SYS_APIBKEYLO_EL1		sys_reg_code(3, 0, 2, 1, 2)
#define SYS_APIBKEYHI_EL1		sys_reg_code(3, 0, 2, 1, 3)

#define SYS_APDAKEYLO_EL1		sys_reg_code(3, 0, 2, 2, 0)
#define SYS_APDAKEYHI_EL1		sys_reg_code(3, 0, 2, 2, 1)
#define SYS_APDBKEYLO_EL1		sys_reg_code(3, 0, 2, 2, 2)
#define SYS_APDBKEYHI_EL1		sys_reg_code(3, 0, 2, 2, 3)

#define SYS_APGAKEYLO_EL1		sys_reg_code(3, 0, 2, 3, 0)
#define SYS_APGAKEYHI_EL1		sys_reg_code(3, 0, 2, 3, 1)

#define SYS_REG(reg) SYS_##reg

/*
 * For registers without architectural names, or simply unsupported by
 * GAS, just like ID_AA64ISAR1_EL1 or ID_AA64ISAR2_EL1.
 *
 * __check_r forces warnings to be generated by the compiler when
 * evaluating r which wouldn't normally happen due to being passed to
 * the assembler via __stringify(r).
 */
#define read_sysreg_s(r) ({						\
	uint64_t __val;							\
	uint32_t __maybe_unused __check_r = (uint32_t)(r);			\
	asm volatile(__mrs_s("%0", r) : "=r" (__val));			\
	__val;								\
})

#define write_sysreg_s(v, r) do {					\
	uint64_t __val = (uint64_t)(v);						\
	uint32_t __maybe_unused __check_r = (uint32_t)(r);			\
	asm volatile(__msr_s(r, "%x0") : : "rZ" (__val));		\
} while (0)

word_t cpu_has_ptr_auth(void);

#endif /* !__ASSEMBLER__ */

