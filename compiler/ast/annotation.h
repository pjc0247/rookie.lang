#pragma once

#include "syntax_travler.h"

class annotation_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_annotation &&
            node->parent->type != syntax_type::syn_method;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            bool found_me = false;
            for (auto c : node->parent->children) {
                if (c == node)
                    found_me = true;

                if (found_me &&
                    c->type == syntax_type::syn_method) {
                    auto method = (method_node*)c;
                    method->push_annotation((annotation_node*)node);

                    return nullptr;
                }
            }
            return nullptr;
        }
        
        return node;
    }
};