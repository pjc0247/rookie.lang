#pragma once

#include <map>

#include "compilation.h"

#include "../ast/syntax_travler.h"

// Prevents useless evaluation:
//   1 + 1 
class unused_validator : public syntax_traveler {
public:
    virtual syntax_node *visit(syntax_node *node) {
        switch (node->type) {
        case syntax_type::syn_op:
        case syntax_type::syn_newarr:
        case syntax_type::syn_newdic:
            if (is_contains_assignment(node))
                break;
            
            if (node->parent->type == syntax_type::syn_block)
                ctx->push_error(syntax_error(node, L"Unused evaluation."));
            break;
        }

        return node;
    }

private:
    bool is_contains_assignment(syntax_node *node) {
        if (node->type == syntax_type::syn_block) {
            auto op = (op_node*)node;
            if (op->op == L"+=" || op->op == L"-=" ||
                op->op == L"*=" || op->op == L"/=")
                return true;
        }
        return false;
    }
};