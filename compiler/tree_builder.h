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
            rklog("%s\n", token.raw.c_str());

            _ending_expression(end_block);
            _ending_expression(st_end_param);
            _ending_expression(st_end_call);
            _ending_expression(st_end_arr);

            if (token.type == stoken_type::endl)
                append_and_set(new endl_node(token, current));
            else if (token.type == stoken_type::st_include)
                append_and_replace(include(token));
            else if (token.type == stoken_type::begin_block) {
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
            else if (token.type == stoken_type::st_annotation) {
                append_and_replace(annotation(token));
            }

            else if (token.type == stoken_type::st_begin_param) {
                append_and_replace(params(token));
            }

            else if (token.type == stoken_type::st_begin_call) {
                append_and_replace(call(token));
            }

            else if (token.type == stoken_type::st_begin_arr) {
                append_and_replace(arr(token));
            }

            else if (token.type == stoken_type::st_memberaccess)
                append_and_replace(memberaccess(token));
            else if (token.type == stoken_type::st_arraccess)
                append_and_replace(arraccess(token));

            else if (token.type == stoken_type::st_if)
                append_and_replace(_if(token));
            else if (token.type == stoken_type::st_for)
                append_and_replace(_for(token));
            else if (token.type == stoken_type::st_return)
                append_and_replace(_return(token));

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

    include_node *include(const stoken &token) {
        auto node = new include_node(token, current);
        return node;
    }
    class_node *klass(const stoken &token) {
        auto node = new class_node(token, current);
        return node;
    }
    params_node *params(const stoken &token) {
        auto node = new params_node(token, current);
        return node;
    }
    annotation_node *annotation(const stoken &token) {
        auto node = new annotation_node(token, current);
        return node;
    }
    method_node *method(const stoken &token) {
        auto node = new method_node(token, current);
        return node;
    }
    pop_node *pop(const stoken &token) {
        auto node = new pop_node(token, current);
        return node;
    }
    block_node *block(const stoken &token) {
        auto node = new block_node(token, current);
        return node;
    }
    newarr_node *arr(const stoken &token) {
        auto node = new newarr_node(token, current);
        return node;
    }
    call_node *call(const stoken &token) {
        auto node = new call_node(token, current);
        return node;
    }
    arraccess_node *arraccess(const stoken &token) {
        auto node = new arraccess_node(token, current);
        return node;
    }
    memberaccess_node *memberaccess(const stoken &token) {
        auto node = new memberaccess_node(token, current);
        return node;
    }
    return_node *_return(const stoken &token) {
        auto node = new return_node(token, current);
        return node;
    }
    if_node *_if(const stoken &token) {
        auto node = new if_node(token, current);
        return node;
    }
    for_node *_for(const stoken &token) {
        auto node = new for_node(token, current);
        return node;
    }
    ident_node *ident(const stoken &token) {
        auto node = new ident_node(token, current, token.raw);
        return node;
    }
    literal_node *literal(const stoken &token) {
        auto node = new literal_node(token, current);

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
        if (token.raw == L"=") {
            return new assignment_node(token, current);
        }
        else if (token.raw == L"++" || token.raw == L"--") {
            auto node = new standalone_op_node(token, current);
            node->op = token.raw;
            return node;
        }
        else {
            auto node = new op_node(token, current);
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