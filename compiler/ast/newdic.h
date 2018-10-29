#pragma once

#include "syntax_travler.h"

class newdic_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        auto parent = node->parent;
        return node->type == syntax_type::syn_block &&
            (parent->type == syntax_type::syn_assignment ||
             parent->type == syntax_type::syn_call ||
             parent->type == syntax_type::syn_callmember ||
             parent->type == syntax_type::syn_newdic);
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto new_node = new newdic_node(node->s_token());
             
            // ARGS
            for (auto &c : node->children)
                new_node->append(c);

            return new_node;
        }
        return node;
    }
};