#pragma once

#include <string>
#include <vector>

#include "string_pool.h"

#include "ast/syntax_travler.h"

class lookup_builder : public syntax_traveler {
public:
    string_pool build(syntax_node *root) {
        _visit(root);

        return sp;
    }

protected:
    virtual syntax_node *visit(syntax_node *node) {
        if (node->type == syntax_type::syn_ident) {
            sp.get_ptr(((ident_node*)node)->ident);
        }

        return node;
    }

private:
    string_pool sp;
};