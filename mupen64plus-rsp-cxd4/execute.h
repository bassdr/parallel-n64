/******************************************************************************\
* Authors:  Iconoclast                                                         *
* Release:  2013.12.11                                                         *
* License:  CC0 Public Domain Dedication                                       *
*                                                                              *
* To the extent possible under law, the author(s) have dedicated all copyright *
* and related and neighboring rights to this software to the public domain     *
* worldwide. This software is distributed without any warranty.                *
*                                                                              *
* You should have received a copy of the CC0 Public Domain Dedication along    *
* with this software.                                                          *
* If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.             *
\******************************************************************************/
#include "Rsp_#1.1.h"
#include "rsp.h"

#include "su.h"
#include "vu/vu.h"
#include "matrix.h"

#define FIT_IMEM(PC)    (PC & 0xFFF & 0xFFC)

NOINLINE void run_task(void)
{
    register unsigned int i;
    register int PC;

    for (i = 0; i < 32; i++)
        MFC0_count[i] = 0;
    stale_signals = 0;

    PC = FIT_IMEM(*RSP.SP_PC_REG);
    while ((*RSP.SP_STATUS_REG & 0x00000001) == 0x00000000)
    {
        register uint32_t inst;
        register int rd, rs, rt;
        register int base;

        inst = *(uint32_t *)(RSP.IMEM + FIT_IMEM(PC));
#ifdef EMULATE_STATIC_PC
        PC = (PC + 0x004);
EX:
#endif
#ifdef SP_EXECUTE_LOG
        step_SP_commands(inst);
#endif
        if (inst >> 25 == 0x25) /* is a VU instruction */
        {
            const int opcode = inst % 64; /* inst.R.func */
            const int vd = (inst & 0x000007FF) >> 6; /* inst.R.sa */
            const int vs = (unsigned short)(inst) >> 11; /* inst.R.rd */
            const int vt = (inst >> 16) & 31; /* inst.R.rt */
            const int e  = (inst >> 21) & 0xF; /* rs & 0xF */

            COP2_C2[opcode](vd, vs, vt, e);
        }
        else
        {
            const int op = inst >> 26;
            const int element = (inst & 0x000007FF) >> 7;

#if (0)
            SR[0] = 0x00000000; /* already handled on per-instruction basis */
#endif
            switch (op)
            {
            int16_t offset;
            register uint32_t addr;

            case 000: /* SPECIAL */
#if (1u >> 1 == 0)
                rd = (inst & 0x0000FFFFu) >> 11;
             /* rs = inst >> 21; // Primary op is 0, so we don't need &= 31. */
#else
                rd = (inst >> 11) % 32;
             /* rs = (inst >> 21) % 32; */
#endif
                rt = (inst >> 16) % (1 << 5);
                switch (inst % 64)
                {
                case 000: /* SLL */
                    SR[rd] = SR[rt] << MASK_SA(inst >> 6);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 002: /* SRL */
                    SR[rd] = (unsigned)(SR[rt]) >> MASK_SA(inst >> 6);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 003: /* SRA */
                    SR[rd] = (signed)(SR[rt]) >> MASK_SA(inst >> 6);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 004: /* SLLV */
                    SR[rd] = SR[rt] << MASK_SA(SR[rs = inst >> 21]);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 006: /* SRLV */
                    SR[rd] = (unsigned)(SR[rt]) >> MASK_SA(SR[rs = inst >> 21]);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 007: /* SRAV */
                    SR[rd] = (signed)(SR[rt]) >> MASK_SA(SR[rs = inst >> 21]);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 011: /* JALR */
                    SR[rd] = (PC + LINK_OFF) & 0x00000FFC;
                    SR[0] = 0x00000000;
                case 010: /* JR */
                    set_PC(SR[rs = inst >> 21]);
                    JUMP
                case 015: /* BREAK */
                    *RSP.SP_STATUS_REG |= 0x00000003; /* BROKE | HALT */
                    if (*RSP.SP_STATUS_REG & 0x00000040)
                    { /* SP_STATUS_INTR_BREAK */
                        *RSP.MI_INTR_REG |= 0x00000001;
                        RSP.CheckInterrupts();
                    }
                    CONTINUE
                case 040: /* ADD */
                case 041: /* ADDU */
                    rs = inst >> 21;
                    SR[rd] = SR[rs] + SR[rt];
                    SR[0] = 0x00000000; /* needed for Rareware ucodes */
                    CONTINUE
                case 042: /* SUB */
                case 043: /* SUBU */
                    rs = inst >> 21;
                    SR[rd] = SR[rs] - SR[rt];
                    SR[0] = 0x00000000;
                    CONTINUE
                case 044: /* AND */
                    rs = inst >> 21;
                    SR[rd] = SR[rs] & SR[rt];
                    SR[0] = 0x00000000; /* needed for Rareware ucodes */
                    CONTINUE
                case 045: /* OR */
                    rs = inst >> 21;
                    SR[rd] = SR[rs] | SR[rt];
                    SR[0] = 0x00000000;
                    CONTINUE
                case 046: /* XOR */
                    rs = inst >> 21;
                    SR[rd] = SR[rs] ^ SR[rt];
                    SR[0] = 0x00000000;
                    CONTINUE
                case 047: /* NOR */
                    rs = inst >> 21;
                    SR[rd] = ~(SR[rs] | SR[rt]);
                    SR[0] = 0x00000000;
                    CONTINUE
                case 052: /* SLT */
                    rs = inst >> 21;
                    SR[rd] = ((signed)(SR[rs]) < (signed)(SR[rt]));
                    SR[0] = 0x00000000;
                    CONTINUE
                case 053: /* SLTU */
                    rs = inst >> 21;
                    SR[rd] = ((unsigned)(SR[rs]) < (unsigned)(SR[rt]));
                    SR[0] = 0x00000000;
                    CONTINUE
                default:
                    res_S();
                    CONTINUE
                }
                CONTINUE
            case 001: /* REGIMM */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                switch (rt)
                {
                case 020: /* BLTZAL */
                    SR[31] = (PC + LINK_OFF) & 0x00000FFC;
                case 000: /* BLTZ */
                    if (!(SR[rs] < 0))
                        CONTINUE
                    set_PC(PC + 4*inst + SLOT_OFF);
                    JUMP
                case 021: /* BGEZAL */
                    SR[31] = (PC + LINK_OFF) & 0x00000FFC;
                case 001: /* BGEZ */
                    if (!(SR[rs] >= 0))
                        CONTINUE
                    set_PC(PC + 4*inst + SLOT_OFF);
                    JUMP
                default:
                    res_S();
                    CONTINUE
                }
                CONTINUE
            case 003: /* JAL */
                SR[31] = (PC + LINK_OFF) & 0x00000FFC;
            case 002: /* J */
                set_PC(4*inst);
                JUMP
            case 004: /* BEQ */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                if (!(SR[rs] == SR[rt]))
                    CONTINUE
                set_PC(PC + 4*inst + SLOT_OFF);
                JUMP
            case 005: /* BNE */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                if (!(SR[rs] != SR[rt]))
                    CONTINUE
                set_PC(PC + 4*inst + SLOT_OFF);
                JUMP
            case 006: /* BLEZ */
                if (!((signed)SR[rs = (inst >> 21) & 31] <= 0x00000000))
                    CONTINUE
                set_PC(PC + 4*inst + SLOT_OFF);
                JUMP
            case 007: /* BGTZ */
                if (!((signed)SR[rs = (inst >> 21) & 31] >  0x00000000))
                    CONTINUE
                set_PC(PC + 4*inst + SLOT_OFF);
                JUMP
            case 010: /* ADDI */
            case 011: /* ADDIU */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = SR[rs] + (signed short)(inst);
                SR[0] = 0x00000000;
                CONTINUE
            case 012: /* SLTI */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = ((signed)(SR[rs]) < (signed short)(inst));
                SR[0] = 0x00000000;
                CONTINUE
            case 013: /* SLTIU */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = ((unsigned)(SR[rs]) < (unsigned short)(inst));
                SR[0] = 0x00000000;
                CONTINUE
            case 014: /* ANDI */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = SR[rs] & (unsigned short)(inst);
                SR[0] = 0x00000000;
                CONTINUE
            case 015: /* ORI */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = SR[rs] | (unsigned short)(inst);
                SR[0] = 0x00000000;
                CONTINUE
            case 016: /* XORI */
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                SR[rt] = SR[rs] ^ (unsigned short)(inst);
                SR[0] = 0x00000000;
                CONTINUE
            case 017: /* LUI */
                SR[rt = (inst >> 16) & 31] = inst << 16;
                SR[0] = 0x00000000;
                CONTINUE
            case 020: /* COP0 */
                rd = (inst & 0x0000F800u) >> 11;
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                switch (rs)
                {
                case 000: /* MFC0 */
                    MFC0(rt, rd & 0xF);
                    CONTINUE
                case 004: /* MTC0 */
                    MTC0[rd & 0xF](rt);
                    CONTINUE
                default:
                    res_S();
                    CONTINUE
                }
                CONTINUE
            case 022: /* COP2 */
                rd = (inst & 0x0000F800u) >> 11;
                rs = (inst >> 21) & 31;
                rt = (inst >> 16) & 31;
                switch (rs)
                {
                case 000: /* MFC2 */
                    MFC2(rt, rd, element);
                    CONTINUE
                case 002: /* CFC2 */
                    CFC2(rt, rd);
                    CONTINUE
                case 004: /* MTC2 */
                    MTC2(rt, rd, element);
                    CONTINUE
                case 006: /* CTC2 */
                    CTC2(rt, rd);
                    CONTINUE
                default:
                    res_S();
                    CONTINUE
                }
                CONTINUE
            case 040: /* LB */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                SR[rt] = RSP.DMEM[BES(addr)];
                SR[rt] = (signed char)(SR[rt]);
                SR[0] = 0x00000000;
                CONTINUE
            case 041: /* LH */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                if (addr%0x004 == 0x003)
                {
                    SR_B(rt, 2) = RSP.DMEM[addr - BES(0x000)];
                    addr = (addr + 0x00000001) & 0x00000FFF;
                    SR_B(rt, 3) = RSP.DMEM[addr + BES(0x000)];
                    SR[rt] = (signed short)(SR[rt]);
                }
                else
                {
                    addr -= HES(0x000)*(addr%0x004 - 1);
                    SR[rt] = *(signed short *)(RSP.DMEM + addr);
                }
                SR[0] = 0x00000000;
                CONTINUE
            case 043: /* LW */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                if (addr%0x004 != 0x000)
                    ULW(rt, addr);
                else
                    SR[rt] = *(int32_t *)(RSP.DMEM + addr);
                SR[0] = 0x00000000;
                CONTINUE
            case 044: /* LBU */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                SR[rt] = RSP.DMEM[BES(addr)];
                SR[rt] = (unsigned char)(SR[rt]);
                SR[0] = 0x00000000;
                CONTINUE
            case 045: /* LHU */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                if (addr%0x004 == 0x003)
                {
                    SR_B(rt, 2) = RSP.DMEM[addr - BES(0x000)];
                    addr = (addr + 0x00000001) & 0x00000FFF;
                    SR_B(rt, 3) = RSP.DMEM[addr + BES(0x000)];
                    SR[rt] = (unsigned short)(SR[rt]);
                }
                else
                {
                    addr -= HES(0x000)*(addr%0x004 - 1);
                    SR[rt] = *(unsigned short *)(RSP.DMEM + addr);
                }
                SR[0] = 0x00000000;
                CONTINUE
            case 050: /* SB */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                RSP.DMEM[BES(addr)] = (unsigned char)(SR[rt]);
                CONTINUE
            case 051: /* SH */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                if (addr%0x004 == 0x003)
                {
                    RSP.DMEM[addr - BES(0x000)] = SR_B(rt, 2);
                    addr = (addr + 0x00000001) & 0x00000FFF;
                    RSP.DMEM[addr + BES(0x000)] = SR_B(rt, 3);
                    CONTINUE
                }
                addr -= HES(0x000)*(addr%0x004 - 1);
                *(short *)(RSP.DMEM + addr) = (short)(SR[rt]);
                CONTINUE
            case 053: /* SW */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst);
                addr = (SR[base = (inst >> 21) & 31] + offset) & 0x00000FFF;
                if (addr%0x004 != 0x000)
                    USW(rt, addr);
                else
                    *(int32_t *)(RSP.DMEM + addr) = SR[rt];
                CONTINUE
            case 062: /* LWC2 */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst & 0x0000FFFFu);
#if defined(ARCH_MIN_SSE2)
                offset <<= 5 + 4; /* safe on x86, skips 5-bit rd, 4-bit element */
                offset >>= 5 + 4;
#else
                offset = SE(offset, 6);
#endif
                base = (inst >> 21) & 31;
                LWC2_op[rd = (inst & 0xF800u) >> 11](rt, element, offset, base);
                CONTINUE
            case 072: /* SWC2 */
                rt = (inst >> 16) % (1 << 5);
                offset = (signed short)(inst & 0x0000FFFFu);
#if defined(ARCH_MIN_SSE2)
                offset <<= 5 + 4; /* safe on x86, skips 5-bit rd, 4-bit element */
                offset >>= 5 + 4;
#else
                offset = SE(offset, 6);
#endif
                base = (inst >> 21) & 31;
                SWC2_op[rd = (inst & 0xF800u) >> 11](rt, element, offset, base);
                CONTINUE
            default:
                res_S();
                CONTINUE
            }
        }
