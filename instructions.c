#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "define.h"
#include "instructions.h"
#include "inst2text.h"
#include "state.h"

struct instruction parse_instruction(struct state *context, struct debug_state *dbg_context, word_T loc) {
        word_T inst = get_word_at(context, dbg_context, loc);

        struct instruction ret;
        ret.address = loc;
        ret.width = 2;
        ret.op = 0;             // aid in debugging

        char opcode = (inst & 0xf000) >> 12; 
        if (opcode == 0x0) {
                // TI spec: no 0-opcodes
                ret.cat = CAT_SINGLEOP;
                switch ((inst & 0x0030) >> 8) {
                case 0: {
                        ret.name = "rrc";
                        ret.op = rrc_inst;
                        break; }
                case 1: {
                        ret.name = "rra";
                        ret.op = rra_inst;
                        break; }
                case 2: {
                        ret.name = "push";
                        ret.op = push_inst;
                        break; }
                case 3: {
                        ret.name = "reti";
                        ret.op = reti_inst;
                        break; }
                default: 
                        ; /* shut up compiler */
                }

                struct non_jumpop ret_specific;
                ret_specific.op_size = !((inst & 0x40) >> 6) + 1;
                ret_specific.Ad = (inst & 0x30) >> 4;
                ret_specific.As = (inst & 0x30) >> 4;
                ret_specific.d_reg = (inst & 0xf);
                ret_specific.s_reg = (inst & 0xf);
                if (ret_specific.Ad == MODE_INDEXED || (ret_specific.s_reg == REG_PC && ret_specific.As == MODE_INDIRECT_ADD)) {
                        ret.width += 2;
                        ret_specific.src_index = get_word_at(context, NULL, loc + 2);
                        ret_specific.dst_index = get_word_at(context, NULL, loc + 2);
                } else {
                        ret_specific.src_index = 0;
                        ret_specific.dst_index = 0;
                }

                ret.specific.non_jump = ret_specific;

        } else if (opcode == 0x1) {
                ret.cat = CAT_SINGLEOP;
                // 15       65 3  0
                // +--------++-+--+
                // |        || |  |
                // +--------++-+--+
                // [15:7]       opcode
                // [6]          B/W
                // [5:4]        Ad
                // [3:0]        D/S-reg

                switch ((inst >> 7) & 0x7) {
                case 0: {
                        ret.name = "rrc";
                        ret.op = rrc_inst;
                        break; }
                case 1: {
                        ret.name = "swpb";
                        ret.op = swpb_inst;
                        break; }
                case 2: {
                        ret.name = "rra";
                        ret.op = rra_inst;
                        break; }
                case 3: {
                        ret.name = "sxt";
                        ret.op = sxt_inst;
                        break; }
                case 4: {
                        ret.name = "push";
                        ret.op = push_inst;
                        break; }
                case 5: {
                        ret.name = "call";
                        ret.op = call_inst;
                        break; }
                case 6: {
                        ret.name = "reti";
                        ret.op = reti_inst;
                        break; }
                /* there is no case 7 */
                default: 
                        ; /* shut up compiler */
                }

                struct non_jumpop ret_specific;
                ret_specific.op_size = !((inst & 0x40) >> 6) + 1;
                ret_specific.Ad = (inst & 0x30) >> 4;
                ret_specific.As = (inst & 0x30) >> 4;
                ret_specific.d_reg = (inst & 0xf);
                ret_specific.s_reg = (inst & 0xf);
                if ((ret_specific.Ad == MODE_INDEXED && ret_specific.d_reg != REG_CG) || (ret_specific.s_reg == REG_PC && ret_specific.As == MODE_INDIRECT_ADD)) {
                        ret.width += 2;
                        ret_specific.src_index = get_word_at(context, NULL, loc + 2);
                        ret_specific.dst_index = get_word_at(context, NULL, loc + 2);
                } else {
                        ret_specific.src_index = 0;
                        ret_specific.dst_index = 0;
                }

                ret.specific.non_jump = ret_specific;
        } else if (opcode == 0x2 || opcode == 0x3) {
                ret.cat = CAT_JUMP;
                // 15 12 9        0
                // +--+--+--------+
                // |  |  |        |
                // +--+--+--------+
                // [15:13]:     opcode
                // [12:10]:     jump type
                // [9:0]:       pc offset

                switch ((inst >> 10) & 0x7) {
                case 0: {
                        ret.name = "jnz";
                        ret.op = jnz_inst;
                        break; }
                case 1: {
                        ret.name = "jz";
                        ret.op = jz_inst;
                        break; }
                case 2: {
                        ret.name = "jnc";
                        ret.op = jnc_inst;
                        break; }
                case 3: {
                        ret.name = "jc";
                        ret.op = jc_inst;
                        break; }
                case 4: {
                        ret.name = "jn";
                        ret.op = jn_inst;
                        break; }
                case 5: {
                        ret.name = "jge";
                        ret.op = jge_inst;
                        break; }
                case 6: {
                        ret.name = "jl";
                        ret.op = jl_inst;
                        break; }
                case 7: {
                        ret.name = "jmp";
                        ret.op = jmp_inst;
                        break; }
                default: 
                        ; /* shut up compiler */
                }

                struct jumpop ret_specific;
                word_T offset = inst & 0x03ff;
                if (inst & 0x0200) {
                        offset = (word_T)(offset | 0xfc00);
                }

                ret_specific.offset = offset;
                ret.specific.jump = ret_specific;

        } else {
                ret.cat = CAT_DOUBLEOP;
                // 15  11  765 3  0
                // +---+---+++-+--+
                // |   |   ||| |  |
                // +---+---+++-+--+
                // [15:12]      opcode
                // [11:8]       S-reg
                // [7]          Ad
                // [6]          B/W
                // [5:4]        As
                // [3:0]        D-reg

                switch ((inst & 0xf000) >> 12) {
                case 4: {
                        ret.name = "mov";
                        ret.op = mov_inst;
                        break; }
                case 5: {
                        ret.name = "add";
                        ret.op = add_inst;
                        break; }
                case 6: {
                        ret.name = "addc";
                        ret.op = addc_inst;
                        break; }
                case 7: {
                        ret.name = "subc";
                        ret.op = subc_inst;
                        break; }
                case 8: {
                        ret.name = "sub";
                        ret.op = sub_inst;
                        break; }
                case 9: {
                        ret.name = "cmp";
                        ret.op = cmp_inst;
                        break; }
                case 10: {
                        ret.name = "dadd";
                        ret.op = dadd_inst;
                        break; }
                case 11: {
                        ret.name = "bit";
                        ret.op = bit_inst;
                        break; }
                case 12: {
                        ret.name = "bic";
                        ret.op = bic_inst;
                        break; }
                case 13: {
                        ret.name = "bis";
                        ret.op = bis_inst;
                        break; }
                case 14: {
                        ret.name = "xor";
                        ret.op = xor_inst;
                        break; }
                case 15: {
                        ret.name = "and";
                        ret.op = and_inst;
                        break; }
                default:
                        ; /* shut up compiler */
                }

                struct non_jumpop ret_specific;
                
                ret_specific.Ad = (inst & 0x80) >> 7;
                ret_specific.op_size = !((inst & 0x40) >> 6) + 1;
                ret_specific.As = (inst & 0x30) >> 4;
                ret_specific.d_reg = inst & 0x0f;
                ret_specific.s_reg = (inst & 0x0f00) >> 8;

                if (((ret_specific.As == MODE_INDEXED && ret_specific.s_reg != REG_CG) || (ret_specific.As == MODE_INDIRECT_ADD && ret_specific.s_reg == REG_PC)) && 
                      ret_specific.Ad == MODE_INDEXED) {
                        ret.width += 4;
                        ret_specific.src_index = get_word_at(context, NULL, loc + 2);
                        ret_specific.dst_index = get_word_at(context, NULL, loc + 4);
                } else if ((ret_specific.As == MODE_INDEXED && ret_specific.s_reg != REG_CG) || (ret_specific.As == MODE_INDIRECT_ADD && ret_specific.s_reg == REG_PC)) {
                        ret.width += 2;
                        ret_specific.src_index = get_word_at(context, NULL, loc + 2);
                        ret_specific.dst_index = 0;
                } else if (ret_specific.Ad == MODE_INDEXED) {
                        ret.width += 2;
                        ret_specific.src_index = 0;
                        ret_specific.dst_index = get_word_at(context, NULL, loc + 2);
                } else {
                        ret_specific.src_index = 0;
                        ret_specific.dst_index = 0;
                }

                ret.specific.non_jump = ret_specific;
        }

