/*  tree_builder.h

    Builds an AST tree from S-Expressions.
 */
#pragma once

#include <vector>

#include "token.h"
#include "syntax/syntax_node.h"
#include "compilation.h"
#include "type_attr.h"

#define _ending_expression(expname) \
    if (token.type == stoken_type:: expname) { \
        current = current->nearest_incomplete_node(); continue; }

const stoken empty_stoken = stoken::empty();

class tree_builder {
public:
    tree_builder(compile_context &ctx) :
        ctx(ctx) {

    }

    root_node *build(const std::vector<stoken> &_stokens) {
        stokens = &_stokens;

        auto root = new root_node();
        current = root;

        ctx.root_node = root;

        for (cursor=0; cursor<stokens->size(); cursor++) {
            auto &token = stokens->at(cursor);

            if (current->is_complete())
                current = current->nearest_incomplete_node();

            //_ending_expression(st_end_block);
            if (token.type == stoken_type::st_end_block) {
                //current = current->parent;
                current = current->nearest_incomplete_node();
            }
            _ending_expression(st_end_param);
            _ending_expression(st_end_call);
            _ending_expression(st_end_arr);

            if (token.raw == L"in")
                current->nth_block_or_single = current->children.size() + 1;

            else if (token.type == stoken_type::endl)
                append_and_set(new endl_node(token));
            else if (token.type == stoken_type::comma)
                append_and_set(new comma_node(token));
            else if (token.type == stoken_type::st_include)
                append_and_replace(include(token));
            else if (token.type == stoken_type::st_begin_block) {
                append_and_replace(block(token));
            }
            else if (token.type == stoken_type::st_class) {
                current_class = klass(token);
                append_and_replace(current_class);
            }
            else if (token.type == stoken_type::st_begin_inherit) {
                append_and_replace(inherit(token));
            }
            else if (token.type == stoken_type::st_end_inherit) {
                if (current->type == syntax_type::syn_inherit)
                    current = current->parent;
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
            else if (token.type == stoken_type::st_else)
                append_and_replace(_else(token));
            else if (token.type == stoken_type::st_for)
                append_and_replace(_for(token));
            else if (token.type == stoken_type::st_foreach)
                append_and_replace(_foreach(token));
            else if (token.type == stoken_type::st_while)
                append_and_replace(_while(token));
            else if (token.type == stoken_type::st_return)
                append_and_replace(_return(token));
            else if (token.type == stoken_type::st_try)
                append_and_replace(___try(token));
            else if (token.type == stoken_type::st_catch)
                append_and_replace(_catch(token));
            else if (token.type == stoken_type::st_throw)
                append_and_replace(_throw(token));

            else if (token.type == stoken_type::op) {
                append_and_replace(op(token));
            }
            else {
                syntax_node *node = nullptr;

                if (token.type == stoken_type::op)
                    node = op(token);
                else if (token.type == stoken_type::ident)
                    node = ident(token);
                else if (token.type == stoken_type::st_this)
                    node = _this(token);
                else if (token.type == stoken_type::st_literal)
                    node = literal(token);
                else if (token.type == stoken_type::st_null)
                    node = simple_create<null_node>(token);
                else if (token.type == stoken_type::st_true ||
                         token.type == stoken_type::st_false)
                    node = _bool(token);

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

    template <typename T>
    T *simple_create(const stoken &token) {
        auto node = new T(token, current);
        return node;
    }

    include_node *include(const stoken &token) {
        auto node = new include_node(token);
        return node;
    }
    class_node *klass(const stoken &token) {
        auto node = new class_node(token);
        return node;
    }
    inherit_node *inherit(const stoken &token) {
        auto node = new inherit_node(token);
        return node;
    }
    params_node *params(const stoken &token) {
        auto node = new params_node(token);
        return node;
    }
    annotation_node *annotation(const stoken &token) {
        auto node = new annotation_node(token);
        return node;
    }
    method_node *method(const stoken &token) {
        auto node = new method_node(token);

        if (prev_token().type == stoken_type::st_static)
            node->attr |= method_attr::method_static;

        return node;
    }
    pop_node *pop(const stoken &token) {
        auto node = new pop_node(token);
        return node;
    }
    block_node *block(const stoken &token) {
        auto node = new block_node(token);
        return node;
    }
    newarr_node *arr(const stoken &token) {
        auto node = new newarr_node(token);
        return node;
    }
    call_node *call(const stoken &token) {
        auto node = new call_node(token);
        return node;
    }
    arraccess_node *arraccess(const stoken &token) {
        auto node = new arraccess_node(token);
        return node;
    }
    memberaccess_node *memberaccess(const stoken &token) {
        auto node = new memberaccess_node(token);
        return node;
    }
    return_node *_return(const stoken &token) {
        auto node = new return_node(token);
        return node;
    }
    if_node *_if(const stoken &token) {
        auto node = new if_node(token);
        return node;
    }
    else_node *_else(const stoken &token) {
        auto node = new else_node(token);
        return node;
    }
    for_node *_for(const stoken &token) {
        auto node = new for_node(token);
        return node;
    }
    foreach_node *_foreach(const stoken &token) {
        auto node = new foreach_node(token);
        return node;
    }
    while_node *_while(const stoken &token) {
        auto node = new while_node(token);
        return node;
    }
    try_node *___try(const stoken &token) {
        auto node = new try_node(token);
        return node;
    }
    catch_node *_catch(const stoken &token) {
        auto node = new catch_node(token);
        return node;
    }
    throw_node *_throw(const stoken &token) {
        auto node = new throw_node(token);
        return node;
    }
    this_node *_this(const stoken &token) {
        auto node = new this_node(token);
        return node;
    }
    bool_node *_bool(const stoken &token) {
        bool v;
        if (token.raw == L"true") v = true;
        else v = false;

        auto node = new bool_node(token, v);
        return node;
    }
    ident_node *ident(const stoken &token) {
        auto node = new ident_node(token, token.raw);
        return node;
    }
    literal_node *literal(const stoken &token) {
        auto node = new literal_node(token);

        node->literal_type = token.source.literal_type;
        switch (token.source.literal_type) {
        case literal_type::boolean:
            node->integer = token.raw == L"true" ? 1 : 0;
            break;
        case literal_type::integer:
            node->integer = std::stoi(token.raw);
            break;
        case literal_type::decimal:
            node->decimal = std::stof(token.raw);
            break;
        case literal_type::string:
        case literal_type::string_with_interpoloation:
            node->str = token.raw;
            break;
        }

        return node;
    }
    syntax_node *op(const stoken &token) {
        if (token.raw == L"=") {
            return new assignment_node(token);
        }
        else if (token.raw == L"++" || token.raw == L"--") {
            auto node = new standalone_op_node(token);
            node->op = token.raw;
            return node;
        }
        else {
            auto node = new op_node(token);
            node->op = token.raw;
            return node;
        }
    }

    const stoken &prev_token() const {
        if (cursor == 0)
            return empty_stoken;
        return stokens->at(cursor - 1);
    }
    const stoken &next_token() const {
        if (cursor + 1 == stokens->size())
            return empty_stoken;
        return stokens->at(cursor + 1);
    }

private:
    compile_context &ctx;

    class_node *current_class;
    method_node *current_method;
    syntax_node *current;

    const std::vector<stoken> *stokens;
    uint32_t cursor;
};