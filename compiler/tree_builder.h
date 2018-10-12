#pragma once

#include <vector>

#include "token.h"
#include "syntax.h"
#include "compilation.h"

#define _ending_expression(expname) \
	if (token.type == stoken_type:: expname) { \
		current = current->nearest_incomplete_node(); continue; }

class tree_builder {
public:
    tree_builder(compile_context &ctx) :
        ctx(ctx) {

    }

    root_node *build(const std::vector<stoken> &stokens) {
        auto root = new root_node(ctx);
        current = root;

        for (auto &token : stokens) {
            printf("%s\n", token.raw.c_str());

			_ending_expression(end_block);
			_ending_expression(st_end_param);
			_ending_expression(st_end_call);

			//if (token.type == stoken_type::endl)
			//	current = current->parent;
			if (token.type == stoken_type::begin_block) {
				append_and_replace(block(token));
			}
			else if (token.type == stoken_type::st_class) {
				current_class = klass(token);
				append_and_replace(current_class);
			}
			else if (token.type == stoken_type::st_defmethod) {
				current_method = method(token);
				current_class->methods.push_back(current_method);
				append_and_replace(current_method);
			}

			else if (token.type == stoken_type::st_begin_param) {
				append_and_replace(params(token));
			}

			else if (token.type == stoken_type::st_begin_call) {
				append_and_replace(call(token));
			}

			else if (token.type == stoken_type::st_if)
				append_and_replace(_if(token));
			else if (token.type == stoken_type::st_for)
				append_and_replace(_for(token));

            else if (token.type == stoken_type::op) {
                append_and_replace(op(token));
            }
            else {
                syntax_node *node = nullptr;

                if (token.type == stoken_type::op)
                    node = op(token);
                else if (token.type == stoken_type::ident)
                    node = ident(token);
                else if (token.type == stoken_type::st_literal)
                    node = literal(token);

                if (node != nullptr)
                    append_and_set(node);
            }
        }

        return root;
    }

private:
    void append_and_set(syntax_node *node) {
        //printf("append %s / %s\n", typeid(*current).name(), typeid(*node).name());
        current = current->append(node);
    }
    void append_and_replace(syntax_node *node) {
        //printf("append %s / %s\n", typeid(*current).name(), typeid(*node).name());
        current->append(node);
        current = node;
    }

    class_node *klass(const stoken &token) {
        auto node = new class_node(current);
        return node;
    }
    params_node *params(const stoken &token) {
        auto node = new params_node(current);
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
    call_node *call(const stoken &token) {
        auto node = new call_node(current);
        return node;
    }
	if_node *_if(const stoken &token) {
		auto node = new if_node(current);
		return node;
	}
	for_node *_for(const stoken &token) {
		auto node = new for_node(current);
		return node;
	}
    ident_node *ident(const stoken &token) {
        auto node = new ident_node(current, token.raw);
        return node;
    }
    literal_node *literal(const stoken &token) {
        auto node = new literal_node(current);

		node->literal_type = token.source.literal_type;
		switch (token.source.literal_type) {
		case literal_type::integer:
			node->integer = std::stoi(token.raw);
			break;
		case literal_type::string:
			node->str = token.raw;
			break;
		}

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
    compile_context &ctx;

    class_node *current_class;
    method_node *current_method;
    syntax_node *current;
};