        return ret;
}

void do_next_instruction(struct state *context, struct debug_state *dbg_context) {
        word_T pc = get_reg(context, dbg_context, REG_PC);
        struct instruction inst = parse_instruction(context, dbg_context, pc);

        if (dbg_context->logfile) {
                char instbuf[MAX_INSTRUCTION_LENGTH];
                char annotationbuf[MAX_ANNOTATION_LENGTH];

                instruction2string(inst, instbuf);
                annotation2string(context, inst, annotationbuf, strlen(instbuf));

                fprintf(dbg_context->logfile, "%04x: %s%s\n", pc & 0xffff, instbuf, annotationbuf);
        }
        
        if (!dep_exec_on(context, pc)) {
                printf(FROM_ERROR("segfault: cannot execute write-only page\n"));
                context->crashed = 1;
        } else {
                inst.op(context, dbg_context, &inst);
                if ((inst.op != call_inst) && !(inst.cat == CAT_DOUBLEOP && inst.op == mov_inst && inst.specific.non_jump.d_reg == REG_PC && inst.specific.non_jump.Ad == MODE_DIRECT)) {
                        delta_reg(context, dbg_context, REG_PC, inst.width);
                }
        }
}

/* gets value from source, incrementing if necessary */
word_T perform_src_calc (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {

        word_T ret = 0;

        enum operand_size size = inst->specific.non_jump.op_size;
        word_T src_index = inst->specific.non_jump.src_index;
        enum instruction_operand reg = inst->specific.non_jump.s_reg;
        enum address_mode mode = inst->specific.non_jump.As;

        if (reg == REG_CG) {
                switch(mode) {
                case MODE_DIRECT: {
                        ret = 0;
                        break; }
                case MODE_INDEXED: {
                        ret = 1;
                        break; }
                case MODE_INDIRECT: {
                        ret = 2;
                        break; }
                case MODE_INDIRECT_ADD: {
                        ret = -1;
                        break; }
                default: 
                        ; /* shut up compiler */
                }

                return ret;
        }

        if (mode == MODE_DIRECT) {
                ret = get_reg(context, dbg_context, reg);
                if (reg == REG_PC) {
                        ret += 2;
                }
        } else {
                word_T addr;
                switch(reg) {
                case REG_PC: {
                        switch(mode) {
                        case MODE_INDEXED: {
                                addr = get_reg(context, dbg_context, REG_PC) + src_index;
                                break; }
                        case MODE_INDIRECT: {
                                addr = get_reg(context, dbg_context, REG_PC) + inst->width;
                                break; }
                        case MODE_INDIRECT_ADD: {
                                ret = src_index;
                                break; }
                        default: ;
                        }
                        break; }
                case REG_SR: {
                        switch(mode) {
                        case MODE_INDEXED: {
                                addr = src_index;
                                break; }
                        case MODE_INDIRECT: {
                                ret = 4;
                                break; }
                        case MODE_INDIRECT_ADD: {
                                ret = 8;
                                break; }
                        default: ;
                        }
                        break; }
                default:
                        addr = get_reg(context, dbg_context, reg) + src_index;
                }

                if (!((reg == REG_SR && mode != MODE_INDEXED) || (reg == REG_PC && mode == MODE_INDIRECT_ADD))) {
                        if (size == SIZE_WORD) {
                                ret = get_word_at(context, dbg_context, addr);
                        } else {
                                ret = get_byte_at(context, dbg_context, addr);
                        }
                }

                if (mode == MODE_INDIRECT_ADD && reg != REG_PC && reg != REG_SR) {
                        if (reg == REG_SP) {
                                delta_reg(context, dbg_context, reg, 2);
                        } else if (reg != REG_PC) {
                                delta_reg(context, dbg_context, reg, size);
                        }
                }
        }

        return ret;
}

word_T doubleop_get_dst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {

        enum operand_size size = inst->specific.non_jump.op_size;
        word_T dst_index = inst->specific.non_jump.dst_index;
        enum instruction_operand reg = inst->specific.non_jump.d_reg;
        enum address_mode mode = inst->specific.non_jump.Ad;

        word_T ret;
        if (mode == MODE_DIRECT && size == SIZE_BYTE) {
                ret = get_reg(context, dbg_context, reg) & 0xff;
        } else if (mode == MODE_DIRECT) {
                ret = get_reg(context, dbg_context, reg);
        } else if (reg == REG_SR && size == SIZE_BYTE) {
                ret = get_byte_at(context, dbg_context, inst->specific.non_jump.dst_index) & 0xff;
        } else if (reg == REG_SR && size == SIZE_WORD) {
                ret = get_word_at(context, dbg_context, inst->specific.non_jump.dst_index);
        } else if (size == SIZE_BYTE) {
                ret = get_byte_at(context, dbg_context, get_reg(context, dbg_context, reg) + dst_index) & 0xff;
        } else {
                /* reg != OP_SR && size == SIZE_WORD */
                ret = get_word_at(context, dbg_context, get_reg(context, dbg_context, reg) + dst_index);
        }
        
        
        return ret;
}

enum result write_to_dst(struct state *context, struct debug_state *dbg_context, struct instruction *inst, word_T val) {
        enum operand_size size = inst->specific.non_jump.op_size;
        word_T dst_index = inst->specific.non_jump.dst_index;
        enum instruction_operand reg = inst->specific.non_jump.d_reg;
        enum address_mode mode = inst->specific.non_jump.Ad;
        enum instruction_category cat = inst->cat;

        enum result ret = RESULT_SUCCESS;

        switch(mode) {
        case MODE_DIRECT: {
                set_reg(context, dbg_context, reg, val);
                break; }
        case MODE_INDEXED: {
                char charval = (char)(val & 0xff);
                if (reg == REG_SR && size == SIZE_BYTE) {
                        set_byte_at(context, dbg_context, dst_index, charval);
                } else if (reg == REG_SR && size == SIZE_WORD) {
                        set_word_at(context, dbg_context, dst_index, val);
                } else if (reg != REG_SR && size == SIZE_BYTE) {
                        set_byte_at(context, dbg_context, get_reg(context, dbg_context, reg) + dst_index, charval);
                } else if (reg != REG_SR && size == SIZE_WORD) {
                                set_word_at(context, dbg_context, get_reg(context, dbg_context, reg) + dst_index, val);
                }
                break; }
        case MODE_INDIRECT: {
                assert (cat == CAT_SINGLEOP);
                // VV not sure if needed yet
                // if (reg != OP_PC && reg != OP_SR) {
                if (size == SIZE_BYTE) {
                        char charval = (char) (val & 0xff);
                        set_byte_at(context, dbg_context, get_reg(context, dbg_context, reg), charval);
                } else {
                        set_word_at(context, dbg_context, get_reg(context, dbg_context, reg), val);
                }
                break; }
        case MODE_INDIRECT_ADD: {
                assert (cat == CAT_SINGLEOP);
                // VV not sure if needed yet
                // if (dst != OP_PC && dst != OP_SR) {
                if (size == SIZE_BYTE) {
                        char charval = (char) (val & 0xff);
                        set_byte_at(context, dbg_context, get_reg(context, dbg_context, reg), charval);
                        delta_reg(context, dbg_context, reg, 1);
                } else {
                        set_word_at(context, dbg_context, get_reg(context, dbg_context, reg), val);
                        delta_reg(context, dbg_context, reg, 2);
                }
                break; }
        default:
                ; /* shut up compiler */
        }

        return ret;
}

void compute_negative_flag(struct state *context, struct debug_state *dbg_context, word_T val, enum operand_size size) {
        if (size == SIZE_WORD) {
                set_flag(context, dbg_context, FLAG_N, val & 0x8000);
        } else {
                set_flag(context, dbg_context, FLAG_N, val & 0x0080);
        }
}

void compute_zero_flag(struct state *context, struct debug_state *dbg_context, word_T val) {
        set_flag(context, dbg_context, FLAG_Z, !val);
}

void compute_zero_and_carry_flag(struct state *context, struct debug_state *dbg_context, word_T val) {
        set_flag(context, dbg_context, FLAG_Z, !val);
        set_flag(context, dbg_context, FLAG_C, val);
}

enum compute_op { COMPUTE_ADD, COMPUTE_SUBTRACT };
void compute_carry_flag(struct state *context, struct debug_state *dbg_context, 
                                word_T lhs, word_T rhs, 
                                enum operand_size op_size, 
                                enum compute_op op) {
        unsigned int lhs_extend = (unsigned int) lhs;
        unsigned int rhs_extend = (unsigned int) rhs;

        if (op == COMPUTE_ADD) {
                if (op_size == SIZE_WORD) {
                        set_flag(context, dbg_context, FLAG_C, lhs_extend + rhs_extend > 0xffff);
                } else {
                        set_flag(context, dbg_context, FLAG_C, lhs_extend + rhs_extend > 0xff);
                }
        } else {
                set_flag(context, dbg_context, FLAG_C, lhs_extend <= rhs_extend);
        }
}

void compute_overflow_flag(struct state *context, struct debug_state *dbg_context, word_T lhs, word_T rhs, word_T result, enum operand_size op_size, enum compute_op op) {
        int lhs_sign, rhs_sign, result_sign;
        if (op_size == SIZE_WORD) {
                lhs_sign = (lhs & 0x8000) >> 15;
                rhs_sign = (rhs & 0x8000) >> 15;
                result_sign = (result & 0x8000) >> 15;
        } else {
                lhs_sign = (lhs & 0x80) >> 7;
                rhs_sign = (rhs & 0x80) >> 7;
                result_sign = (result & 0x80) >> 7;
        }

        if (op == COMPUTE_ADD) {
                set_flag(context, dbg_context, FLAG_V, 
                                        ((lhs_sign == 0 && rhs_sign == 0 && result_sign == 1) ||
                                        (lhs_sign == 1 && rhs_sign == 1 && result_sign == 0)));
        } else {
                set_flag(context, dbg_context, FLAG_V, 
                                        ((lhs_sign == 0 && rhs_sign == 1 && result_sign == 1) ||
                                        (lhs_sign == 1 && rhs_sign == 0 && result_sign == 0)));
        }
}


/////////
// INSTRUCTION SPECIFIC FUNCTIONS
/////////

// DONE
// manual page 86
void mov_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T check_breakpoint = doubleop_get_dst(context, dbg_context, inst); ++check_breakpoint;
        word_T val = perform_src_calc (context, dbg_context, inst);

        if (inst->specific.non_jump.op_size == SIZE_BYTE) {
                val = val & 0xff; 
        }

        write_to_dst(context, dbg_context, inst, val);
        // does not affect flags
}

