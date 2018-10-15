#pragma once

#include <string>

#include "ast/syntax_travler.h"

class precalc : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_op &&
            node->children[0]->type == syntax_type::syn_literal &&
            node->children[1]->type == syntax_type::syn_literal;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto left = (literal_node*)node->children[0];
            auto right = (literal_node*)node->children[1];

            if (left->literal_type == literal_type::integer &&
                right->literal_type == left->literal_type) {

                auto _node = new literal_node(node->source, node->parent);
                _node->literal_type = literal_type::integer;

                auto op = ((op_node*)node)->op;

                if (op == L"+")
                    _node->integer = left->integer + right->integer;
                else if (op == L"-")
                    _node->integer = left->integer - right->integer;
                else if (op == L"*")
                    _node->integer = left->integer * right->integer;
                else if (op == L"/")
                    _node->integer = left->integer / right->integer;
                else if (op == L">")
                    _node->integer = left->integer > right->integer;
                else if (op == L"<")
                    _node->integer = left->integer < right->integer;
                else if (op == L">=")
                    _node->integer = left->integer >= right->integer;
                else if (op == L"<=")
                    _node->integer = left->integer <= right->integer;
                // unimplemented op, stash current works
                else
                    return node;

                node = _node;
            }
        }
        return node;
    }
};

class tco : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_return &&
            node->children[0]->type == syntax_type::syn_call &&
            ((call_node*)node->children[0])->ident_str() == node->declaring_method()->ident_str();
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto method = node->declaring_method();
            auto call = (call_node*)node->children[0];
            auto head = new label_node(stoken::empty(), method->body());
            method->body()->push_front(head);

            auto new_node = new block_node(stoken::empty(), node->parent);

            int cnt = 0;
            for (auto it = call->begin_args(); it != call->end_args(); ++it) {
                auto param_ident = (ident_node*)method->params()->children[cnt];
                auto assign = new assignment_node(node->source, new_node);

                auto left = new ident_node(
                    param_ident->source, assign, param_ident->ident);
                auto right = *it;
                assign->append(left);
                assign->append(right);
                new_node->append(assign);

                ++cnt;
            }

            new_node->append(new goto_node(stoken::empty(), new_node, head));
            return new_node;
        }
        else 
            return node;
    }
};