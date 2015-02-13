#ifndef JONSP430_STATE_H_
#define JONSP430_STATE_H_

#include "debug-state.h"
#include "define.h"
#include "interrupts.h"
#include "util.h"

enum dep_status { 
        DEP_X = 0,
        DEP_W = 1,
        DEP_UNSET = 2 };

struct memory_cell {
        char value;
        unsigned int breakpoint_on      : 1;
};

struct state {
        char asm_filename[MAX_FILENAME_LENGTH];

        word_T reg[16];
        struct memory_cell memory[0x10000];
        enum dep_status dep_table[0x100];

        int seed;

        unsigned int crashed           : 1;
        unsigned int unlocked          : 1;
        unsigned int dep_on            : 1;
};

enum flag {
        OFF_FLAG = 0, 
        ON_FLAG = 1,
};

// ALLOCATION

// creates a new state
// allocates space, up to caller to free memory later
struct state *init_state(char *asm_filename, int seed);
enum result load_file(struct state *context);

void reset_state(struct state *context);
void destroy_state(struct state *context);

void do_next_state(struct state *context, struct debug_state *dbg_context);

// MEMORY MANIPULATION
// byte manipulation
char get_byte_at (struct state *context, struct debug_state *dbg_context, word_T loc);
void set_byte_at (struct state *context, struct debug_state *dbg_context, word_T loc, char val);

// word manipulation
word_T get_word_at (struct state *context, struct debug_state *dbg_context, word_T loc);
void set_word_at (struct state *context, struct debug_state *dbg_context, word_T loc, word_T val);

// dep manipulation
int dep_write_on(struct state *context, word_T loc);
int dep_exec_on(struct state *context, word_T loc);

// REGISTER MANIPULATION
void set_reg(struct state *context, struct debug_state *dbg_context, int reg, word_T val);
void delta_reg(struct state *context, struct debug_state *dbg_context, int reg, word_T delta);
word_T get_reg(struct state *context, struct debug_state *dbg_context, int reg);

// SR FLAGS
void set_flag(struct state *context, struct debug_state *dbg_context, word_T flag, int val);
enum flag get_flag(struct state *context, word_T flag);

#endif