// DONE
// manual page 56
void add_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc (context, dbg_context, inst);
        word_T val;
        if (inst->specific.non_jump.op_size == SIZE_WORD) {
                val = src + dst;
        } else {
                val = (src + dst) & 0xff;
        }
        
        // flags
        compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        compute_zero_flag(context, dbg_context, val);
        compute_carry_flag(context, dbg_context, src, dst, inst->specific.non_jump.op_size, COMPUTE_ADD);
        compute_overflow_flag(context, dbg_context, src, dst, val, inst->specific.non_jump.op_size, COMPUTE_ADD);

        // this hack brought to you by bangalore:44c6
        word_T prev_sr = get_reg(context, dbg_context, REG_SR);
        set_reg(context, dbg_context, REG_SR, prev_sr & 0x00ff);

        write_to_dst(context, dbg_context, inst, val);
}

// manual page 57
void addc_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        /* never used in any challenges */
}

// manual page 101
void subc_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        /* never used in any challenges */
}

// manual page 100
void sub_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = dst - src;
        
        // TI spec: compute negative flag for sub.b instructions as well 
        if (inst->specific.non_jump.op_size == SIZE_WORD) {
                compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        } else {
                set_flag(context, dbg_context, FLAG_N, OFF_FLAG);
        }

        compute_zero_flag(context, dbg_context, val);
        compute_carry_flag(context, dbg_context, src, dst, inst->specific.non_jump.op_size, COMPUTE_SUBTRACT);
        // TI spec: computes overflow flag
        write_to_dst(context, dbg_context, inst, val);
}

