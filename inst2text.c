#include "inst2text.h"

void operand2string(enum instruction_operand reg, enum address_mode mode, word_T index, char *buf);
void reg2string(enum instruction_operand reg, char *buf);

void reg2string(enum instruction_operand reg, char *buf) {
        switch(reg) {
        case REG_PC: {
                strcpy(buf, "pc");
                break; }
        case REG_SP: {
                strcpy(buf, "sp");
                break; }
        case REG_SR: {
                strcpy(buf, "sr");
                break; }
        default: 
                sprintf(buf, "r%d", reg); 
        }
}


void operand2string(enum instruction_operand reg, enum address_mode mode, word_T index, char *buf) {
        char regbuf[4];

        reg2string(reg, regbuf);

        switch(mode) {
        case MODE_DIRECT: {
                if (reg == REG_CG) {
                        strcpy(buf, "#0x0");
                } else {
                        strcpy(buf, regbuf);
                }
                break; }
        case MODE_INDEXED: {
                if (reg == REG_CG) {
                        strcpy(buf, "#0x1");
                } else if (reg == REG_SR) {
                        sprintf(buf, "&0x%04x", index & 0xffff);
                } else {
                        sprintf(buf, "0x%04x(%s)", index & 0xffff, regbuf);
                }

                break; }
        case MODE_INDIRECT: {
                if (reg == REG_CG) {
                        strcpy(buf, "#0x2");
                } else if (reg == REG_SR) {
                        strcpy(buf, "#0x4");
                } else {
                        strcpy(buf, "@");
                        strcat(buf + 1, regbuf);
                }
                break; }
        case MODE_INDIRECT_ADD: {
                if (reg == REG_PC) {
                        sprintf(buf, "#0x%04x", index & 0xffff);
                } else if (reg == REG_CG) {
                        strcpy(buf, "#-0x1");
                } else if (reg == REG_SR) {
                        strcpy(buf, "#0x8");
                } else {
                        buf[0] = '@';
                        strcpy(buf + 1, regbuf);
                        strcat(buf, "+");
                }
                break; }
        default:
                ;
        }
}

int emulated_instruction2string(struct instruction inst, char *buf) {
        char opbuf[MAX_OP_LENGTH];

        int ret = 0;
        if (inst.cat == CAT_DOUBLEOP) {
                if (!strcmp(inst.name, "mov") && 
                        inst.specific.non_jump.s_reg == REG_SP &&
                        inst.specific.non_jump.As == MODE_INDIRECT_ADD &&
                        inst.specific.non_jump.d_reg == REG_PC) {

                        strcpy(buf, "ret");
                        ret = 1;
                } else if (!strcmp(inst.name, "mov") &&
                        inst.specific.non_jump.s_reg == REG_SP &&
                        inst.specific.non_jump.As == MODE_INDIRECT_ADD) {

                        operand2string(inst.specific.non_jump.d_reg, inst.specific.non_jump.Ad, inst.specific.non_jump.dst_index, opbuf);
                        strcpy(buf, "pop");
                        if (inst.specific.non_jump.op_size == SIZE_BYTE) {
                                strcat(buf, ".b");
                                strcat(buf, FIVE_LETTER_PAD);
                        } else {
                                strcat(buf, THREE_LETTER_PAD);
                        }
                        strcat(buf, opbuf);
                        ret = 1;
                }
        }

        return ret;
}

void instruction2string(struct instruction inst, char *buf) {
        buf[0] = '\x00';
        if (!emulated_instruction2string(inst, buf)) {
                strcpy (buf, inst.name);
                if (inst.cat != CAT_JUMP && inst.specific.non_jump.op_size == SIZE_BYTE) {
                        strcat(buf, ".b");
                }

                switch(strlen(buf)) {
                case 2: {
                        strcat(buf, TWO_LETTER_PAD);
                        break; }
                case 3: {
                        strcat(buf, THREE_LETTER_PAD);
                        break; }
                case 4: {
                        strcat(buf, FOUR_LETTER_PAD);
                        break; }
                case 5: {
                        strcat(buf, FIVE_LETTER_PAD);
                        break; }
                case 6: {
                        strcat(buf, SIX_LETTER_PAD);
                        break; }
                default:
                        ;
                }

                char opbuf[MAX_OP_LENGTH];
                switch(inst.cat) {
                case CAT_SINGLEOP: {
                        operand2string(inst.specific.non_jump.s_reg, inst.specific.non_jump.As, inst.specific.non_jump.src_index, opbuf);
                        strcat(buf, opbuf);
                        break; }
                case CAT_DOUBLEOP: {
                        operand2string(inst.specific.non_jump.s_reg, inst.specific.non_jump.As, inst.specific.non_jump.src_index, opbuf);
                        strcat(buf, opbuf);

                        strcat(buf, ", ");

                        operand2string(inst.specific.non_jump.d_reg, inst.specific.non_jump.Ad, inst.specific.non_jump.dst_index, opbuf);
                        strcat(buf, opbuf);
                        break; }
                case CAT_JUMP: {
                        sprintf(opbuf, "#0x%04x", (inst.address + 2 * (inst.specific.jump.offset + 1)) & 0xffff);
                        strcat(buf, opbuf);
                        break; }
                default:
                        ;
                }
        }
}

void annotation2string(struct state *context, struct instruction inst, char *buf, int prev_length) {
        buf[0] = '\x00';
        int i;
        if (inst.cat == CAT_JUMP) {
                return;
        }

        for (i=0; i < MAX_INSTRUCTION_LENGTH - prev_length; ++i) {
                buf[i] = ' ';
        }

        buf[MAX_INSTRUCTION_LENGTH - prev_length] = '\x00';
        strcat(buf, ";  ");

        if (inst.specific.non_jump.s_reg != REG_CG) {
                word_T indirect, direct;
                direct = get_reg(context, NULL, inst.specific.non_jump.s_reg);
                indirect = get_word_at(context, NULL, get_reg(context, NULL, inst.specific.non_jump.s_reg) + inst.specific.non_jump.src_index);

                sprintf(buf + strlen(buf), "src: DIRECT 0x%04x INDIRECT 0x%04x  ||  ", direct, indirect);
        }

        if (inst.cat == CAT_DOUBLEOP && inst.specific.non_jump.d_reg != REG_CG) {
                word_T indirect, direct;
                direct = get_reg(context, NULL, inst.specific.non_jump.d_reg);
                indirect = get_word_at(context, NULL, get_reg(context, NULL, inst.specific.non_jump.d_reg) + inst.specific.non_jump.dst_index);

                sprintf(buf + strlen(buf), "dst: DIRECT 0x%04x INDIRECT 0x%04x", direct, indirect);

        }

        sprintf(buf + strlen(buf), "  ||  sr: 0x%04x", context->reg[REG_SR]);
}

void disassemble(struct state *context, word_T start, int num_insts, FILE *outfile) {
        int i;
        struct instruction inst;
        word_T cur = start;

        char instbuf[MAX_INSTRUCTION_LENGTH];

        for (i = 0; i < num_insts; ++i) {
                inst = parse_instruction(context, NULL, cur); 
                instruction2string(inst, instbuf);
                fprintf(outfile, "%04x: %s\n", cur & 0xffff, instbuf);
                cur += inst.width;
        }
}
