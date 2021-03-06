#pragma once

#include "syntax/syntax_node.h"

class vnode_transformer {
public:
    void transform(root_node *root) {
        _transform(root);
    }

private:
    syntax_node *_transform(syntax_node *node) {
        if (node->type == syntax_type::syn_standalone_op)
            node = standalone_op((standalone_op_node*)node);

        for (uint32_t i = 0; i < node->children.size(); i++) {
            node->children[i] = _transform(node->children[i]);
            node->children[i]->parent = node;
        }

        return node;
    }

    // BEFORE: a ++
    // AFTER:  a = a + 1
    syntax_node *standalone_op(standalone_op_node *node) {
        syntax_node *new_node = new assignment_node(node->source);
        new_node->append(node->left());
        auto plus = new op_node(node->source);
        plus->append(node->left());
        plus->append(new literal_node(node->source, 1));
        if (node->op == L"++") plus->op = L"+";
        else if (node->op == L"--") plus->op = L"-";
        new_node->append(plus);

        return new_node;
    }
};