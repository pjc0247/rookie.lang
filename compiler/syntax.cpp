#include "stdafx.h"
#include "syntax.h"

bool syntax_node::is_complete() const {
    return capacity > 0 ? children.size() >= capacity : false;
}
syntax_node *syntax_node::nearest_incomplete_node() {
    syntax_node *current = parent;
    if (parent == nullptr)
        return this;

    while (true) {
        if (current->type == syntax_type::syn_root)
            return current;

        if (current->is_complete())
            current = current->parent;
        else
            return current;
    }
}

compile_context &syntax_node::ctx() const {
    return root()->ctx;
}