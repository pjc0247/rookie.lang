#pragma once

#include <string>

#include "syntax.h"

class syntax_traveler {
public:
    void transform(root_node *root) {
        _visit(root);
    }

protected:
    virtual syntax_node *visit(syntax_node *node) {
        return node;
    }
    syntax_node *_visit(syntax_node *node) {
        auto ret = visit(node);

        for (int i = 0; i < node->children.size(); i++)
            node->children[i] = _visit(node->children[i]);

        return ret;
    }
};

class tco : public syntax_traveler {
public:
    void transform(root_node *root) {
        visit(root);
    }

protected:
    bool is_transformable(const std::string &ident, syntax_node *node) {
        return node->type == syntax_type::syn_return &&
            node->children[0]->type == syntax_type::syn_call &&
            ((call_node*)node->children[0])->ident_str() == ident;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(ident, node)) {
            auto call = (call_node*)node->children[0];
            auto head = new label_node(method->body());
            method->body()->push_front(head);

            auto new_node = new block_node(node->parent);

            int cnt = 0;
            for (auto it = call->begin_args(); it != call->end_args(); ++it) {
                auto param_ident = (ident_node*)method->params()->children[cnt];
                auto assign = new assignment_node(new_node);

                auto left = new ident_node(assign, param_ident->ident);
                auto right = *it;
                assign->append(left);
                assign->append(right);
                new_node->append(assign);

                ++cnt;
            }

            new_node->append(new goto_node(new_node, head));
            return new_node;
        }
        else 
            return node;
    }

private:
    method_node *method;
    std::string ident;
};