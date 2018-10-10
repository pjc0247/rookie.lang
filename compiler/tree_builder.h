#pragma once

#include <vector>

#include "token.h"
#include "syntax.h"

class tree_builder {
public:
	root_node *build(const std::vector<stoken> &stokens) {
		auto root = new root_node();
		current = root;

		for (auto &token : stokens) {
			printf("%s\n", token.raw.c_str());
			if (token.type == stoken_type::begin_block) {
				append_and_set(block(token));
			}
			else if (token.type == stoken_type::end_block) {
				current = current->nearest_incomplete_node();
			}
			else {
				syntax_node *node = nullptr;

				if (token.type == stoken_type::op)
					node = op(token);
				else if (token.type == stoken_type::st_class) {
					current_class = klass(token);
					node = current_class;
				}
				else if (token.type == stoken_type::st_defmethod) {
					current_method = method(token);
					node = current_method;
				}

				if (node != nullptr)
					append_and_set(node);
			}
		}

		return root;
	}

private:
	void append_and_set(syntax_node *node) {
		current->children.push_back(node);
		current = node;
	}

	class_node *klass(const stoken &token) {
		auto node = new class_node(current);
		return node;
	}
	method_node *method(const stoken &token) {
		auto node = new method_node(current);
		return node;
	}
	block_node *block(const stoken &token) {
		auto node = new block_node(current);
		return node;
	}
	syntax_node *op(const stoken &token) {
		if (token.raw == "=") {
			return new assignment_node(current);
		}
		else if (token.raw == "++" || token.raw == "--") {
			auto node = new standalone_op_node(current);
			node->op = token.raw;
			return node;
		}
		else {
			auto node = new op_node(current);
			node->op = token.raw;
			return node;
		}
	}

private:
	class_node *current_class;
	method_node *current_method;
	syntax_node *current;
};