// DONE
// manual page 68
void cmp_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = dst - src;

        // TI spec: compute negative flag for cmp.b instructions as well
        if (inst->specific.non_jump.op_size == SIZE_WORD) {
                compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        } else {
                set_flag(context, dbg_context, FLAG_N, OFF_FLAG);
        }
        compute_zero_flag(context, dbg_context, val);
        compute_carry_flag(context, dbg_context, src, dst, inst->specific.non_jump.op_size, COMPUTE_SUBTRACT);
        compute_overflow_flag(context, dbg_context, src, dst, val, inst->specific.non_jump.op_size, COMPUTE_SUBTRACT);
}

// manual page 70
void dadd_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) { 
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);

        /*
         * reverse engineered algorithm:
         * add each digit normally
         * if sum >= 10, set carry, subtract 10
         * summand digit is least significant 4 bits of value post-subtract-10

         * doesn't seem to change carry flag as manual suggests
         */

        int val = 0; 

        char carry = 0;
        word_T mask;
        int digit;
        // int stop_point = inst->specific.non_jump.op_size == SIZE_WORD ? 4 : 2;

        for (digit = 0, mask = 0x000f; digit < (inst->specific.non_jump.op_size == SIZE_WORD ? 4 : 2); ++digit, mask = mask << 4) {
                char cur_bcd1 = (dst & mask) >> (digit * 4);
                char cur_bcd2 = (src & mask) >> (digit * 4);

                char sum = carry + cur_bcd1 + cur_bcd2;
                if (sum >= 10) {
                        carry = 1;
                        sum -= 10;
                } else {
                        carry = 0;
                }

                val |= (sum & 0x0f) << (digit * 4);
        }

        // TI spec: compute negative flag
        // TI spec: change carry flag

        compute_zero_flag(context, dbg_context, (word_T)(val & 0xffff));
        set_flag(context, dbg_context, FLAG_C, carry);

        write_to_dst(context, dbg_context, inst, (word_T)(val & 0xffff));
}

