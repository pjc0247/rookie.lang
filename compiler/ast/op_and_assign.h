#pragma once

#include "syntax_travler.h"

class op_and_assign_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        if (node->type == syntax_type::syn_op) {
            auto op = (op_node*)node;
            if (op->op == L"+=" ||
                op->op == L"-=" ||
                op->op == L"*=" ||
                op->op == L"/=")
                return true;
        }
        return false;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto op = (op_node*)node;
            auto new_node = new assignment_node(node->s_token());
            auto left = node->children[0];
            auto right = node->children[1];

            auto new_op = new op_node(node->s_token());
            if (op->op == L"+=")
                new_op->op = L"+";
            else if (op->op == L"-=")
                new_op->op = L"-";
            else if (op->op == L"*=")
                new_op->op = L"*";
            else if (op->op == L"/=")
                new_op->op = L"/";

            new_op->append(left);
            new_op->append(right);

            new_node->append(left);
            new_node->append(new_op);

            return new_node;
        }

        return node;
    }
};