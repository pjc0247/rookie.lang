#pragma once

#include <map>
#include <set>

#include "string_pool.h"
#include "syntax.h"
#include "program.h"
#include "compilation.h"
#include "binding.h"
#include "errors.h"
#include "type_attr.h"

#include "ast/ast_reflection.h"

#include "sig2hash.h"

enum class lookup_type {
    not_exist,

    var_local,
    var_field,
    var_typename,

    mtd_method,
    mtd_syscall
};
struct lookup_result {
    lookup_type type;
    int index;
};

class scope {
public:
    scope(compile_context &ctx, calltable_builder &syscalls) :
        ctx(ctx),
        syscalls(syscalls) {
    }

    void set_class(class_node *node) {
        current_class = node;
    }
    void set_method(method_node *node) {
        current_method = node;
    }

    lookup_result lookup_method(const std::wstring &ident) {
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
    lookup_result lookup_variable(const std::wstring &ident) {
        lookup_result result;

        for (auto _type : ctx.types) {
            auto type = _type.second;

            if (type.name == ident) {
                result.type = lookup_type::var_typename;
                return result;
            }
        }

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
    compile_context &ctx;

    calltable_builder &syscalls;

    class_node *current_class;
    method_node *current_method;
};

class program_builder {
public:
    program_builder(compile_context &ctx) :
        ctx(ctx) {
    }

    void set_codeindex(unsigned int index) {
        codeindex = index;
    }

    void emit(opcode_t opcode, const std::wstring &operand) {
        emit(opcode, spool.get_ptr(operand));
    }
    void emit(opcode_t opcode, const callsite &cs) {
        instructions.push_back(instruction(opcode, cs));
        instruction_indexes.push_back(codeindex);
    }
    void emit_defer(opcode_t opcode, const callsite &cs, const std::wstring &signature) {
        instructions.push_back(instruction(opcode, cs));
        instruction_indexes.push_back(codeindex);

        defered_calls.push_back(signature);
    }
    void emit(opcode_t opcode, int operand) {
        instructions.push_back(instruction(opcode, operand));
        instruction_indexes.push_back(codeindex);
    }
    int emit(opcode_t opcode) {
        instructions.push_back(instruction(opcode, 0));
        instruction_indexes.push_back(codeindex);
        return instructions.size() - 1;
    }

    void modify_operand(int i, int operand) {
        instructions[i].operand = operand;
    }

    void emit_class(class_node *_class) {
        current_class = _class;
    }
    void emit_method(method_node *method) {
        printf("EMIT %S\n", method->ident_str().c_str());
        compiletime_methoddata mdata;
        mdata.name = method->ident_str();
        mdata.entry = entries.size();
        for (auto &_type : ctx.types) {
            auto &type = _type.second;

            if (type.name == current_class->ident_str()) {
                for (auto &_method : type.methods) {
                    if (_method.name == method->ident_str())
                        _method.entry = entries.size();
                }
                break;
            }
        }

        program_entry entry;
        memset(&entry, 0, sizeof(program_entry));
        swprintf(entry.signature, sizeof(entry.signature), L"%s::%s", current_class->ident_str().c_str(), method->ident_str().c_str());
        entry.entry = get_cursor();
        entry.params = method->params()->children.size();
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

    program *fin() {
        resolve_defered_calls();

        program *_p = new program();
        program &p = *_p;
        auto types = ctx.get_programtypes();

        memset(&p, 0, sizeof(program));
        p.header.code_len = instructions.size();
        p.header.rdata_len = spool.size();
        p.header.entry_len = entries.size();
        p.header.types_len = types.size();

        p.header.main_entry = 0;

        auto rdata = spool.fin();

        if (ctx.types.size() > 0) {
            p.types = (typedata*)malloc(sizeof(typedata) * types.size());
            int i = 0;
            for (auto &type : types) {
                wcscpy(p.types[i].name, type.name.c_str());
                p.types[i].methods_len = type.methods.size();
                p.types[i].methods = (methoddata*)malloc(sizeof(methoddata) * type.methods.size());
                
                for (int j = 0; j < type.methods.size(); j++) {
                    auto method = type.methods[j];

                    wcscpy(p.types[i].methods[j].name, method.name.c_str());
                    p.types[i].methods[j].entry = method.entry;
                }

                i++;
            }
        }
        if (spool.size() > 0) {
            p.rdata = (wchar_t*)malloc(sizeof(wchar_t) * spool.size());
            memcpy((wchar_t*)p.rdata, rdata, sizeof(wchar_t) * spool.size());
        }
        if (instructions.size() > 0) {
            p.code = (instruction*)malloc(sizeof(instruction) * instructions.size());
            memcpy(p.code, &instructions[0], sizeof(instruction) * instructions.size());
        }
        if (entries.size() > 0) {
            p.entries = (program_entry*)malloc(sizeof(program_entry) * entries.size());
            memcpy(p.entries, &entries[0], sizeof(program_entry) * entries.size());
        }

        return _p;
    }

    pdb *generate_pdb(string_pool &code, binding &bindings) {
        pdb *_p = new pdb();
        pdb &p = *_p;
        std::vector<pdb_signature> sigs;

        for (auto &func : bindings.get_functions()) {
            sigs.push_back(pdb_signature(sig2hash(func.first), func.first.c_str()));
        }
        for (auto &type : bindings.get_types()) {
            sigs.push_back(pdb_signature(sig2hash(type.get_name()), type.get_name().c_str()));

            for (auto &method : type.get_methods()) {
                sigs.push_back(pdb_signature(sig2hash(method.first), method.first.c_str()));
            }
        }
        for (auto &type : ctx.types) {
            auto type_name = type.first;

            sigs.push_back(pdb_signature(sig2hash(type_name), type_name.c_str()));

            for (auto &method : type.second.methods) {
                sigs.push_back(pdb_signature(sig2hash(method.name), method.name.c_str()));
            }
        }

        p.sigtable = new pdb_signature[sigs.size()];
        p.sigtable_len = sigs.size();
        memcpy(p.sigtable, &sigs[0], sizeof(pdb_signature) * sigs.size());

        p.inst_data = new pdb_instruction[instructions.size()];
        p.inst_data_len = instructions.size();
        for (int i = 0; i < instructions.size(); i++) {
            p.inst_data[i].codeindex = instruction_indexes[i];
        }

        p.code = new wchar_t[code.size()];
        p.code_len = code.size();
        memcpy(p.code, code.fin(), sizeof(wchar_t) * code.size());

        return _p;
    }

private:
    void resolve_defered_calls() {
        rklog("resolve defered calls\n");

        for (auto &inst : instructions) {
            if (inst.opcode == opcode::op_call) {
                if (inst.cs.flags & callsite_flag::cf_defer) {
                    auto method_name = defered_calls[inst.cs.index];

                    auto eidx = find_entry(method_name);
                    if (eidx == -1)
                        ctx.push_error(codegen_error(L"Unresolved name: " + method_name));
                    inst.cs.index = eidx;

                    end_loop :;
                }
            }
        }
    }

    int find_entry(const std::wstring &signature) {
        for (int i = 0; i<entries.size(); i++) {
            if (signature == entries[i].signature)
                return i;
        }
        return -1;
    }

private:
    compile_context &ctx;

    string_pool spool;

    class_node *current_class;

    std::vector<program_entry> entries;
    std::vector<instruction> instructions;
    std::vector<unsigned int> instruction_indexes;

    std::vector<std::wstring> defered_calls;

    unsigned int codeindex;
};

class code_gen {
public:
    code_gen(compile_context &ctx, calltable_builder &syscalls) :
        scope(ctx, syscalls),
        ctx(ctx),
        emitter(ctx) {
    }

    program *generate(root_node *root) {
        auto classes = astr::all_classes(ctx.root_node);

        for (auto _class : ctx.bindings.get_types()) {
            compiletime_typedata tdata;
            tdata.name = _class.get_name();
            tdata.attr = class_attr::class_systype;
            for (auto _method : _class.get_methods()) {
                compiletime_methoddata mdata;
                mdata.name = _method.first;
                mdata.attr = method_attr::method_syscall;
                tdata.methods.push_back(mdata);
            }
            ctx.types[tdata.name] = tdata;
        }

        for (auto _class : classes) {
            compiletime_typedata tdata;
            tdata.name = _class->ident_str();
            for (auto _method : _class->methods) {
                compiletime_methoddata mdata;
                mdata.name = _method->ident_str();
                tdata.methods.push_back(mdata);
            }
            ctx.types[tdata.name] = tdata;
        }

        auto m = astr::find_method_with_annotation(root, L"main");
        if (m.size() == 0)
            ctx.push_error(codegen_error(L"No entry for `main`"));
        else if (m.size() >= 2)
            ctx.push_error(codegen_error(L"More than 2 entries for `main`"));
        else if (!(m[0]->attr & method_attr::method_static))
            ctx.push_error(codegen_error(L"`main` is not a static method."));

        emit(root);

        return emitter.fin();
    }
    pdb *generate_pdb(binding &bindings) {
        if (ctx.opts.generate_pdb == false)
            throw codegen_exception("generate_pdb == false");

        return emitter.generate_pdb(ctx.code, bindings);
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

        emitter.set_codeindex(node->token().dbg_codeidx);

        rklog("%s %d, %d\n", typeid(*node).name(), node->type, node->token().line);
        switch (node->type) {
            _route(root);
            _route(class);
            _route(method);
            _route(pop);
            _route(callmember);
            _route(call);
            _route(return);
            _route(block);
            _route(this);
            _route(memberaccess);
            _route(ident);
            _route(literal);
            _route(null);
            _route(bool);
            _route(newobj);
            _route(newarr);
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
    void emit_pop(pop_node *pop) {
        emitter.emit(opcode::op_pop);
    }
    void emit_class(class_node *node) {
        current_class = node;
        scope.set_class(node);
        emitter.emit_class(node);

        for (int i = 1; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_method(method_node *node) {
        current_method = node;
        scope.set_method(node);
        emitter.emit_method(node);

        emit(node->body());

        emitter.emit(opcode::op_ldnull);
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
                emitter.emit(opcode::op_syscall, callsite(callsite_lookup::cs_syscall, lookup.index));
        }
    }
    void emit_callstatic(callmember_node *node) {
        auto callee = (ident_node*)node->children[1];
        auto callee_name = callee->ident;
        if (ctx.types.find(callee_name) == ctx.types.end())
            ; // ???
        auto &callee_type = (*ctx.types.find(callee_name)).second;

        for (auto it = std::next(node->begin_args()); it != node->end_args(); ++it)
            emit(*it);

        if (callee_type.attr & class_attr::class_systype) {
            auto signature = callee_name + L"::" + node->ident_str();
            auto lookup = scope.lookup_method(signature);

            emitter.emit(opcode::op_syscall,
                callsite(callsite_lookup::cs_method, lookup.index));
        }
        else {
            emitter.emit_defer(opcode::op_call,
                callsite(callsite_lookup::cs_method, 1, 0),
                callee_name + L"::" + node->ident_str());
        }
    }
    void emit_callmember(callmember_node *node) {
        auto callee = (ident_node*)node->children[1];
        auto lookup = scope.lookup_variable(callee->ident);

        if (lookup.type == lookup_type::var_typename) {
            emit_callstatic(node);
            return;
        }

        if (node->children[1]->type == syntax_type::syn_ident &&
            ((ident_node*)node->children[1])->ident == L"this") {

            if (current_method->attr & method_attr::method_static)
                ctx.push_error(syntax_error(node, L"invalid this inside static method."));

            emitter.emit(opcode::op_ldthis);
        }
        else
            emit(*node->begin_args());

        emitter.emit(opcode::op_setcallee);

        for (auto it = std::next(node->begin_args()); it != node->end_args(); ++it)
            emit(*it);

        /*
        auto target = node->calltarget();
        if (target->type == syntax_type::syn_ident) {
            auto lookup = scope.lookup_method(node->ident_str());

            if (lookup.type == lookup_type::not_exist ||
                lookup.type == lookup_type::mtd_syscall) {
                ctx.push_error(undeclared_method_error(node->token(), node->ident_str()));
                return;
            }
            else if (lookup.type == lookup_type::mtd_method)
                
        }*/
        emitter.emit(opcode::op_vcall, sig2hash(node->ident_str()));
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
    void emit_this(this_node *node) {
        if (current_method->attr & method_attr::method_static)
            ctx.push_error(syntax_error(node, L"invalid this inside static method."));

        emitter.emit(opcode::op_ldloc, 0);
    }
    void emit_memberaccess(memberaccess_node *node) {
        if (node->children[0]->type == syntax_type::syn_ident &&
            ((ident_node*)node->children[0])->ident == L"this") {

            if (current_method->attr & method_attr::method_static)
                ctx.push_error(syntax_error(node, L"invalid this inside static method."));
            emitter.emit(opcode::op_ldthis);
        }
        else
            emit(node->children[0]);

        emitter.emit(opcode::op_ldprop, sig2hash(node->property_name()));
    }
    void emit_ident(ident_node *node) {
        if (node->ident == L"this") {
            emitter.emit(opcode::op_ldthis);
            return;
        }

        auto lookup = scope.lookup_variable(node->ident);
        if (lookup.type == lookup_type::not_exist) {
            //ctx.push_error(undefined_variable_error(node->token()));
        }

        if (lookup.type == lookup_type::var_local)
            emitter.emit(opcode::op_ldloc, lookup.index);
        else //if (lookup.type == lookup_type::var_field)
            emitter.emit(opcode::op_ldprop, lookup.index);
    }
    void emit_literal(literal_node *node) {
        if (node->literal_type == literal_type::integer)
            emitter.emit(opcode::op_ldi, node->integer);
        else if (node->literal_type == literal_type::string)
            emitter.emit(opcode::op_ldstr, node->str);
    }
    void emit_bool(bool_node *node) {
        if (node->value)
            emitter.emit(opcode::op_ldtrue);
        else 
            emitter.emit(opcode::op_ldfalse);
    }
    void emit_null(null_node *node) {
        emitter.emit(opcode::op_ldnull);
    }
    void emit_newobj(newobj_node *node) {
        for (auto it = node->begin_args(); it != node->end_args(); ++it)
            emit(*it);
        emitter.emit(opcode::op_newobj, sig2hash(node->ident_str()));
    }
    void emit_newarr(newarr_node *node) {
        for (int i=node->children.size()-1; i>=0;i--)
            emit(node->children[i]);
        emitter.emit(opcode::op_newarr, node->children.size());
    }
    void emit_op(op_node *node) {
        emit(node->left());
        emit(node->right());

        if (node->op == L"+")
            emitter.emit(opcode::op_add);
        else if (node->op == L"-")
            emitter.emit(opcode::op_sub);
        else if (node->op == L"*")
            emitter.emit(opcode::op_mul);
        else if (node->op == L"/")
            emitter.emit(opcode::op_div);
        else if (node->op == L">")
            emitter.emit(opcode::op_g);
        else if (node->op == L"<")
            emitter.emit(opcode::op_l);
        else if (node->op == L">=")
            emitter.emit(opcode::op_ge);
        else if (node->op == L"<=")
            emitter.emit(opcode::op_le);
        else if (node->op == L"==")
            emitter.emit(opcode::op_eq);
    }
    void emit_assignment(assignment_node *node) {
        emit(node->right());

        auto ident = dynamic_cast<ident_node*>(node->left());
        if (ident != nullptr) {
            auto lookup = scope.lookup_variable(ident->ident);
            if (lookup.type == lookup_type::not_exist) {
                ctx.push_error(undefined_variable_error(ident->token()));
                return;
            }

            if (lookup.type == lookup_type::var_local)
                emitter.emit(opcode::op_stloc, lookup.index);
            else if (lookup.type == lookup_type::var_field)
                emitter.emit(opcode::op_stprop, lookup.index);

            return;
        }

        auto memberaccess = dynamic_cast<memberaccess_node*>(node->left());
        if (memberaccess != nullptr) {
            emit(memberaccess->children[0]);
            emitter.emit(opcode::op_stprop, sig2hash(memberaccess->property_name()));
            return;
        }

        ctx.push_error(syntax_error(node->left()->token(), L"Wrong l-value type."));
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