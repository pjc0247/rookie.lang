#pragma once

#include "compilation.h"

#include "../ast/syntax_travler.h"

class syntax_validator : public syntax_traveler {
public:
    syntax_validator(compile_context &ctx) :
        ctx(ctx) {
    }

    virtual syntax_node *visit(syntax_node *node) {
        switch (node->type) {
        case syntax_type::syn_op:
            syn_op((op_node*)node);
            break;
        }
        
        return node;
    }

private:
    void syn_op(op_node *op) {
        if (is_math_op(op) &&
            (is_bool(op->left()) || is_bool(op->right())))
            ctx.push_error(syntax_error(op, L"Unexpected `bool`."));

        // Prevents useless evaluation:
        // 1 + 1 
        if (op->parent->type == syntax_type::syn_block)
            ctx.push_error(syntax_error(op, L"Unused evaluation."));
    }

    bool is_math_op(op_node *op) {
        if (op->op == L"+" || op->op == L"-" || op->op == L"*" ||
            op->op == L"/")
            return true;
        return false;
    }
    bool is_bool(syntax_node *node) {
        return node->type == syntax_type::syn_bool;
    }
    bool is_literal_true(syntax_node *node) {
        if (node->type != syntax_type::syn_bool)
            return false;
        auto b = (bool_node*)node;
        return b->value == true;
    }
    bool is_literal_false(syntax_node *node) {
        if (node->type != syntax_type::syn_bool)
            return false;
        auto b = (bool_node*)node;
        return b->value == false;
    }

private:
    compile_context &ctx;
};