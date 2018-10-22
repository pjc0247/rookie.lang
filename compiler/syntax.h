#pragma once

#include <deque>
#include <string>
#include <vector>
#include <list>

#include "token.h"

enum class syntax_type {
    syn_none,
    syn_root,

    syn_endl,
    syn_pop,

    syn_include,

    syn_annotation,
    syn_class, syn_field, syn_method, syn_params,
    syn_inherit,

    syn_block,
    syn_literal,
    syn_ident,
    syn_bool,
    syn_this,
    syn_null,
    syn_newarr,
    syn_newobj,
    syn_newdic,

    syn_arraccess,
    syn_memberaccess,
    syn_call, syn_callmember,
    syn_label, syn_goto,
     
    syn_assignment,
    syn_op, syn_standalone_op,

    syn_if,
    syn_for,
    syn_return,

    syn_try, syn_catch, syn_finally
};

class root_node;
class class_node;
class method_node;
class params_node;

class syntax_node {
public:
    syntax_node(const stoken &token, syntax_node *parent) :
        source(token),
        type(syntax_type::syn_none),
        parent(parent), capacity(-1),
        is_virtual(false) {

        root_ref = parent ? parent->root() : nullptr;
    }

    bool is_complete() const;
    syntax_node *nearest_incomplete_node();

    syntax_node *append(syntax_node *node, bool fire_oncomplete=true) {
        // 'endl' only can be accepted in block_node.
        if (node->type == syntax_type::syn_endl) {
            if (type != syntax_type::syn_block)
                return this;
        }

        node->parent = this;
        children.push_back(node);

        if (fire_oncomplete && is_complete()) {
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

    token &token() {
        return source.source;
    }
    stoken &s_token() {
        return source;
    }

    root_node *root() const {
        return root_ref;
    }
    
    class_node *declaring_class() const {
        return (class_node*)find_upward_until(syntax_type::syn_class);
    }
    method_node *declaring_method() const {
        return (method_node*)find_upward_until(syntax_type::syn_method);
    }

    void dump(int depth = 0);

    virtual void on_validate() { }

protected:
    virtual void on_complete() { }

    //compile_context &ctx() const;

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
    stoken source;

    syntax_type type;

    root_node *root_ref;
    syntax_node *parent;
    std::deque<syntax_node*> children;
    uint32_t capacity;

    bool is_virtual;
};

class virtual_node : public syntax_node {
public:
    virtual_node(const stoken &token, syntax_node *parent)
        : syntax_node(token, parent) {
        is_virtual = true;
    }
};
class endl_node : public syntax_node {
public:
    endl_node(const stoken &token, syntax_node *parent)
        : syntax_node(token, parent) {
        type = syntax_type::syn_endl;
    }
};
class pop_node : public syntax_node {
public:
    pop_node(const stoken &token, syntax_node *parent)
        : syntax_node(token, parent) {
        type = syntax_type::syn_pop;
    }
};

class this_node : public syntax_node {
public:
    this_node(const stoken &token, syntax_node *parent)
        : syntax_node(token, parent) {
        type = syntax_type::syn_this;
    }
};
class null_node : public syntax_node {
public:
    null_node(const stoken &token, syntax_node *parent)
        : syntax_node(token, parent) {
        type = syntax_type::syn_null;
    }
};

class root_node : public syntax_node {
public:
    root_node() :
        syntax_node(stoken(::token()), nullptr) {

        root_ref = this;
        type = syntax_type::syn_root;
    }
    virtual ~root_node() {
        for (auto child : flatten_children)
            delete child;
    }

    void add_reference(syntax_node *node) {
        flatten_children.push_back(node);
    }

private:
    std::list<syntax_node*> flatten_children;
};

class block_node : public syntax_node {
public:
    block_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_block;
    }

    void push_front(syntax_node *node) {
        children.push_front(node);
    }
};

class literal_node : public syntax_node {
public:
    literal_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_literal;
    }
    literal_node(const stoken &token, syntax_node *parent, int n) :
        literal_node(token, parent) {
        literal_type = literal_type::integer;
        integer = n;
    }
    literal_node(const stoken &token, syntax_node *parent, const std::wstring &_str) :
        literal_node(token, parent) {
        literal_type = literal_type::string;
        str = _str;
    }

public:
    literal_type literal_type;

    int integer;
    std::wstring str;
};
class ident_node : public syntax_node {
public:
    ident_node(const stoken &token, syntax_node *parent, const std::wstring &ident) :
        syntax_node(token, parent), ident(ident) {
        type = syntax_type::syn_ident;
    }
public:
    std::wstring ident;
};

class include_node : public syntax_node {
public:
    include_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_include;
    }

    std::wstring &path() const {
        return ((literal_node*)children[0])->str;
    }
};

class memberaccess_node : public syntax_node {
public:
    memberaccess_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_memberaccess;
        capacity = 2;
    }

    const std::wstring &property_name() const {
        return ((ident_node*)children[1])->ident;
    }
};
class arraccess_node : public syntax_node {
public:
    arraccess_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_arraccess;
        capacity = 2;
    }
};

class params_node : public syntax_node {
public:
    params_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_params;
    }
};
class annotation_node : public syntax_node {
public:
    annotation_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_annotation;
    }

    ident_node *ident() {
        return (ident_node*)children[0];
    }
    const std::wstring &ident_str() {
        return ident()->ident;
    }
};
class method_node : public syntax_node {
public:
    method_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        attr = 0;
        capacity = 3;
        type = syntax_type::syn_method;
    }

    ident_node *ident() {
        return (ident_node*)children[0];
    }
    const std::wstring &ident_str() {
        return ident()->ident;
    }
    params_node *params() {
        return (params_node*)children[1];
    }
    block_node *body() {
        return (block_node*)children[2];
    }

    void push_local(const std::wstring &str) {
        if (locals.empty() ||
            std::find(locals.begin(), locals.end(), str) == locals.end())
            locals.push_back(str);
    }
    void push_annotation(annotation_node *node) {
        append(node, false);
    }

