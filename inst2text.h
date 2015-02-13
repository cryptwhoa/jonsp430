#ifndef JONSP430_INST2TEXT_H_
#define JONSP430_INST2TEXT_H_

#include <stdio.h>
#include <string.h>

#include "define.h"
#include "instructions.h"
#include "state.h"

#define TWO_LETTER_PAD          "        "
#define THREE_LETTER_PAD        "       "
#define FOUR_LETTER_PAD         "      "
#define FIVE_LETTER_PAD         "     "
#define SIX_LETTER_PAD          "    "

#define ANNOTATION_PAD          "       "

#define MAX_OP_LENGTH           20
#define MAX_REG_LENGTH          4
#define MAX_INSTRUCTION_LENGTH  40
#define MAX_ANNOTATION_LENGTH   200

void disassemble(struct state *context, word_T start, int num_insts, FILE *outfile);
void instruction2string(struct instruction inst, char *buf);
void annotation2string(struct state *context, struct instruction inst, char *buf, int prev_length);

#endif
