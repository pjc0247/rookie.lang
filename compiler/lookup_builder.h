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
            auto id = ((ident_node*)node)->ident;

            if (id.size() >= 1 && id[0] == '@')
                sp.get_ptr(id.substr(1));
            else
                sp.get_ptr(id);
        }

        return node;
    }

private:
    string_pool sp;
};