// DONE
// manual page 61
void bit_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = src & dst;

        compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        compute_zero_and_carry_flag(context, dbg_context, val);
        set_flag(context, dbg_context, FLAG_V, OFF_FLAG);
}

// DONE
// manual page 59
void bic_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = (~src) & dst;

        // does not affect SR

        write_to_dst(context, dbg_context, inst, val);
}

// DONE
// manual page 60
void bis_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = src | dst;

        // does not affect SR

        write_to_dst(context, dbg_context, inst, val);
}

// DONE
// manual page 105
void xor_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = src ^ dst;

        compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        compute_zero_and_carry_flag(context, dbg_context, val);
        // TI spec: compute overflow flag

        write_to_dst(context, dbg_context, inst, val);
}

// DONE
// manual page 58
void and_inst (struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = doubleop_get_dst(context, dbg_context, inst);
        word_T src = perform_src_calc(context, dbg_context, inst);
        word_T val = src & dst;

        compute_negative_flag(context, dbg_context, val, inst->specific.non_jump.op_size);
        compute_zero_and_carry_flag(context, dbg_context, val);
        set_flag(context, dbg_context, FLAG_V, OFF_FLAG);

        write_to_dst(context, dbg_context, inst, val);
}


////////
// SINGLEOP INSTRUCTIONS
///////

