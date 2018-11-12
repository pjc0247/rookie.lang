#include "stdafx.h"

#include "syntax_node.h"

// FOR_NODE
for_node::for_node(const stoken &token) :
	syntax_node(token) {

    nth_block_or_single = 4;
	type = syntax_type::syn_for;
}

syntax_node *for_node::init() const {
	return children[0];
}
syntax_node *for_node::cond() const {
	return children[1];
}
syntax_node *for_node::increment() const {
	return children[2];
}
syntax_node *for_node::body() const {
	return children[3];
}

// FOREACH_NODE
foreach_node::foreach_node(const stoken &token) :
	syntax_node(token) {
	type = syntax_type::syn_foreach;
}

ident_node *foreach_node::left() const {
	return (ident_node*)children[0];
}

std::deque<syntax_node*>::iterator foreach_node::begin_vars() {
	return children.begin();
}
std::deque<syntax_node*>::iterator foreach_node::end_vars() {
	return children.end() - 2;
}

syntax_node *foreach_node::right() const {
	return children[children.size() - 2];
}
syntax_node *foreach_node::body() const {
	return children[children.size() - 1];
}
void foreach_node::on_complete() {
	auto method = declaring_method();

	if (method != nullptr) {
		for (auto it = begin_vars(); it != end_vars(); ++it) {
			auto ident = ((ident_node*)*it)->ident;
			nearest_block()->push_local(ident);
		}
	}
}

// WHILE_NODE
while_node::while_node(const stoken &token) :
	syntax_node(token) {
	capacity = 2;
	type = syntax_type::syn_while;
}

syntax_node *while_node::cond() const {
	return children[0];
}
syntax_node *while_node::body() const {
	return children[1];
}