protected:
    virtual void on_complete() {
        auto prev_locals = locals;

        locals.clear();
        for (uint32_t i = 0; i < params()->children.size(); i++) {
            if (params()->children[i]->type == syntax_type::syn_ident)
                push_local(((ident_node*)params()->children[i])->ident);
        }

        for (auto &ident : prev_locals)
            push_local(ident);
    }

public:
    std::vector<std::wstring> locals;

    unsigned int attr;
};
class field_node : public syntax_node {
public:
    field_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_field;
    }

    ident_node *ident() const {
        return dynamic_cast<ident_node*>(children[0]);
    }
    std::wstring &ident_str() const {
        return ident()->ident;
    }
};

class inherit_node : public syntax_node {
public:
    inherit_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_inherit;
    }
};
class class_node : public syntax_node {
public:
    class_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        attr = 0;
        type = syntax_type::syn_class;
    }

    inherit_node *parents() const {
        if (children.size() <= 1) return nullptr;
        if (children[1]->type == syntax_type::syn_inherit)
            return (inherit_node*)children[1];
        return nullptr;
    }

    ident_node *ident() const {
        return dynamic_cast<ident_node*>(children[0]);
    }
    std::wstring &ident_str() const {
        return ident()->ident;
    }

public:
    std::vector<field_node*> fields;
    std::vector<method_node*> methods;

    unsigned int attr;
};

class call_node : public syntax_node {
public:
    call_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_call;
    }

    ident_node *ident() {
        return (ident_node*)children[0];
    }
    const std::wstring &ident_str() {
        return ident()->ident;
    }

    std::deque<syntax_node*>::iterator begin_args() {
        return std::next(children.begin());
    }
    std::deque<syntax_node*>::iterator end_args() {
        return children.end();
    }
};
class callmember_node : public call_node {
public:
    callmember_node(const stoken &token, syntax_node *parent) :
        call_node(token, parent) {
        type = syntax_type::syn_callmember;
    }
};

class return_node : public syntax_node {
public:
    return_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_return;
    }

    syntax_node *value() {
        if (children.size() == 0)
            return nullptr;
        return children[0];
    }
};

class bool_node : public syntax_node {
public:
    bool_node(const stoken &token, syntax_node *parent, bool v) :
        syntax_node(token, parent),
        value(v) {
        capacity = 1;
        type = syntax_type::syn_bool;
    }

public:
    bool value;
};

class try_node : public syntax_node {
public:
    try_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_try;
    }

    block_node *block() {
        return (block_node*)children[0];
    }
};
class catch_node : public syntax_node {
public:
    catch_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 2;
        type = syntax_type::syn_catch;
    }

    syntax_node *expression() {
        return children[0];
    }
    block_node *block() {
        return (block_node*)children[1];
    }
};
class finally_node : public syntax_node {
public:
    finally_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 1;
        type = syntax_type::syn_finally;
    }
};

class label_node : public syntax_node {
public:
    label_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_label;
    }
};
class goto_node : public syntax_node {
public:
    goto_node(const stoken &token, syntax_node *parent, label_node *dst) :
        syntax_node(token, parent), dst(dst) {
        type = syntax_type::syn_goto;
    }

public:
    label_node * dst;
};

class standalone_op_node : public syntax_node {
public:
    standalone_op_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {

        capacity = 1;
        type = syntax_type::syn_standalone_op;
    }

    syntax_node *left() {
        return children[0];
    }
public:
    std::wstring op;
};

class op_node : public syntax_node {
public:
    op_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {

        capacity = 2;
        type = syntax_type::syn_op;
    }
    op_node(const stoken &token, syntax_node *parent, const std::wstring &op) :
        op(op),
        syntax_node(token, parent) {

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
    std::wstring op;
};

class assignment_node : public op_node {
public:
    assignment_node(const stoken &token, syntax_node *parent) :
        op_node(token, parent) {
        capacity = 2;
        type = syntax_type::syn_assignment;
    }

protected:
    virtual void on_complete() {
        auto ident = dynamic_cast<ident_node*>(left());
        if (ident != nullptr)
            declaring_method()->push_local(ident->ident);
    }
};

class newarr_node : public syntax_node {
public:
    newarr_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_newarr;
    }
};
class newobj_node : public callmember_node {
public:
    newobj_node(const stoken &token, syntax_node *parent) :
        callmember_node(token, parent) {
        type = syntax_type::syn_newobj;
    }
};
class newdic_node : public syntax_node {
public:
    newdic_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        type = syntax_type::syn_newdic;
    }
};

class if_node : public syntax_node {
public:
    if_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {

        capacity = 2;
        type = syntax_type::syn_if;
    }

    syntax_node *cond() const {
        return children[0];
    }
    syntax_node *then() const {
        return children[1];
    }
};
class for_node : public syntax_node {
public:
    for_node(const stoken &token, syntax_node *parent) :
        syntax_node(token, parent) {
        capacity = 4;
        type = syntax_type::syn_for;
    }

    syntax_node *init() const {
        return children[0];
    }
    syntax_node *cond() const {
        return children[1];
    }
    syntax_node *increment() const {
        return children[2];
    }
    syntax_node *body() const {
        return children[3];
    }
};