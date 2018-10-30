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
            auto new_node = new foreach_node(node->s_token());
            for (auto child : node->children)
                new_node->append(child);

            return new_node;
        }
        return node;
    }
};