// DONE
// manual page 95
void rrc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        enum operand_size size = inst->specific.non_jump.op_size;

        word_T dst = perform_src_calc(context, dbg_context, inst);
        char lsb = dst & 0x1;
        dst >>= 1;

        if (size == SIZE_WORD) {
                dst |= (get_flag(context, FLAG_C) << 15);
        } else {
                dst |= (get_flag(context, FLAG_C) << 7);
        }

        set_flag(context, dbg_context, FLAG_C, lsb);
        compute_zero_flag(context, dbg_context, dst);
        // matasano rrc N flag logic:
        // if negative flag is set, keep it set (even if result is not negative)
        // if negative flag is not set, then set if result is negative

        if (((size == SIZE_WORD) && (dst & 0x8000)) ||
            ((size == SIZE_BYTE) && (dst & 0x0080))) {
                set_flag(context, dbg_context, FLAG_N, ON_FLAG);
        }

        set_flag(context, dbg_context, FLAG_V, OFF_FLAG);

        write_to_dst(context, dbg_context, inst, dst);
}

// DONE
// manual page 102
void swpb_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = perform_src_calc (context, dbg_context, inst);
        word_T tmp = dst & 0xff00;
        dst <<= 8;
        dst |= (tmp >> 8);
        write_to_dst(context, dbg_context, inst, dst);

        // does not affect SR
}

