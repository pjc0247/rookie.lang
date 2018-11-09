#pragma once

#include "syntax/syntax_node.h"

class syntax_traveler {
public:
    virtual int transform(root_node *root) {
        _visit(root);

        return changes;
    }
    void prepare() {
        changes = 0;
    }

protected:
    virtual syntax_node *visit(syntax_node *node) {
        return node;
    }
    void _visit(syntax_node *node) {
        for (uint32_t i = 0; i < node->children.size(); i++) {
            auto child = node->children[i];

            node->children[i] = visit(child);

            if (child != node->children[i]) {
                changes++;
            }

            // `return nullptr;` means 'erase me'
            if (node->children[i] == nullptr) {
                node->children.erase(
                    std::find(node->children.begin(), node->children.end(),
                    node->children[i]));

                i--;
            }
            // Make sure new_node always has a parent.
            else {
                node->children[i]->parent = node;

                if (child != node->children[i]) {
                    node->children[i]->force_complete();
                }
            }

            _visit(child);
        }
    }

protected:
    int changes;
};

class optimize_travler : public syntax_traveler {};