#include "stdafx.h"

#include "syntax_node.h"

// TRY_NODE
try_node::try_node(const stoken &token) :
    syntax_node(token) {
    capacity = 1;
    type = syntax_type::syn_try;
}

block_node *try_node::block() {
    return (block_node*)children[0];
}

// CATCH_NODE
catch_node::catch_node(const stoken &token) :
    syntax_node(token) {
    capacity = 2;
    type = syntax_type::syn_catch;
}

ident_node *catch_node::exception() {
    return (ident_node*)children[0];
}
block_node *catch_node::block() {
    return (block_node*)children[1];
}
void catch_node::on_complete() {
    block()->push_local(exception()->ident);
}

// FINALLY_NODE
finally_node::finally_node(const stoken &token) :
    syntax_node(token) {
    capacity = 1;
    type = syntax_type::syn_finally;
}

// THROW_NODE
throw_node::throw_node(const stoken &token) :
    syntax_node(token) {
    capacity = 1;
    type = syntax_type::syn_throw;
}
syntax_node *throw_node::exception() {
    return children[0];
}