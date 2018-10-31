#pragma once

#include "syntax_travler.h"

class ifelse_chain_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_block;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {

            for (int i = 0; i < node->children.size(); i++) {
                auto child = node->children[i];

                if (child->type == syntax_type::syn_if) {
                    for (int j = i + 1; j < node->children.size(); j++) {
                        auto child2 = node->children[j];

                        if (child2->type == syntax_type::syn_else) {
                            child->append(child2);

                            node->remove(child2);
                        }
                    }
                }
            }
        }
        return node;
    }
};