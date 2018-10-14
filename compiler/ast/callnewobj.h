#pragma once

#include "syntax_travler.h"

class callnewobj_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_callmember &&
            ((callmember_node*)node)->ident_str() == "new";
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto call_node = (callmember_node*)node;
            auto new_node = new newobj_node(node->s_token(), node->parent);

            // ARGS
            for (auto it = call_node->begin_args(); it != call_node->end_args(); ++it)
                new_node->append(*it);

            return new_node;
        }
        return node;
    }
};