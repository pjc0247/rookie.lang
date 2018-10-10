#pragma once

#include <deque>
#include <string>
#include <vector>

#include "token.h"

enum class syntax_type {
    syn_none,
    syn_root,

    syn_class, syn_field, syn_method, syn_params,

    syn_block,
    syn_literal,
    syn_ident,

    syn_call,
    syn_label, syn_goto,
     
    syn_assignment,
    syn_op, syn_standalone_op,

    syn_if,
    syn_for,
    syn_return
};

class class_node;
class method_node;

class syntax_node {
public:
    syntax_node(syntax_node *parent) :
        type(syntax_type::syn_none),
        parent(parent), capacity(-1),
        is_virtual(false) {
    }
    virtual ~syntax_node() {
        for (auto child : children)
            delete child;
    }

    bool is_complete() const {
        return capacity > 0 ? children.size() >= capacity : false;
    }
    syntax_node *nearest_incomplete_node() {
        syntax_node *current = parent;
        if (parent == nullptr)
            return this;

        while (true) {
            printf("%s\n", typeid(*current).name());
            if (current->type == syntax_type::syn_root)
                return current;

            if (current->is_complete())
                current = current->parent;
            else
                return current;
        }
    }

    syntax_node *append(syntax_node *node) {
        node->parent = this;
        children.push_back(node);

        if (is_complete()) {
            on_complete();
            return nearest_incomplete_node();
        }
        return this;
    }
    syntax_node *pop() {
        if (children.size() == 0) return nullptr;

        auto last = children[children.size() - 1];
        children.pop_back();
        return last;
    }

    class_node *declaring_class() const {
        return (class_node*)find_upward_until(syntax_type::syn_class);
    }
    method_node *declaring_method() const {
        return (method_node*)find_upward_until(syntax_type::syn_method);
    }

    void dump(int depth = 0) {
        for (int i = 0; i < depth; i++)
            putchar(' ');

        printf("%s\n", typeid(*this).name());

        for (auto child : children)
            child->dump(depth + 1);
    }

protected:
    virtual void on_complete() { }

    syntax_node *find_upward_until(syntax_type type) const {
        syntax_node *current = parent;
        while (current != nullptr) {
            if (current->type == type)
                break;
            current = current->parent;
        }
        return current;
    }

public:
    syntax_type type;

    syntax_node *parent;
    std::deque<syntax_node*> children;
    int capacity;

    bool is_virtual;
};

class virtual_node : public syntax_node {
public:
    virtual_node(syntax_node *parent)
        : syntax_node(parent) {
        is_virtual = true;
    }
};

class root_node : public syntax_node {
public:
    root_node() :
        syntax_node(nullptr) {
        type = syntax_type::syn_root;
    }
};

class block_node : public syntax_node {
public:
    block_node(syntax_node *parent) :
        syntax_node(parent) {
        type = syntax_type::syn_block;
    }

    void push_front(syntax_node *node) {
        children.push_front(node);
    }
};

class literal_node : public syntax_node {
public:
    literal_node(syntax_node *parent) :
        syntax_node(parent) {
        type = syntax_type::syn_literal;
    }
    literal_node(syntax_node *parent, int n) :
        literal_node(parent) {
        literal_type = literal_type::integer;
        integer = n;
    }
    literal_node(syntax_node *parent, const std::string &_str) :
        literal_node(parent) {
        literal_type = literal_type::string;
        str = _str;
    }

public:
    literal_type literal_type;

    int integer;
    std::string str;
};
class ident_node : public syntax_node {
public:
    ident_node(syntax_node *parent, const std::string &ident) :
        syntax_node(parent), ident(ident) {
        type = syntax_type::syn_ident;
    }
public:
    std::string ident;
};

class params_node : public syntax_node {
public:
    params_node(syntax_node *parent) :
        syntax_node(parent) {
        type = syntax_type::syn_params;
    }
};
class method_node : public syntax_node {
public:
    method_node(syntax_node *parent) :
        syntax_node(parent) {
        capacity = 3;
        type = syntax_type::syn_method;
    }

    ident_node *ident() {
        return (ident_node*)children[0];
    }
    const std::string &ident_str() {
        return ident()->ident;
    }
    params_node *params() {
        return (params_node*)children[1];
    }
    block_node *body() {
        return (block_node*)children[2];
    }

    void push_local(const std::string &str) {
        if (std::find(locals.begin(), locals.end(), str) == locals.end())
            locals.push_back(str);
    }

public:
    std::vector<std::string> locals;
};

class field_node : public syntax_node {
public:
    field_node(syntax_node *parent) :
        syntax_node(parent) {
        capacity = 1;
        type = syntax_type::syn_field;
    }

    ident_node *ident() const {
        return dynamic_cast<ident_node*>(children[0]);
    }
    std::string &ident_str() const {
        return ident()->ident;
    }
};
class class_node : public syntax_node {
public:
    class_node(syntax_node *parent) :
        syntax_node(parent) {
        type = syntax_type::syn_class;
    }

public:
    std::vector<field_node*> fields;
    std::vector<method_node*> methods;
};

class call_node : public syntax_node {
public:
    call_node(syntax_node *parent) :
        syntax_node(parent) {
        syntax_type::syn_call;
    }

    ident_node *ident() {
        return (ident_node*)children[0];
    }
    const std::string &ident_str() {
        return ident()->ident;
    }

    std::deque<syntax_node*>::iterator begin_args() {
        return std::next(children.begin());
    }
    std::deque<syntax_node*>::iterator end_args() {
        return children.end();
    }
};

class return_node : public syntax_node {
public:
    return_node(syntax_node *parent) :
        syntax_node(parent) {
        capacity = 1;
        type = syntax_type::syn_return;
    }

    syntax_node *value() {
        if (children.size() == 0)
            return nullptr;
        return children[0];
    }
};

class label_node : public syntax_node {
public:
    label_node(syntax_node *parent) :
        syntax_node(parent) {
        type = syntax_type::syn_label;
    }
};
class goto_node : public syntax_node {
public:
    goto_node(syntax_node *parent, label_node *dst) :
        syntax_node(parent), dst(dst) {
        type = syntax_type::syn_goto;
    }

public:
    label_node * dst;
};

class standalone_op_node : public syntax_node {
public:
    standalone_op_node(syntax_node *parent) :
        syntax_node(parent) {

        capacity = 1;
        type = syntax_type::syn_standalone_op;
    }

    syntax_node *left() {
        return children[0];
    }
public:
    std::string op;
};

class op_node : public syntax_node {
public:
    op_node(syntax_node *parent) :
        syntax_node(parent) {

        capacity = 2;
        type = syntax_type::syn_op;
    }

    syntax_node *left() {
        return children[0];
    }
    syntax_node *right() {
        return children[1];
    }

public:
    std::string op;
};

class assignment_node : public op_node {
public:
    assignment_node(syntax_node *parent) :
        op_node(parent) {

        capacity = 2;
        type = syntax_type::syn_assignment;
    }

protected:
    virtual void on_complete() {
        auto ident = dynamic_cast<ident_node*>(left());
        declaring_method()->push_local(ident->ident);
    }
};

class for_node : public syntax_node {
public:
    for_node(syntax_node *parent) :
        syntax_node(parent) {
        capacity = 4;
        type = syntax_type::syn_for;
    }

    syntax_node *init() {
        return children[0];
    }
    syntax_node *cond() {
        return children[1];
    }
    syntax_node *increment() {
        return children[2];
    }
    syntax_node *body() {
        return children[3];
    }
};