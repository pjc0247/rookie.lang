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
    virtual syntax_node *visit(syntax_node *node) {
        if (node->type == syntax_type::syn_block) {
            depth = 0;

            for (auto it = node->children.begin();it!=node->children.end();++it) {
                auto child = *it;

                if (child->type == syntax_type::syn_endl ||
                    child->type == syntax_type::syn_comma) {
                    auto prev = *std::prev(it);

                    if (prev->type == syntax_type::syn_call ||
                        prev->type == syntax_type::syn_callmember ||
                        prev->type == syntax_type::syn_newobj) {

                        auto pop = new pop_node(node->s_token());
                        pop->parent = node;
                        *it = pop;
                    }
                }
            }
        }

        /*
        else if (node->type == syntax_type::syn_ident ||
            node->type == syntax_type::syn_literal)
            depth++;
        else if (
            node->type == syntax_type::syn_call ||
            node->type == syntax_type::syn_callmember) {

            depth -= node->children.size();
            depth++;
        }
        else if (node->type == syntax_type::syn_newarr ||
            node->type == syntax_type::syn_newdic) {

            depth -= node->children.size();
        }
        else if (
            node->type == syntax_type::syn_assignment ||
            node->type == syntax_type::syn_op) {
            depth -= 2;
        }
        else if (
            node->type == syntax_type::syn_memberaccess ||
            node->type == syntax_type::syn_return)
            depth --;

        if (node->type == syntax_type::syn_endl) {
            if (depth >= 1) {
                auto pop = new pop_node(node->s_token());
                node = pop;
            }
            else
                node = nullptr;

            depth = 0;
        }
        */

        return node;
    }

private:
    int depth = 0;
};