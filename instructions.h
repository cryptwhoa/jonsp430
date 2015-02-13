#ifndef JONSP430_INSTRUCTIONS_H_
#define JONSP430_INSTRUCTIONS_H_

#include "define.h"
#include "state.h"

// make sure compatible with register enumeration in state
enum instruction_operand {
        REG_PC  = 0,
        REG_SP  = 1,
        REG_SR  = 2,
        REG_CG  = 3,
        REG_R4  = 4,
        REG_R5  = 5,
        REG_R6  = 6,
        REG_R7  = 7,
        REG_R8  = 8,
        REG_R9  = 9,
        REG_R10 = 10,
        REG_R11 = 11,
        REG_R12 = 12,
        REG_R13 = 13,
        REG_R14 = 14,
        REG_R15 = 15,
};

enum operand_size {
        SIZE_BYTE = 1,
        SIZE_WORD = 2,
};

// these match up with actual values
enum address_mode {
        MODE_DIRECT = 0,             // Rn
        MODE_INDEXED = 1,            // x(Rn)
        MODE_INDIRECT = 2,           // @Rn
        MODE_INDIRECT_ADD = 3,       // @Rn+
};

/////////
// INSTRUCTION PARSING
/////////


void do_next_instruction(struct state *context, struct debug_state *dbg_context);

enum instruction_category { 
        CAT_SINGLEOP, 
        CAT_JUMP, 
        CAT_DOUBLEOP 
};

struct jumpop {
        word_T offset;
};

struct non_jumpop {
        // singleop: src/dst_* are same
        word_T src_index;
        word_T dst_index;
        
        enum operand_size op_size;
        enum instruction_operand s_reg;
        enum instruction_operand d_reg;
        enum address_mode As;
        enum address_mode Ad;
};

struct instruction;
typedef void (* instruction_operation_T) (struct state*, struct debug_state*, struct instruction*);
struct instruction {
        char *name;
        word_T address;
        word_T width;              // width of instruction in bytes
        enum instruction_category cat;

        instruction_operation_T op;

        union {
                struct jumpop jump;
                struct non_jumpop non_jump;
        } specific;
};

struct instruction parse_instruction(struct state *context, struct debug_state *dbgcontext, word_T loc);

// FORMAT I INSTRUCTIONS
void rrc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void swpb_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void rra_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void sxt_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void push_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void call_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void reti_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);

// FORM
void jnz_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jz_inst(struct state *context, struct debug_state *dbg_context, struct instruction  *inst);
void jnc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jn_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jge_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jl_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void jmp_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);

// FORMAT II INSTRUCTIONS
void mov_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void add_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void addc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void subc_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void sub_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void cmp_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void dadd_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void bit_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void bic_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void bis_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void xor_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);
void and_inst(struct state *context, struct debug_state *dbg_context, struct instruction *inst);

#endif
