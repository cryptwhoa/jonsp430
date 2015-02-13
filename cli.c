#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "debug-state.h"
#include "define.h"
#include "inst2text.h"
#include "state.h"
#include "util.h"

#define MAX_COMMAND_TOKENS 10

const int MAX_COMMAND_LENGTH = 512;
const char* PROMPT = "(jonsp430) ";

typedef void (* cli_operation_T) (struct state*, struct debug_state*, char **);
struct command_type {
        char *name;
        cli_operation_T op;
};

struct command_instance {
        struct command_type type;
        char *args[MAX_COMMAND_TOKENS];
};

// dummy placeholder
void cli_quit(struct state *context, struct debug_state *dbg_state, char **args) { }

void cli_start(struct state *context) {
        struct debug_state *dbg_context = init_debug_state();

	printf ("===================================\n");
	printf ("=            jonsp430             =\n");
	printf ("=                                 =\n");
	printf ("= custom msp430 emulator/debugger =\n");
	printf ("===================================\n");
	printf ("\n");
        printf ("legend: %s, %s, %s\n", "debugger text", FROM_CONSOLE("device stdout"), FROM_ERROR("device stderr"));
        printf ("the random seed for this session is: %d\n", context->seed);
        printf ("\n");

	char line[MAX_COMMAND_LENGTH];
	int keep_going = 1;
	while (keep_going) {
		printf ("%s", PROMPT);
                struct command_instance command;

		if (!fgets (line, MAX_COMMAND_LENGTH, stdin)) {
                        command.type.op = cli_quit;
		} else {
			command = cli_parse(line);
		}

                if (command.type.op == cli_quit) {
                        keep_going = 0;
                        printf("\n");
                } else {
                        command.type.op(context, dbg_context, command.args);
                }

                if (context == NULL) {
                        keep_going = 0;
                        printf("\n");
                }
	}

        destroy_debug_state(dbg_context);
}

// no args
void cli_unrecognized(struct state *context, struct debug_state *dbg_state, char **args) {
        printf ("unrecognized instruction: %s\n", args[0]);
}

// no args
void cli_ambiguous(struct state *context, struct debug_state *dbg_state, char **args) {
        printf ("ambiguous instruction: %s\n", args[0]);
}

// no args
void cli_none (struct state *context, struct debug_state *dbg_state, char **args) { }

//////
// NO MODIFY STATE
//////
// [*] arg 1: address to read
//     arg 2: number of bytes to read
void cli_read(struct state *context, struct debug_state *dbg_context, char **args) {
        int start, nbytes, i;

        if (args[1] == NULL) {
                printf ("insufficient arguments for read command\n");
        } else {
                start = strtol(args[1], NULL, 0);
        }

        if (args[2] == NULL) {
                nbytes = 16;
        } else {
                nbytes = strtol(args[2], NULL, 0);
        }

        if (start + nbytes > 0x10000) {
                printf ("command reads out-of-bounds memory\n");
        } else {
                for (i=0; i < nbytes; i += 2) {
                        if (i % 16 == 0) {
                                if (i) {
                                        printf("\n");
                                }
                                printf ("0x%04x:  ", start + i);
                        }

                        char byteval[2];
                        byte2ascii(get_byte_at(context, NULL, start+i), byteval);
                        printf ("%c%c", byteval[0], byteval[1]);
                        byte2ascii(get_byte_at(context, NULL, start+i+1), byteval);
                        printf("%c%c ", byteval[0], byteval[1]);
                }

                printf ("\n");
        }
}

// no args
void cli_regs(struct state *context, struct debug_state *dbg_context, char **args) {
        int i, j;

        // printf changes x to unsigned int; we only want the last 4 hex digits
        printf ("pc  %04x  ", context->reg[REG_PC] & 0x0000ffff);
        printf ("sp  %04x  ", context->reg[REG_SP] & 0x0000ffff);
        printf ("sr  %04x  ", context->reg[REG_SR] & 0x0000ffff);
        printf ("cg  %04x\n", context->reg[REG_CG] & 0x0000ffff);

        for (i=1; i < 4; ++i) {
                for (j=0; j < 4; ++j) {
                        printf ("r%02d %04x  ", 4*i + j, context->reg[4*i + j] & 0x0000ffff);
                }
                printf ("\n");
        }
}

// [*] arg1: file to disassemble to
void cli_disassemble(struct state *context, struct debug_state *dbg_context, char **args) {
        word_T start = 0x4400;
        int num_insts = 5;
        FILE *outfile = stdout;

        if (args[1]) {
                start = strtol(args[1], NULL, 0) & 0xffff;
        }

        if (args[2]) {
                int tmp_num_insts = strtol(args[2], NULL, 0);
                if (tmp_num_insts > 0) {
                        num_insts = tmp_num_insts;
                }
        }

        // if (args[3]) {
                // TODO: disassemble to file

        // }

        disassemble(context, start, num_insts, outfile);
}

