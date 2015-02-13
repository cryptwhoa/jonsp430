#ifndef JONSP430_INTERRUPTS_H_
#define JONSP430_INTERRUPTS_H_

#include "define.h"
#include "state.h"

typedef enum result (* interrupt_T) (struct state*, struct debug_state*, word_T*);

enum result do_interrupt(struct state *context, struct debug_state *dbg_context);

enum result do_nothing(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_putchar(struct state *context, struct debug_state *dbg_context,  word_T *args);
enum result int_getchar(struct state *context, struct debug_state *dbg_context,  word_T *args);
enum result int_gets(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_dep_on(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_mark_page(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_random(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_hsm_1_test(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_hsm_2_test(struct state *context, struct debug_state *dbg_context, word_T *args);
enum result int_unlock(struct state *context, struct debug_state *dbg_context, word_T *args);

#endif
