#include "stdafx.h"
#include "syntax.h"

syntax_node::syntax_node(const stoken &token) :
    source(token),
    type(syntax_type::syn_none),
    capacity(-1), nth_block_or_single(-1),
    is_virtual(false) {

}
bool syntax_node::is_complete() const {
    if (capacity > 0)
        return children.size() >= capacity;

    return false;
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

void syntax_node::remove(syntax_node *node) {
    children.erase(
        std::find(children.begin(), children.end(),
            node));
}
syntax_node *syntax_node::append(syntax_node *node, bool fire_oncomplete) {
    if (nth_block_or_single >= 0 &&
        children.size() == nth_block_or_single &&
        node->type != syntax_type::syn_block) {

        auto block = new block_node(node->s_token());
        block->append(node);
        node = block;

        block->capacity = 2;
    }

    // 'endl' only can be accepted in block_node.
    if (node->type == syntax_type::syn_endl) {
        if (type != syntax_type::syn_block)
            return this;
    }

    node->parent = this;
    children.push_back(node);
    if (root != nullptr) {
        node->root = root;
        root->add_reference(node);
    }

    if (fire_oncomplete && is_complete()) {
        on_complete();
        return nearest_incomplete_node();
    }
    return this;
}
syntax_node *syntax_node::pop() {
    if (children.size() == 0) return nullptr;

    auto last = children[children.size() - 1];
    children.pop_back();
    return last;
}

syntax_node *syntax_node::find_upward_until(syntax_type type) const {
    syntax_node *current = parent;
    while (current != nullptr) {
        if (current->type == type)
            break;
        current = current->parent;
    }
    return current;
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
        else if (ln->literal_type == literal_type::decimal)
            printf("literal (%f)\n", ((literal_node*)this)->decimal);
        else
            wprintf(L"literal (%s)\n", ((literal_node*)this)->str.c_str());
    }
    else
        printf("%s\n", typeid(*this).name());

    for (auto child : children)
        child->dump(depth + 1);
}


void block_node::push_local(const std::wstring &str) {
    if (locals.empty() ||
        std::find(locals.begin(), locals.end(), str) == locals.end()) {

        declaring_method()->local_size++;
        locals.push_back(str);
    }
}