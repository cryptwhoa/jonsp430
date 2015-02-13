#ifndef JONSP430_DEBUG_STATE_H
#define JONSP430_DEBUG_STATE_H

#include <stdio.h>

#include "define.h"
#include "inst2text.h"
#include "state.h"

#define INSTRUCTION_HISTORY_LENGTH      5

struct debug_state {
        word_T breakpoint;
        int start_triggered;
        int stop_triggered;
        int on;
        // char inst_history[5][MAX_INSTRUCTION_LENGTH];
        FILE *logfile;
};

struct debug_state *init_debug_state();
void destroy_debug_state(struct debug_state *dbg_context);

//////
// BREAKPOINTS
//////

void add_breakpoint(struct debug_state *dbg_context, word_T val);
void remove_breakpoint(struct debug_state *dbg_context);
void check_breakpoint(struct debug_state *dbg_context, word_T val);

int stop_triggered(struct debug_state *dbg_context);

/* tells debug context that we'd like to proceed
 * keeps us from breaking on the same instruction
 */
void trigger_start(struct debug_state *dbg_context);
void untrigger_start(struct debug_state *dbg_context);

/* intentionally no trigger_stop, since it may be triggered in multiple ways
 * (memory, register, etc.)
 */
void untrigger_stop(struct debug_state *dbg_context);

int should_write(struct state *context, struct debug_state *dbg_context);
int should_write_with_check(struct state *context, struct debug_state *dbg_context, word_T loc);

/////
// INSTRUCTION HISTORY
/////

#if 0
void push_to_inst_history(struct debug_state *dbg_context, struct instruction) {
        for (int i=1; i < INSTRUCTION_HISTORY_LENGTH; ++i) {
                memcpy(dbg_context->inst_hist[i-1], dbg_context->inst_hist[i], [INSTRUCTION_HISTORY_LENGTH]);
        }

        disassemble(context, get_reg(context, dbg_context, PC), 1, stdout);

}

void display_inst_history(struct instruction) {

}
#endif
#endif
