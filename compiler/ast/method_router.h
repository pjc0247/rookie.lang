#pragma once

#include "syntax_travler.h"

class method_router_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_class;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            
        }
        return node;
    }
};