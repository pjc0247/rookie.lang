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

void syntax_node::dump(int depth) {
    for (int i = 0; i < depth * 2; i++)
        putchar(' ');

    if (type == syntax_type::syn_ident)
        wprintf(L"ident (%s)\n", ((ident_node*)this)->ident.c_str());
    else if (type == syntax_type::syn_literal) {
        auto ln = (literal_node*)this;
        if (ln->literal_type == literal_type::integer)
            printf("literal (%d)\n", ((literal_node*)this)->integer);
        else
            wprintf(L"literal (%s)\n", ((literal_node*)this)->str.c_str());
    }
    else
        printf("%s\n", typeid(*this).name());

    for (auto child : children)
        child->dump(depth + 1);
}