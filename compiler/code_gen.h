#pragma once

#include <map>
#include <set>

#include "syntax.h"
#include "program.h"
#include "compilation.h"

enum class lookup_type {
    not_exist,
    local,
    field
};
struct lookup_result {
    lookup_type type;
    int index;
};

class scope {
public:
    void set_class(class_node *node) {
        current_class = node;
    }
    void set_method(method_node *node) {
        current_method = node;
    }

    lookup_result lookup(const std::string &ident) {
        lookup_result result;
        
        for (int i = 0; i<current_method->locals.size(); i++) {
            if (current_method->locals[i] == ident) {
                result.type = lookup_type::local;
                result.index = i;
                return result;
            }
        }

        for (int i=0;i<current_class->fields.size();i++) {
            if (current_class->fields[i]->ident_str() == ident) {
                result.type = lookup_type::field;
                result.index = i;
                return result;
            }
        }

        result.type = lookup_type::not_exist;
        return result;
    }

private:
    class_node *current_class;
    method_node *current_method;
};

class string_pool {
public:
    int get_ptr(const std::string &str) {
        if (indexes.find(str) != indexes.end())
            return indexes[str];
        return append(str);
    }
    const char *fin() const {
        return &(pool.front());
    }
    unsigned int size() const {
        return pool.size();
    }

    void dump() {
        printf("===begin_string_pool====\r\n");
        for (int i = 0; i < pool.size(); i++)
            putchar(pool[i]);
        printf("\r\n===end_string_pool====\r\n");
    }
private:
    int append(const std::string &str) {
        auto ptr = pool.size();
        pool.insert(pool.end(), str.begin(), str.end());
        pool.insert(pool.end(), 0);
        indexes[str] = ptr;
        return ptr;
    }

private:
    std::map<std::string, int> indexes;
    std::vector<char> pool;
};

class program_builder {
public:
    void emit(opcode_t opcode, const std::string &operand) {
        emit(opcode, spool.get_ptr(operand));
    }
    void emit(opcode_t opcode, int operand) {
        instructions.push_back(instruction(opcode, operand));
    }
    void emit(opcode_t opcode) {
        instructions.push_back(instruction(opcode, 0));
    }

    int get_cursor() const {
        return instructions.size();
    }

    program fin() const {
        program p;
        p.header.code_len = instructions.size();
        p.header.rdata_len = spool.size();
        p.code = &instructions[0];
        p.rdata = spool.fin();
        return p;
    }

private:
    string_pool spool;

    std::vector<instruction> instructions;
};

class code_gen {
public:
    code_gen(compile_context &ctx) :
        ctx(ctx) {
    }

    void generate(root_node *root) {
        scope = ::scope();
        emitter = program_builder();

        emit(root);
    }

private:
#define _route(syntax_name) \
    case syntax_type::syn_##syntax_name: \
        emit_##syntax_name (dynamic_cast<syntax_name##_node*>(node)); \
        break

    void emit(syntax_node *node) {
        if (node == nullptr) return;

        if (node->is_virtual)
            ; // incomplete vnode transformation

        switch (node->type) {
            _route(class);
            _route(method);
            _route(return);
            _route(block);
            _route(literal);
            _route(op);
            _route(assignment);
            _route(for);
        }
    }

    void emit_class(class_node *node) {
        scope.set_class(node);

        for (int i = 1; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_method(method_node *node) {
        scope.set_method(node);

        emit(node->body());
    }
    void emit_return(return_node *node) {
        auto val = node->value();
        if (val != nullptr)
            emit(val);
        emitter.emit(opcode::op_ret);
    }
    void emit_block(block_node *node) {
        for (auto child : node->children)
            emit(child);
    }
    void emit_literal(literal_node *node) {
        if (node->literal_type == literal_type::integer)
            emitter.emit(opcode::op_ldi, node->integer);
        else if (node->literal_type == literal_type::string)
            emitter.emit(opcode::op_ldstr, node->str);
    }
    void emit_op(op_node *node) {
        emit(node->left());
        emit(node->right());

        if (node->op == "+")
            emitter.emit(opcode::op_add);
        else if (node->op == "-")
            emitter.emit(opcode::op_sub);
        else if (node->op == "*")
            emitter.emit(opcode::op_mul);
        else if (node->op == "/")
            emitter.emit(opcode::op_div);
        else if (node->op == ">")
            emitter.emit(opcode::op_g);
        else if (node->op == "<")
            emitter.emit(opcode::op_l);
        else if (node->op == ">=")
            emitter.emit(opcode::op_ge);
        else if (node->op == "<=")
            emitter.emit(opcode::op_le);
    }
    void emit_assignment(assignment_node *node) {
        emit(node->right());

        auto ident = dynamic_cast<ident_node*>(node->left());
        if (ident == nullptr) {
            return;
        }

        auto lookup = scope.lookup(ident->ident);
        if (lookup.type == lookup_type::not_exist) {
            return;
        }

        if (lookup.type == lookup_type::local)
            emitter.emit(opcode::op_ldloc, lookup.index);
        else if (lookup.type == lookup_type::field)
            emitter.emit(opcode::op_ldstate, lookup.index);
    }
    void emit_for(for_node *node) {
        emit(node->init());
        auto cursor = emitter.get_cursor();
        emit(node->body());
        emit(node->increment());
        emit(node->cond());
        emitter.emit(opcode::op_jmp_true, cursor);
    }

private:
    compile_context &ctx;

    scope scope;
    program_builder emitter;
};