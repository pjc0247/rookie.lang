#pragma once

#include <list>
#include <vector>

#include "program.h"

class dup_transformer {
public:
    dup_transformer(const std::vector<instruction> &src) {
        instructions = src;
    }

    bool is_transformable(std::vector<instruction>::iterator &it) {
        if ((*it).opcode == opcode::op_stloc &&
            (*std::next(it)).opcode == opcode::op_ldloc &&
            (*it).operand == (*std::next(it)).operand) {
            return true;
        }
        return false;
    }

    std::vector<instruction> &transform() {
        int c = 0;
        for (auto it = instructions.begin(); it != std::prev(instructions.end()); ++it) {
            if (is_transformable(it)) {
                int operand = (*it).operand;
                *it = instruction(opcode::op_dup, 0);
                *std::next(it) = instruction(opcode::op_stloc, operand);
            }
            c++;
        }

        return instructions;
    }

private:
    int cursor;

    std::vector<instruction>::iterator it;
    std::vector<instruction> instructions;
};