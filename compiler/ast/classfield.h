#pragma once

#include "syntax_travler.h"

class classfield_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_assignment &&
            node->parent->type == syntax_type::syn_class;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto new_node = new field_node(node->s_token(), node->parent);

            new_node->append(node->children[0]);
            new_node->append(node->children[1]);

            node->declaring_class()->fields.push_back(new_node);

            return new_node;
        }
        return node;
    }
};