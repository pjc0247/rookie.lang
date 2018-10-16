// Provides relfection methods in AST level

#pragma once

#include <vector>
#include <functional>

#include "syntax.h"

class astr {
public:
    static std::vector<method_node*>
    find_method_with_annotation(syntax_node *root, const std::wstring &name) {
        return filter<method_node*>(root, [name](syntax_node *n) {
            if (n->type == syntax_type::syn_method) {
                return any<annotation_node*>(n, [name](annotation_node *n) {
                    return n->ident_str() == name;
                });
            }
            return false;
        });
    }

private:
    static bool
    any(
        syntax_node *root,
        const std::function<bool(syntax_node*)> &pred) {

        return _any(root, pred);
    }
    static bool
    _any(
        syntax_node *node,
        const std::function<bool(syntax_node*)> &pred) {

        if (pred(node)) return true;
        for (auto c : node->children)
            if (_any(c, pred)) return true;
        return false;
    }

    template <typename NODE_TYPE>
    static bool
    any(
        syntax_node *root,
        const std::function<bool(NODE_TYPE)> &pred) {

        return _any(root, pred);
    }
    template <typename NODE_TYPE>
    static bool
    _any(
        syntax_node *node,
        const std::function<bool(NODE_TYPE)> &pred) {

        auto casted = dynamic_cast<NODE_TYPE>(node);
        if (casted && pred(casted)) return true;
        for (auto c : node->children)
            if (_any(c, pred)) return true;
        return false;
    }

    template <typename NODE_TYPE>
    static std::vector<NODE_TYPE>
    filter(
        syntax_node *root,
        const std::function<bool(syntax_node*)> &pred) {

        std::vector<NODE_TYPE> result;
        _filter(root, pred, result);
        return result;
    }
    template <typename NODE_TYPE>
    static void
    _filter(
        syntax_node *node,
        const std::function<bool(syntax_node*)> &pred,
        std::vector<NODE_TYPE> &result) {

        if (pred(node)) result.push_back((NODE_TYPE)node);
        for (auto c : node->children)
            _filter(c, pred, result);
    }
};