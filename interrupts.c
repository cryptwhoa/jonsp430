#include <stdio.h>
#include <stdlib.h>

#include "define.h"
#include "interrupts.h"
#include "state.h"
#include "util.h"

// returns positive value on success, 0 on failure
enum result do_interrupt(struct state *context, struct debug_state *dbg_context) {
        // at most two args
        // word_T interrupt = get_word_at(context, NULL, get_reg(context, NULL, SP) + 6);
        char interrupt = (get_reg(context, NULL, REG_SR) >> 8) & 0x7f;
        word_T args[3];

        interrupt_T op = do_nothing;
        switch(interrupt) {
        case 0x00: {
                args[0] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 8);

                op = int_putchar;
                break; }
        case 0x01: {
                // no args
                op = int_getchar;
                break; }
        case 0x02: {
                args[0] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 8);
                args[1] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 10);

                op = int_gets;
                break; }
        case 0x10: {
                // no args
                op = int_dep_on;
                break; }
        case 0x11: {
                args[0] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 8);
                args[1] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 10);

                op = int_mark_page;
                break; }
        case 0x20: {
                // no args
                op = int_random;
                break; }
        case 0x7D: {
                args[0] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 8);
                args[1] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 10);
                args[2] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 12);

                op = int_hsm_1_test;
                break; }
        case 0x7E: {
                args[0] = get_word_at(context, NULL, get_reg(context, NULL, REG_SP) + 8);

                op = int_hsm_2_test;
                break; }
        case 0x7F: {
                // no args
                op = int_unlock;
                break; }
        default: 
                ; /* shut up compiler */
        }

        if (op != do_nothing && dbg_context->logfile) {
                fprintf(dbg_context->logfile, ":: INTERRUPT 0x%02x ", interrupt);
        }

        return op (context, dbg_context, args);
}

enum result do_nothing(struct state *context, struct debug_state *dbg_context, word_T *args) { return RESULT_SUCCESS; }

// arg0: character to print
enum result int_putchar(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(putchar)  ||  ARG[0] 0x%04x (char)\n", args[0] & 0xffff);
        }
        printf(FROM_CONSOLE("%c"), (char) (args[0] & 0xff));
        return RESULT_SUCCESS;
}

// no args
enum result int_getchar(struct state *context, struct debug_state *dbg_context, word_T *args) { return RESULT_FAILURE; }

// arg0: address to put string
// arg1: max bytes to read
enum result int_gets(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        // TODO: gets to DEP controlled memory
        int i;
        enum result ret = RESULT_FAILURE;
        word_T addr = args[0];
        word_T nbytes = args[1];

        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(gets)  ||  ARG[0] 0x%04x (addr)  ||  ARG[1] 0x%04x (nbytes)\n", addr & 0xffff, nbytes & 0xffff);
        }

        // get twice numbytes since entered as ascii
        // last byte is always null, so we subtract 2
        // add 2 back for newline and null terminator
        int buflength = 2*nbytes;

        char *buf = malloc(buflength);

        printf("\n==============\n");
        printf("enter input:  ");

        fgets(buf, buflength, stdin);

        for (i=0; i < buflength && buf[i] != '\n'; ++i) ;

        if (!i) {
                printf("continue or go to next state when ready to enter input\n");
        } else if (i % 2 == 1) {
                printf("enter even number of hex digits\n");
        } else {
                if (dbg_context->logfile) {
                        fprintf(dbg_context->logfile, ":: RECEIVED INPUT STRING __");
                }

                ret = RESULT_SUCCESS;
                char byte[2];
                for (i=0; i < buflength - 2 && buf[i] != '\00' && buf[i] != '\n'; i += 2) {
                        if (dbg_context->logfile) {
                                fprintf(dbg_context->logfile, "%c%c", buf[i], buf[i+1]);
                        }
                        byte[0] = buf[i];
                        byte[1] = buf[i+1];

                        set_byte_at(context, NULL, addr + i/2, ascii2byte(byte));
                }
                byte[0] = '\x00';
                byte[1] = '\x00';
                set_byte_at(context, NULL, addr + i/2, ascii2byte(byte));
                
                if (dbg_context->logfile) {
                        fprintf(dbg_context->logfile, "__\n");
                }
        }

        free (buf);
        return ret;
}

// no args
enum result int_dep_on(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(dep on)\n");
        }
        context->dep_on = 1;
        return RESULT_SUCCESS; 
}

// arg0: page number
// arg1: 1 if writable, 0 if executable
enum result int_mark_page(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        // maybe !!(args[1])?
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(mark page)  ||  ARG[0] 0x%04x (page)  ||  ARG[1] 0x%04x (1 = W, 0 = X)\n", args[0] & 0xffff, args[1] & 0xffff);
        }
        context->dep_table[(args[0]) & 0xff] = args[1] & 1;
        
        return RESULT_SUCCESS; 
}

// no args
enum result int_random(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(rand)\n");
        }
        set_reg(context, NULL, 15, random() & 0xffff);
        return RESULT_FAILURE; 
}

// arg0: password to test
// arg1: location of flag
enum result int_hsm_1_test(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(hsm 1 test)  ||  ");
        }

        set_word_at(context, NULL, args[1], 0);
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "ARG[0] 0x%04x (address)\n", args[1] & 0xffff);
        }

        return RESULT_SUCCESS;
}

// arg0: password to test
enum result int_hsm_2_test(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(hsm 2 test)  ||  ARG[0] 0x%04x (password addr)  ||  MEMVAL 0x%04x\n", args[0] & 0xffff, get_reg(context, NULL, REG_SP) + 12);
        }
        set_byte_at(context, NULL, get_reg(context, NULL, REG_SP) + 12, 0);

        return RESULT_SUCCESS; 
}

// no args
enum result int_unlock(struct state *context, struct debug_state *dbg_context, word_T *args) { 
        if (dbg_context->logfile) {
                fprintf(dbg_context->logfile, "(unlock)\n");
        }
        printf(FROM_ERROR("unlocked!\n"));
        context->unlocked = 1;
        return RESULT_SUCCESS;
}