#ifndef EMULATE_STATIC_PC
        if (stage == 2) /* branch phase of scheduler */
        {
            stage = 0*stage;
            PC = temp_PC & 0x00000FFC;
            *RSP.SP_PC_REG = temp_PC;
        }
        else
        {
            stage = 2*stage; /* next IW in branch delay slot? */
            PC = (PC + 0x004) & 0xFFC;
            *RSP.SP_PC_REG = 0x04001000 + PC;
        }
        continue;
#else
        continue;
BRANCH:
        inst = *(uint32_t *)(RSP.IMEM + FIT_IMEM(PC));
        PC = temp_PC & 0x00000FFC;
        goto EX;
#endif
    }
    *RSP.SP_PC_REG = 0x04001000 | FIT_IMEM(PC);
    if (*RSP.SP_STATUS_REG & 0x00000002) /* normal exit, from executing BREAK */
        return;
    else if (*RSP.MI_INTR_REG & 0x00000001) /* interrupt set by MTC0 to break */
        RSP.CheckInterrupts();
    else if (stale_signals != 0) /* too many iterations of MFC0:  timed out */
        MF_SP_STATUS_TIMEOUT = 16384; /* This is slow:  Make 16 if it works. */
    else if (*RSP.SP_SEMAPHORE_REG != 0x00000000) /* semaphore lock fixes */
        {}
    else /* ??? unknown, possibly external intervention from CPU memory map */
        return; /* SP_SET_HALT */
    *RSP.SP_STATUS_REG &= ~0x00000001; /* CPU restarts with the correct SIGs. */
    return;
}
