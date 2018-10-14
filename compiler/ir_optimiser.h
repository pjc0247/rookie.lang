#pragma once

#include <list>

#include "program.h"

class ir_optimiser {
public:

	bool is_transformable() {
		if ((*it).opcode == opcode::op_stloc &&
			(*std::next(it)).opcode == opcode::op_ldloc &&
			(*it).operand == (*std::next(it)).operand) {
			return true;
		}
		return false;
	}

private:
	int cursor;

	std::list<instruction>::iterator it;
	std::list<instruction> instructions;
};