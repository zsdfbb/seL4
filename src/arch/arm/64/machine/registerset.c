/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <assert.h>
#include <arch/machine/registerset.h>
#include <stdint.h>

const register_t msgRegisters[] = {
    X2, X3, X4, X5
};
compile_assert(
    consistent_message_registers,
    sizeof(msgRegisters) / sizeof(msgRegisters[0]) == n_msgRegisters
);

const register_t frameRegisters[] = {
    FaultIP, SP_EL0, SPSR_EL1,
    X0, X1, X2, X3, X4, X5, X6, X7, X8, X16, X17, X18, X29, X30
};
compile_assert(
    consistent_frame_registers,
    sizeof(frameRegisters) / sizeof(frameRegisters[0]) == n_frameRegisters
);

const register_t gpRegisters[] = {
    X9, X10, X11, X12, X13, X14, X15,
    X19, X20, X21, X22, X23, X24, X25, X26, X27, X28,
    TPIDR_EL0, TPIDRRO_EL0,
};
compile_assert(
    consistent_gp_registers,
    sizeof(gpRegisters) / sizeof(gpRegisters[0]) == n_gpRegisters
);

#ifdef CONFIG_AARCH64_PTR_AUTH
const register_t ptrAuthRegisters[] = {
    APDAKeyHi_EL1,
    APDAKeyLo_EL1,
    APDBKeyHi_EL1,
    APDBKeyLo_EL1,
    APGAKeyHi_EL1,
    APGAKeyLo_EL1,
    APIAKeyHi_EL1,
    APIAKeyLo_EL1,
    APIBKeyHi_EL1,
    APIBKeyLo_EL1,
};
compile_assert(
    consistent_ptrAuth_registers,
    sizeof(ptrAuthRegisters) / sizeof(ptrAuthRegisters[0]) == n_ptrAuthRegisters
);
#else
typtdef ptrAuthRegisters NULL
#endif

typedef struct registersItem {
    const register_t *regs;
    enum messageSizes count;
} registersItem_t;


/*
 * Only the root server has permission to access all registers.
 * 1: frameRegisters
 * 2: gpRegisters
 * 3: ptrAuthRegisters (root server only)
 */
const registersItem_t writeRegistersArray[] = {
    {frameRegisters, n_frameRegisters},
    {gpRegisters, n_gpRegisters},
    {ptrAuthRegisters, n_ptrAuthRegisters}
};

const word_t writeRegistersArraySizeExt = sizeof(writeRegistersArray) / sizeof(writeRegistersArray[0]);
const word_t writeRegistersArraySize = writeRegistersArraySizeExt - 1;

/* Instruction set attribute register
 *
 *   ID_AA64ISAR1_EL1 - Instruction set attribute register 1
 *   +------------------------------+---------+---------+
 *   | Name                         |  bits   | visible |
 *   +------------------------------+---------+---------+
 *   | I8MM                         | [55-52] |    y    |
 *   +------------------------------+---------+---------+
 *   | DGH                          | [51-48] |    y    |
 *   +------------------------------+---------+---------+
 *   | BF16                         | [47-44] |    y    |
 *   +------------------------------+---------+---------+
 *   | SB                           | [39-36] |    y    |
 *   +------------------------------+---------+---------+
 *   | FRINTTS                      | [35-32] |    y    |
 *   +------------------------------+---------+---------+
 *   | GPI                          | [31-28] |    y    |
 *   +------------------------------+---------+---------+
 *   | GPA                          | [27-24] |    y    |
 *   +------------------------------+---------+---------+
 *   | LRCPC                        | [23-20] |    y    |
 *   +------------------------------+---------+---------+
 *   | FCMA                         | [19-16] |    y    |
 *   +------------------------------+---------+---------+
 *   | JSCVT                        | [15-12] |    y    |
 *   +------------------------------+---------+---------+
 *   | API                          | [11-8]  |    y    |
 *   +------------------------------+---------+---------+
 *   | APA                          | [7-4]   |    y    |
 *   +------------------------------+---------+---------+
 *   | DPB                          | [3-0]   |    y    |
 *   +------------------------------+---------+---------+
 *
 *   ID_AA64ISAR2_EL1 - Instruction set attribute register 2
 *   +------------------------------+---------+---------+
 *   | Name                         |  bits   | visible |
 *   +------------------------------+---------+---------+
 *   | CSSC                         | [55-52] |    y    |
 *   +------------------------------+---------+---------+
 *   | RPRFM                        | [51-48] |    y    |
 *   +------------------------------+---------+---------+
 *   | BC                           | [23-20] |    y    |
 *   +------------------------------+---------+---------+
 *   | MOPS                         | [19-16] |    y    |
 *   +------------------------------+---------+---------+
 *   | APA3                         | [15-12] |    y    |
 *   +------------------------------+---------+---------+
 *   | GPA3                         | [11-8]  |    y    |
 *   +------------------------------+---------+---------+
 *   | RPRES                        | [7-4]   |    y    |
 *   +------------------------------+---------+---------+
 *   | WFXT                         | [3-0]   |    y    |
 *   +------------------------------+---------+---------+
 */
#define ID_AA64ISAR1_EL1_API    0x0000000000000f00UL
#define ID_AA64ISAR1_EL1_APA    0x00000000000000f0UL
#define ID_AA64ISAR2_EL1_APA3   0x000000000000f000UL

word_t cpu_has_ptr_auth(void)
{
    /* Check if the CPU supports pointer authentication */
    word_t isar1 = 0;
    word_t isar2 = 0;

    isar1 = read_sysreg_s(SYS_REG(ID_AA64ISAR1_EL1));
    isar2 = read_sysreg_s(SYS_REG(ID_AA64ISAR2_EL1));

    printf("ID_AA64ISAR1_EL1: 0x%lx, ID_AA64ISAR2_EL1: 0x%lx\n", isar1, isar2);
    printf("ID_AA64ISAR1_EL1_API: 0x%lx\n", (isar1 & ID_AA64ISAR1_EL1_API) >> 8);
    printf("ID_AA64ISAR1_EL1_APA: 0x%lx\n", (isar1 & ID_AA64ISAR1_EL1_APA) >> 4);
    printf("ID_AA64ISAR2_EL1_APA3: 0x%lx\n", (isar2 & ID_AA64ISAR2_EL1_APA3) >> 12);

    if ((isar1 & ID_AA64ISAR1_EL1_API) >> 8) {
        return true;
    }

    if ((isar1 & ID_AA64ISAR1_EL1_APA) >> 4) {
        return true;
    }

    if ((isar2 & ID_AA64ISAR2_EL1_APA3) >> 12) {
        return true;
    }

    return false;
}

#ifdef CONFIG_KERNEL_MCS
word_t getNBSendRecvDest(void)
{
    return getRegister(NODE_STATE(ksCurThread), nbsendRecvDest);
}
#endif
