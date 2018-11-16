#pragma once

#include "compilation.h"

#include "../ast/syntax_travler.h"

class syntax_validator : public syntax_traveler {
public:
    virtual syntax_node *visit(syntax_node *node) {
        switch (node->type) {
        case syntax_type::syn_class:
            syn_class((class_node*)node);
            break;
        case syntax_type::syn_method:
            syn_method((method_node*)node);
            break;
        case syntax_type::syn_ident:
            syn_ident((ident_node*)node);
            break;
        case syntax_type::syn_op:
            syn_op((op_node*)node);
            break;
        case syntax_type::syn_assignment:
            syn_assignment((assignment_node*)node);
            break;
        }
        
        return node;
    }

private:
    void syn_class(class_node *klass) {
        for (int i = 1; i < klass->children.size();i++) {
            auto child = klass->children[i];

            if (child->type == syntax_type::syn_ident) {
                ctx->push_error(syntax_error(child, L"Unexpectd identifier."));
            }
        }
    }
    void syn_method(method_node *method) {
        if (method->children[1]->type != syntax_type::syn_params) {
            ctx->push_error(syntax_error(method, L"Wrong method definition."));
        }
        else {
            auto params = method->children[1]->children;

            for (int i=0;i<params.size();i++) {
                if (params[i]->type == syntax_type::syn_ident) {
                    auto id = (ident_node*)params[i];
                    if (id->ident[0] == L'*' && i != params.size()-1)
                        ctx->push_error(syntax_error(method, L"Asterisk(`*`) param must be positioned at last."));
                }
            }
        }
    }
    void syn_ident(ident_node *id) {
        auto method = id->declaring_method();
        if (method && method->attr & method_attr::method_static) {
            if (id->ident[0] == L'@') {
                ctx->push_error(syntax_error(method, L"`@` is not allowed in static method."));
            }
        }
    }
    void syn_op(op_node *op) {
        if (is_math_op(op) &&
            (is_bool(op->left()) || is_bool(op->right())))
            ctx->push_error(syntax_error(op, L"Unexpected `bool`."));

        // Prevents useless evaluation:
        // 1 + 1 
        if (op->parent->type == syntax_type::syn_block) {
            if (op->op == L"+=" || op->op == L"-=" ||
                op->op == L"*=" || op->op == L"/=")
                return;
            ctx->push_error(syntax_error(op, L"Unused evaluation."));
        }
    }
    void syn_assignment(assignment_node *node) {
        if (node->parent->type != syntax_type::syn_block &&
            node->parent->type != syntax_type::syn_class &&
			node->parent->type != syntax_type::syn_params &&
            node->parent->type != syntax_type::syn_for)
            ctx->push_error(syntax_error(node, L"Invalid operation"));
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
};