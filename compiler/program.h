#pragma once

typedef enum opcode : unsigned char {
	op_nop = 0,

	op_add,
	op_sub,
	op_mul,
	op_div,
	op_g, op_l, op_ge, op_le,

	op_pop,
	op_ldloc, op_stloc,
	op_ldstate, op_ststate,

	op_ldi,
	op_ldstr, op_ldnull,

	op_jmp_true, op_jmp_false,

	op_ret
} opcode_t;

#pragma pack (push, 1)
struct instruction {
	unsigned char opcode;
	int operand;

	instruction(opcode_t _o, int operand) :
		opcode((unsigned char)_o), operand(operand) {
	}
};
#pragma pack (pop)

#pragma pack (push, 1)
struct program_header {
	unsigned int code_len;
	unsigned int rdata_len;
};
struct program {
	program_header header;

	const instruction *code;
	const char *rdata;
};
#pragma pack (pop)