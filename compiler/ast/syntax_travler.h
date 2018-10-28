#pragma once

#include "syntax.h"

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
    syntax_node *_visit(syntax_node *node) {
        auto ret = visit(node);

        for (uint32_t i = 0; i < node->children.size(); i++) {
            auto child = node->children[i];
            node->children[i] = _visit(child);

            if (child != node->children[i])
                changes++;

            // remove
            if (node->children[i] == nullptr) {
                node->children.erase(
                    std::find(node->children.begin(), node->children.end(),
                    node->children[i]));

                i--;
            }
        }

        return ret;
    }

protected:
    int changes;
};

class optimize_travler : public syntax_traveler {};