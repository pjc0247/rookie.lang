#include "stdafx.h"

#include "syntax_node.h"

// ROOT_NODE
root_node::root_node() :
	syntax_node(stoken(::token())) {

	root = this;
	type = syntax_type::syn_root;
}
root_node::~root_node() {
	for (auto child : flatten_children)
		delete child;
}

void root_node::add_reference(syntax_node *node) {
#ifndef __EMSCRIPTEN__
	flatten_children.insert(node);
#endif
}

// BLOCK_NODE
block_node::block_node(const stoken &token) :
	syntax_node(token) {
	type = syntax_type::syn_block;
}

void block_node::push_front(syntax_node *node) {
	children.push_front(node);
}
void block_node::push_local(const std::wstring &_str) {
    assert(_str.empty() == false);

    auto str = _str;
    if (str[0] == L'*')
        str = str.substr(1);

	if (locals.empty() ||
		std::find(locals.begin(), locals.end(), str) == locals.end()) {

		declaring_method()->local_size++;
		locals.push_back(str);
	}
}