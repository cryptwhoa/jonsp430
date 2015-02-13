#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug-state.h"
#include "define.h"
#include "instructions.h"
#include "interrupts.h"
#include "state.h"

// INITIALIZATION
enum result load_file(struct state *context) {
        FILE *infile = fopen(context->asm_filename, "r");
        if (infile == NULL) {
                perror("fopen");
                return RESULT_FAILURE;
        }

        int buf[2];
        char byte[2];
        int cur;

        int i = 0;
        int done = 0;
        while (!done) {
                cur = fgetc(infile);
                while (cur == '\n' || cur == '\r' || cur == ' ' || cur == '\t') {
                        cur = fgetc(infile);
                        if (cur == EOF) break;
                }
                buf[0] = cur;
                cur = fgetc(infile);
                while (cur == '\n' || cur == '\r' || cur == ' ' || cur == '\t') {
                        cur = fgetc(infile);
                        if (cur == EOF) break;
                }
                buf[1] = cur;

                if (buf[0] == EOF || buf[1] == EOF) {
                        done = 1;
                }

                byte[0] = (buf[0] == EOF) ? 0x00 : (char)buf[0];
                byte[1] = (buf[1] == EOF) ? 0x00 : (char)buf[1];

                set_byte_at(context, NULL, i, ascii2byte(byte));
                ++i;
        }

        if (fclose(infile)) {
                perror("fclose");
                return RESULT_FAILURE;
        }

        return RESULT_SUCCESS;
}

struct state *init_state(char *asm_filename, int seed) {
	struct state *ret = calloc(1, sizeof(struct state));
        int i;

        if (ret == NULL) {
                return NULL;
        }

        strcpy(ret->asm_filename, asm_filename);

        for (i = 0; i < 0x100; ++i) {
                ret->dep_table[i] = DEP_UNSET;
        }

        if (load_file(ret) == RESULT_FAILURE) {
                free(ret);
                return NULL;
        }

        ret->seed = seed;
        srand(seed);

        ret->crashed = 0;
        ret->dep_on = 0;
        ret->unlocked = 0;

        ret->reg[REG_PC] = 0x4400;

	return ret;
}

void destroy_state(struct state *context) {
	free(context);
}

void reset_state(struct state *context) {
        int i;

        context->crashed = 0;
        context->dep_on = 0;
        context->unlocked = 0;

        load_file(context);
        for (i = 0; i < 0x100; ++i) {
                context->dep_table[i] = DEP_UNSET;
        }

        for (i = 0; i < 16; ++i) {
                context->reg[i] = 0;
        }

        context->reg[REG_PC] = 0x4400;
}

/////////
// STATE MANIPULATION
/////////

void do_next_state(struct state *context, struct debug_state *dbg_context) {
        if (!context->unlocked) {
                if (get_reg(context, dbg_context, REG_PC) == 0x10) {
                        do_interrupt(context, dbg_context);
                }

                do_next_instruction(context, dbg_context);
        } else {
                printf("cannot go to next state; door has been unlocked\n");
        }
}

///////////
// MEMORY MANIPULATION
///////////

char get_byte_at(struct state *context, struct debug_state *dbg_context, word_T loc) {
        if (dbg_context != NULL) {
                check_breakpoint(dbg_context, loc);
        }
        return context->memory[(uint16_t)loc].value;
}

void set_byte_at(struct state *context, struct debug_state *dbg_context, word_T loc, char val) {
        if (dep_write_on(context, loc)) {
                if (should_write_with_check(context, dbg_context, loc)) {
                        context->memory[(uint16_t)loc].value = val;
                }
        } else {
                printf(FROM_ERROR("segfault: cannot write to execute-only page\n"));
                context->crashed = 1;
        }
}

word_T get_word_at(struct state *context, struct debug_state *dbg_context, word_T loc) {
        // little endian!
        if (dbg_context != NULL) {
                check_breakpoint(dbg_context, loc);
        }
        return ((get_byte_at(context, dbg_context, loc+1) << 8) & 0xff00) | (get_byte_at(context, dbg_context, loc) & 0xff);
}

void set_word_at(struct state *context, struct debug_state *dbg_context, word_T loc, word_T val) {
        // little endian; put least significant byte in first
        if (dep_write_on(context, loc)) {
                if (should_write_with_check(context, dbg_context, loc)) {
                        set_byte_at(context, dbg_context, loc, ((char)(val & 0xff)));
                        set_byte_at(context, dbg_context, loc + 1, (val >> 8) & 0xff);
                }
        } else {
                printf(FROM_ERROR("segfault: cannot write to execute-only page\n"));
                context->crashed = 1;
        }
}

int dep_write_on(struct state *context, word_T loc) {
        return !context->dep_on || (context->dep_table[loc / 0x100] == DEP_W || context->dep_table[loc / 0x100] == DEP_UNSET);
}

int dep_exec_on(struct state *context, word_T loc) {
        return !context->dep_on || (context->dep_table[loc / 0x100] == DEP_X || context->dep_table[loc / 0x100] == DEP_UNSET);
}

///////////
// REGISTER MANIPULATION
//////////

void set_reg(struct state *context, struct debug_state *dbg_context, int reg, word_T val) {
        if (should_write(context, dbg_context)) {
                context->reg[reg] = val;
        }
}

void delta_reg(struct state *context, struct debug_state *dbg_context, int reg, word_T delta) {
        if (should_write(context, dbg_context)) {
                context->reg[reg] += delta;
        }
}

word_T get_reg(struct state *context, struct debug_state *dbg_context, int reg) {
        /* TODO: implement register watching */
        return context->reg[reg];
}

///////////
// SR FLAGS
///////////

void set_flag(struct state *context, struct debug_state *dbg_context, word_T flag, int val) {
        word_T cur_flag = get_reg(context, dbg_context, REG_SR);
        if (val) {
                cur_flag |= flag;
        } else {
                cur_flag &= ~flag;
        }

        set_reg(context, dbg_context, REG_SR, cur_flag);
}

enum flag get_flag(struct state *context, word_T flag) {
        return !!(context->reg[REG_SR] & flag);
}
