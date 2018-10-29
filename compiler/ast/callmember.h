#pragma once

#include "syntax_travler.h"

/*
&&
(node->children[0]->type == syntax_type::syn_ident ||
node->children[0]->type == syntax_type::syn_this)
*/
class callmember_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_memberaccess  &&
            node->children[1]->type == syntax_type::syn_call;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto left = (ident_node*)node->children[0];
            auto right = (call_node*)node->children[1];

            auto new_node = new callmember_node(right->s_token());
            // METHOD_NAME
            new_node->append(right->ident());
            // .this
            new_node->append(left);

            // ARGS
            for (auto it = right->begin_args(); it != right->end_args(); ++it)
                new_node->append(*it);

            return new_node;
        }
        return node;
    }
};