void cli_log(struct state *context, struct debug_state *dbg_context, char **args) {
        int fd;
        FILE *fp = NULL;

        if (args[1]) {
                fd = open(args[1], O_WRONLY | O_CREAT | O_EXCL);
                if (fd == -1) {
                        perror("open");
                        // TODO: return fail
                }

                fp = fdopen(fd, "w");
                if (fp == NULL) {
                        perror("fopen");
                        // TODO: return fail
                }

                dbg_context->logfile = fp;
        } else {
                dbg_context->logfile = stdout;
        }

        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "random seed: %d\n", context->seed);
        }
}

void cli_unlog(struct state *context, struct debug_state *dbg_context, char **args) {
        fclose(dbg_context->logfile);
        dbg_context->logfile = NULL;
}

///////
// MODIFY STATE
///////

// no args
void cli_next (struct state *context, struct debug_state *dbg_context, char **args) {
        trigger_start(dbg_context);
        untrigger_stop(dbg_context);
        do_next_state(context, dbg_context);
        untrigger_start(dbg_context);
        cli_regs(context, dbg_context, NULL);
        disassemble(context, get_reg(context, dbg_context, REG_PC), 1, stdout);
}

// no args
void cli_continue (struct state *context, struct debug_state *dbg_context, char **args) {
        trigger_start(dbg_context);
        untrigger_stop(dbg_context);
        while (!(get_flag(context, FLAG_CPUOFF) || stop_triggered(dbg_context) || context->unlocked || context->crashed)) {
                do_next_state(context, dbg_context);
                untrigger_start(dbg_context);
        }

        cli_regs(context, dbg_context, NULL);
        disassemble(context, get_reg(context, dbg_context, REG_PC), 1, stdout);
}

// no args
void cli_reset(struct state *context, struct debug_state *dbg_context, char **args) {
        reset_state(context);
}

//////
// MODIFY DEBUG STATE
/////

// [*] arg1: address to set breakpoint at
void cli_break (struct state *context, struct debug_state *dbg_context, char **args) {
        if (args[1] == NULL) {
                printf ("break requires an argument\n");
        } else {
                word_T loc = strtol(args[1], NULL, 0) & 0xffff;
                add_breakpoint(dbg_context, loc);
                printf ("breakpoint set at 0x%04x\n", loc & 0xffff);
        }
}

// [*] arg1: address to unset breakpoint at
void cli_unbreak (struct state *context, struct debug_state *dbg_context, char **args) {
        printf ("NOT IMPLEMENTED YET\n");
}

struct command_type command_function_table[] = {
        {"break",               cli_break},
        {"continue",            cli_continue},
        {"disassemble",         cli_disassemble},
        {"log",                 cli_log},
        {"next",                cli_next},
        {"quit",                cli_quit},
        {"read",                cli_read},
        {"registers",           cli_regs},
        {"reset",               cli_reset},
        {"unbreak",             cli_unbreak},
        {"unlog",               cli_unlog},
};

struct command_instance cli_parse (char *line) {

        struct command_instance ret;
        struct command_type ret_type;

        int i = 0;
        int num_toks = 0;
        int num_matches = 0;

        for (i=0; i < MAX_COMMAND_TOKENS; ++i) {
                ret.args[i] = NULL;
        }

        if (line[0] == '\n' || line[0] == '\r') {
                ret_type.name = "none";
                ret_type.op = cli_none;
                ret.type = ret_type;
                return ret;
        }

        char *tok = strtok(line, " \t\r\n");
        while (num_toks < MAX_COMMAND_TOKENS && tok != NULL) {
                ret.args[num_toks] = tok;
                ++num_toks;
                tok = strtok(NULL, " \t\r\n");
        }

        if (ret.args[0][0] == '\00') {
                ret_type.name = "none";
                ret_type.op = cli_none;
                ret.type = ret_type;
                return ret;
        }

        for (i=0; i < sizeof(command_function_table) / sizeof(struct command_type);  ++i) {
                if (strstr(command_function_table[i].name, ret.args[0]) == command_function_table[i].name) {
                        ++num_matches;
                        ret_type.name = command_function_table[i].name;
                        ret_type.op = command_function_table[i].op;
                }
        }

        if (num_matches == 0) {
                ret_type.name = "unrecognized";
                ret_type.op = cli_unrecognized;
        } else if (num_matches > 1) {
                ret_type.name = "ambiguous";
                ret_type.op = cli_ambiguous;
        } 

        ret.type = ret_type;
        return ret;
}
