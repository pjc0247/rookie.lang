#pragma once

#include <map>
#include <set>

#include "syntax.h"
#include "program.h"
#include "compilation.h"
#include "binding.h"
#include "errors.h"

enum class lookup_type {
    not_exist,

    var_local,
    var_field,

    mtd_method,
    mtd_syscall
};
struct lookup_result {
    lookup_type type;
    int index;
};

class scope {
public:
	scope(calltable_builder &syscalls) :
		syscalls(syscalls) {
	}

    void set_class(class_node *node) {
        current_class = node;
    }
    void set_method(method_node *node) {
        current_method = node;
    }
    
    lookup_result lookup_method(const std::string &ident) {
        lookup_result result;

        for (int i = 0; i<current_class->methods.size(); i++) {
            if (current_class->methods[i]->ident_str() == ident) {
                result.type = lookup_type::mtd_method;
                result.index = i;
                return result;
            }
        }

		callinfo ci;
		if (syscalls.try_get(ident, ci)) {
			result.type = lookup_type::mtd_syscall;
			result.index = ci.entry;
			return result;
		}

        result.type = lookup_type::not_exist;
        return result;
    }
    lookup_result lookup_variable(const std::string &ident) {
        lookup_result result;
        
        for (int i = 0; i<current_method->locals.size(); i++) {
            if (current_method->locals[i] == ident) {
                result.type = lookup_type::var_local;
                result.index = i;
                return result;
            }
        }

        for (int i=0;i<current_class->fields.size();i++) {
            if (current_class->fields[i]->ident_str() == ident) {
                result.type = lookup_type::var_field;
                result.index = i;
                return result;
            }
        }

        result.type = lookup_type::not_exist;
        return result;
    }

private:
	calltable_builder &syscalls;

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
        if (pool.empty())
            return "";
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
    void emit(opcode_t opcode, const callsite &cs) {
        printf("[emit] %d\n", opcode);
        instructions.push_back(instruction(opcode, cs));
    }
    void emit(opcode_t opcode, int operand) {
        printf("[emit] %d\n", opcode);
        instructions.push_back(instruction(opcode, operand));
    }
    int emit(opcode_t opcode) {
        printf("[emit] %d\n", opcode);
        instructions.push_back(instruction(opcode, 0));
        return instructions.size() - 1;
    }

    void modify_operand(int i, int operand) {
        instructions[i].operand = operand;
    }

    void emit_class(const std::string &name) {

    }
    void emit_method(const std::string &classname, method_node *method) {
        program_entry entry;

        printf(" emitmethod %s::%s\n", classname.c_str(), method->ident_str().c_str());

        memset(&entry, 0, sizeof(program_entry));
        //sprintf_s(entry.signature, "%s::%s", classname.c_str(), method->ident_str().c_str());
		sprintf_s(entry.signature, "%s", method->ident_str().c_str());
        entry.entry = get_cursor();
        entry.locals = method->locals.size();
        entries.push_back(entry);
    }
    void fin_method() {
        auto &last_entry = entries[entries.size() - 1];
        last_entry.codesize = get_cursor() - last_entry.entry;
    }

    int get_cursor() const {
        return instructions.size();
    }

    program fin() {
        program p;
        memset(&p, 0, sizeof(program));
        p.header.code_len = instructions.size();
        p.header.rdata_len = spool.size();
        p.header.entry_len = entries.size();
        p.code = &instructions[0];

		auto rdata = spool.fin();

		if (spool.size() > 0) {
			p.rdata = (char*)malloc(sizeof(char) * spool.size());
			memcpy((char*)p.rdata, rdata, sizeof(char) * spool.size());
		}
        if (instructions.size() > 0) {
            p.code = (instruction*)malloc(sizeof(instruction) * instructions.size());
            memcpy(p.code, &instructions[0], sizeof(instruction) * instructions.size());
        }
        if (entries.size() > 0) {
            p.entries = (program_entry*)malloc(sizeof(program_entry) * entries.size());
            memcpy(p.entries, &entries[0], sizeof(program_entry) * entries.size());
        }

        return p;
    }

private:
    string_pool spool;

    std::vector<program_entry> entries;
    std::vector<instruction> instructions;
};

class code_gen {
public:
    code_gen(compile_context &ctx, calltable_builder &syscalls) :
		scope(syscalls),
		ctx(ctx) {
    }

    program generate(root_node *root) {
        emitter = program_builder();

        emit(root);

        return emitter.fin();
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

        printf("%s %d, %d\n", typeid(*node).name(), node->type, node->token().line);
        switch (node->type) {
            _route(root);
            _route(class);
            _route(method);
            _route(call);
            _route(return);
            _route(block);
            _route(ident);
            _route(literal);
            _route(op);
            _route(assignment);
            _route(if);
            _route(for);
        }
    }

    void emit_root(root_node *node) {
        for (int i = 0; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_class(class_node *node) {
        current_class = node;
        scope.set_class(node);
        emitter.emit_class(node->ident_str());

        for (int i = 1; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_method(method_node *node) {
        current_method = node;
        scope.set_method(node);
        emitter.emit_method(current_class->ident_str(), node);

        emit(node->body());

        emitter.emit(opcode::op_ret);
        emitter.fin_method();
    }
    void emit_call(call_node *node) {
		for (auto it = node->begin_args(); it != node->end_args(); ++it)
			emit(*it);

        auto target = node->calltarget();
        if (target->type == syntax_type::syn_ident) {
            auto lookup = scope.lookup_method(node->ident_str());

			if (lookup.type == lookup_type::not_exist) {
				ctx.push_error(undeclared_method_error(node->token(), node->ident_str()));
				return;
			}
            else if (lookup.type == lookup_type::mtd_method)
                emitter.emit(opcode::op_call, callsite(callsite_lookup::cs_method, lookup.index));
            else if (lookup.type == lookup_type::mtd_syscall)
                emitter.emit(opcode::op_call, callsite(callsite_lookup::cs_syscall, lookup.index));
        }
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
    void emit_ident(ident_node *node) {
        auto lookup = scope.lookup_variable(node->ident);
        if (lookup.type == lookup_type::not_exist) {
			ctx.push_error(undefined_variable_error(node->token()));
            return;
        }

        if (lookup.type == lookup_type::var_local)
            emitter.emit(opcode::op_ldloc, lookup.index);
        else if (lookup.type == lookup_type::var_field)
            emitter.emit(opcode::op_ldstate, lookup.index);
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
			ctx.push_error(syntax_error(node->left()->token(), "Wrong l-value type."));
            return;
        }

        auto lookup = scope.lookup_variable(ident->ident);
        if (lookup.type == lookup_type::not_exist) {
            return;
        }

        if (lookup.type == lookup_type::var_local)
            emitter.emit(opcode::op_stloc, lookup.index);
        else if (lookup.type == lookup_type::var_field)
            emitter.emit(opcode::op_ststate, lookup.index);
    }
    void emit_if(if_node *node) {
        emit(node->cond());
        int jmp = emitter.emit(opcode::op_jmp_false);
        emit(node->then());
        emitter.modify_operand(jmp, emitter.get_cursor());
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

    class_node *current_class;
    method_node *current_method;
};