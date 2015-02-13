#ifndef JONSP430_CLI_H_
#define JONSP430_CLI_H_

#include "define.h"
#include "state.h"
#include "debug-state.h"
#include "util.h"

struct command_instance;

// starts command line interface
// void cli_start(struct state *context, struct debug_state *dbg_context);
void cli_start(struct state *context);

// parses command
struct command_instance cli_parse(char *buffer);
void cli_do_next(struct state *context);

#endif
