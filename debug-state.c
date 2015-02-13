#include <stdio.h>
#include <stdlib.h>

#include "debug-state.h"
#include "define.h"

struct debug_state *init_debug_state() {
        // struct debug_state *ret = calloc(1, sizeof(struct debug_state));
        struct debug_state *ret = malloc(sizeof(struct debug_state));
        memset(ret, 0, sizeof(struct debug_state));
        return ret;
}

void destroy_debug_state(struct debug_state *dbg_context) {
        if (dbg_context->logfile) {
                fclose(dbg_context->logfile);
        }

        free (dbg_context);
}

void add_breakpoint(struct debug_state *dbg_context, word_T val) {
        dbg_context->breakpoint = val;
        dbg_context->on = 1;
}

void remove_breakpoint(struct debug_state *dbg_context) {
        dbg_context->on = 0;
}

void check_breakpoint(struct debug_state *dbg_context, word_T val) {
        if (dbg_context) {
                if (dbg_context->on && !dbg_context->start_triggered && !dbg_context->stop_triggered && (dbg_context->breakpoint == val)) {
                        printf ("breakpoint reached\n");
                        dbg_context->stop_triggered = 1;
                }
        }
}

void trigger_start(struct debug_state *dbg_context) {
        dbg_context->start_triggered = 1;
}

void untrigger_start(struct debug_state *dbg_context) {
        dbg_context->start_triggered = 0;
}

void untrigger_stop(struct debug_state *dbg_context) {
        dbg_context->stop_triggered = 0;
}

int stop_triggered(struct debug_state *dbg_context) {
        if (dbg_context == NULL) {
                return 0;
        } else {
                return dbg_context->stop_triggered;
        }
}

int should_write(struct state *context, struct debug_state *dbg_context) {
        return !context->crashed && !stop_triggered(dbg_context);
}

int should_write_with_check(struct state *context, struct debug_state *dbg_context, word_T loc) {
        check_breakpoint(dbg_context, loc);
        return !context->crashed && !stop_triggered(dbg_context);
}
