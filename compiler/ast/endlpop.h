#pragma once

#include "syntax_travler.h"

// Inserts pop_node if required.
// 
//    example)
//      - 1 + 2;
//          to
//      - pop(1 + 2);
class endlpop_transformer : public syntax_traveler {
protected:
    virtual int transform(root_node *root) {
        for (auto klass : root->children) {
            if (klass->type != syntax_type::syn_class)
                continue;

            for (auto method : klass->children) {
                if (method->type != syntax_type::syn_method)
                    continue;

                auto mn = (method_node*)method;

                depth = 0;
                _visit(mn->body());
            }
        }

        return changes;
    }
    virtual syntax_node *visit(syntax_node *node) {
        printf("%s, %d\n", typeid(*node).name(), depth);

        if (node->type == syntax_type::syn_ident ||
            node->type == syntax_type::syn_literal)
            depth++;
        else if (
            node->type == syntax_type::syn_call ||
            node->type == syntax_type::syn_callmember) {

            depth -= node->children.size();
        }
        else if (node->type == syntax_type::syn_newarr) {
            depth -= node->children.size();
        }
        else if (
            node->type == syntax_type::syn_memberaccess ||
            node->type == syntax_type::syn_assignment ||
            node->type == syntax_type::syn_op) {
            depth -= 2;
        }
        else if (node->type == syntax_type::syn_return)
            depth --;

        if (node->type == syntax_type::syn_endl) {
            if (depth >= 1) {
                auto pop = new pop_node(node->s_token(), node->parent);
                node = pop;
            }
            depth = 0;
        }

        return node;
    }

private:
    int depth = 0;
};