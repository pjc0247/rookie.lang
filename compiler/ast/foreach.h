#pragma once

#include "syntax_travler.h"

class foreach_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_for  &&
            node->children[0]->type == syntax_type::syn_ident;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto left = (ident_node*)node->children[0];
            auto right = (call_node*)node->children[1];
            auto block = (call_node*)node->children[2];

            auto new_node = new foreach_node(node->s_token());
            new_node->append(left);
            new_node->append(right);
            new_node->append(block);

            return new_node;
        }
        return node;
    }
};