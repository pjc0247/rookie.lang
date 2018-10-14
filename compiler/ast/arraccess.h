#pragma once

#include "syntax_travler.h"

class arraccess_transformer : public syntax_traveler {
protected:
	bool is_transformable(syntax_node *node) {
		return node->type == syntax_type::syn_arraccess;
	}
	virtual syntax_node *visit(syntax_node *node) {
		if (is_transformable(node)) {
			auto new_node = new callmember_node(node->s_token(), node->parent);
			auto at = new ident_node(node->s_token(), new_node, "at");

			// METHOD_NAME
			new_node->append(at);
			// .this
			new_node->append(node->children[0]);

			// ARGS
			new_node->append(node->children[1]);

			return new_node;
		}
		return node;
	}
};