// DONE
// manual page 94
void rra_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = perform_src_calc(context, dbg_context, inst);
        // char lsb = dst & 0x1;
        // char msb;
        word_T msb;

        if (inst->specific.non_jump.op_size == SIZE_BYTE) {
                msb = dst & 0x80;
        } else {
                // msb = (word_T)(dst & 0x8000);
                msb = dst & 0x8000;
        }

        dst >>= 1;

        dst |= msb;
        
        // TI spec: compute negative flag
        // TI spec: set carry to lsb
        compute_zero_flag(context, dbg_context, dst);
        set_flag(context, dbg_context, FLAG_V, OFF_FLAG);

        write_to_dst(context, dbg_context, inst, dst);
}

// DONE
// manual page 103
void sxt_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T dst = perform_src_calc (context, dbg_context, inst);
        int sign_bit = (dst & 0x80);
        if (sign_bit) { 
                dst |= 0xff00;
        } else {
                dst &= 0x00ff;
        }

        compute_negative_flag(context, dbg_context, dst, inst->specific.non_jump.op_size);
        compute_zero_and_carry_flag(context, dbg_context, dst);
        set_flag(context, dbg_context, FLAG_V, OFF_FLAG);

        write_to_dst(context, dbg_context, inst, dst);
}

// DONE
// manual page 89
void push_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T val = perform_src_calc (context, dbg_context, inst);
        // TODO: what if pushing to memory being watched
        delta_reg(context, dbg_context, REG_SP, -2);
        set_word_at(context, dbg_context, context->reg[REG_SP], val);
        // does not affect status bits
}

// DONE
// manual page 63
void call_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        word_T val = perform_src_calc (context, dbg_context, inst);

        delta_reg(context, dbg_context, REG_SP, -2);
        // add width to return to instruction *after* this one
        set_word_at(context, dbg_context, context->reg[REG_SP], context->reg[REG_PC] + inst->width);
        set_reg(context, dbg_context, REG_PC, val);

        // does not affect status bits
}

// manual page 91
void reti_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {

}

////////
// JUMP INSTRUCTIONS
///////

// DONE
// manual page 85
void jnz_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (!get_flag(context, FLAG_Z)) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
// manual page 79
void jz_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (get_flag(context, FLAG_Z)) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
// manual page 84
void jnc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (!get_flag(context, FLAG_C)) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
// manual page 78
void jc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (get_flag(context, FLAG_C)) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
// manual page 83
void jn_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (get_flag(context, FLAG_N)) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}


// DONE
// manual page 80
void jge_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if (!(get_flag(context, FLAG_N) ^ get_flag(context, FLAG_V))) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
//manual page 81
void jl_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        if ((get_flag(context, FLAG_N) ^ get_flag(context, FLAG_V))) {
                delta_reg(context, dbg_context, REG_PC, 2 * inst->specific.jump.offset);
        }
}

// DONE
// manual page 82
void jmp_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst) {
        delta_reg(context, dbg_context, REG_PC, 2 * (inst->specific.jump.offset));
}
