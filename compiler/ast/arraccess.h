#pragma once

#include "syntax_travler.h"

class arr_setitem_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_assignment &&
            node->children[0]->type == syntax_type::syn_arraccess;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto new_node = new callmember_node(node->s_token());
            auto arr_node = (arraccess_node*)node->children[0];
            auto at = new ident_node(node->s_token(), rk_setitem);

            // METHOD_NAME
            new_node->append(at);
            // .this
            new_node->append(arr_node->children[0]);

            // ARGS 
            new_node->append(arr_node->children[1]);
            new_node->append(node->children[1]);

            return new_node;
        }
        return node;
    }
};
class arr_getitem_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_arraccess &&
            node->parent->type != syntax_type::syn_assignment;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto new_node = new callmember_node(node->s_token());
            auto at = new ident_node(node->s_token(), rk_getitem);

            // METHOD_NAME
            new_node->append(at);
            // .this
            new_node->append(node->children[0]);

            // ARGS
            new_node->append(node->children[1]);

            return new_node;
        }
        return node;
    }
};