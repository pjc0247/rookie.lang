#pragma once

#include <map>
#include <set>

#include "string_pool.h"
#include "syntax/syntax_node.h"
#include "program.h"
#include "compilation.h"
#include "binding.h"
#include "errors.h"
#include "type_attr.h"
#include "ir_optimiser.h"
#include "lookup_builder.h"

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

    method_node *method;
    callinfo callinfo;

    lookup_result() :
        type(lookup_type::not_exist),
        method(nullptr) {
    }
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

    void push_block(block_node *node) {
        scope_stack.push_back(node);
    }
    void pop_block() {
        scope_stack.pop_back();
    }

    bool is_typename(const std::wstring &ident) {
        for (auto _type : ctx.types) {
            auto type = _type.second;

            if (type.name == ident)
                return true;
        }
        return false;
    }

    lookup_result lookup_method(const std::wstring &ident) {
        lookup_result result;

        for (uint32_t i = 0; i<current_class->methods.size(); i++) {
            if (current_class->methods[i]->ident_str() == ident) {
                result.type = lookup_type::mtd_method;
                result.index = i;
                result.method = current_class->methods[i];
                return result;
            }
        }

        callinfo ci;
        if (syscalls.try_get(ident, ci)) {
            result.type = lookup_type::mtd_syscall;
            result.index = ci.entry;
            result.callinfo = ci;
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

        int offset = 0;
        for (auto block : scope_stack) {
            for (uint32_t i = 0; i<block->locals.size(); i++) {
                if (block->locals[i] == ident) {
                    result.type = lookup_type::var_local;
                    result.index = i + offset;
                    return result;
                }
            }

            offset += block->locals.size();
        }

        for (uint32_t i=0;i<current_class->fields.size();i++) {
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

    std::deque<block_node*> scope_stack;
};

class program_builder {
public:
    program_builder(compile_context &ctx) :
        ctx(ctx), ep(0) {
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
    void emit(opcode_t opcode, const callsite &cs, uint8_t params) {
        instructions.push_back(instruction(opcode, cs, params));
        instruction_indexes.push_back(codeindex);
    }
    // Don't know exact callsite yet, but will be resolved later.
    void emit_defer(opcode_t opcode, const std::wstring &signature, int args) {
        auto cs = callsite(args, 1, 0);
        cs.index = defered_calls.size();
        instructions.push_back(instruction(opcode, cs, args));
        instruction_indexes.push_back(codeindex);

        defered_calls.push_back(signature);
    }
    void emit(opcode_t opcode, int operand) {
        instructions.push_back(instruction(opcode, operand));
        instruction_indexes.push_back(codeindex);
    }
    void emit(opcode_t opcode, int operand, uint8_t operand2) {
        instructions.push_back(instruction(opcode, operand, operand2));
        instruction_indexes.push_back(codeindex);
    }
    void emit_f(opcode_t opcode, float operand) {
        auto inst = instruction(opcode, operand);
        inst.f32 = operand;
        instructions.push_back(inst);
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
    void modify_operand(int i, const callsite &cs) {
        instructions[i].cs = cs;
    }

    void emit_class(class_node *_class) {
        current_class = _class;
    }
    void emit_method(method_node *method) {
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
        swprintf(entry.signature, sizeof(entry.signature), L"%ls::%ls", current_class->ident_str().c_str(), method->ident_str().c_str());
        entry.entry = get_cursor();
        entry.params = method->params()->children.size();
        entry.locals = method->local_size;
        entry.flags = method->is_va_args() ? rk_entry_va_args : 0;
        entries.push_back(entry);
    }
    void begin_bootstrap() {
        assert(entries.size() == 0);

        program_entry entry;
        memset(&entry, 0, sizeof(program_entry));
        swprintf(entry.signature, sizeof(entry.signature), L"_rkboot");
        entry.entry = get_cursor();
        entry.params = 0;
        entry.locals = 0;
        entries.push_back(entry);
    }

    void begin_try() {
        exception_handler eh;
        eh.pc_begin = get_cursor();
        exception_handlers.push_back(eh);
        ep++;
    }
    void end_try() {
        exception_handlers[ep-1].pc_end = get_cursor();
        ep--;

        // TODO
        emit(opcode::op_jmp);
    }
    void begin_catch() {
        exception_handlers[ep]._catch = get_cursor();
    }
    void end_catch() {
        auto jmp_ptr = exception_handlers[ep]._catch - 1;
        modify_operand(jmp_ptr, get_cursor());
    }

    void fin_method() {
        auto &last_entry = entries[entries.size() - 1];
        last_entry.codesize = get_cursor() - last_entry.entry;
    }

    int get_cursor() const {
        return instructions.size();
    }

    program *fin(method_node *main_node) {
        resolve_defered_calls();

        //auto iro = ir_optimiser(instructions);
        //instructions = iro.transform();

        program *_p = new program();
        program &p = *_p;
        auto types = ctx.get_programtypes();

        auto root = main_node->root;
        lookup_builder lb;
        auto ids = lb.build(root);

        memset(&p, 0, sizeof(program));
        p.header.code_len = instructions.size();
        p.header.rdata_len = spool.size();
        p.header.entry_len = entries.size();
        p.header.exception_handler_len = exception_handlers.size();
        p.header.types_len = types.size();
        p.header.lookup_len = ids.size();

        p.header.main_entry = 0;

        auto jmp_entry = 0;
        for (uint32_t i = 0; i < entries.size(); i++) {
            auto sig = main_node->declaring_class()->ident_str() + L"::" + main_node->ident_str();
            if (sig == entries[i].signature) {
                jmp_entry = i;
                break;
            }
        }
        modify_operand(entries[1].entry - 3, callsite(callsite_lookup::cs_method, jmp_entry));

        auto rdata = spool.fin();

        if (ctx.types.size() > 0) {
            p.types = (typedata*)malloc(sizeof(typedata) * types.size());
            int i = 0;
            for (auto &type : types) {
                wcscpy(p.types[i].name, type.name.c_str());
                p.types[i].methods_len = type.methods.size();
                p.types[i].methods = (methoddata*)malloc(sizeof(methoddata) * type.methods.size());
                
                for (uint32_t j = 0; j < type.methods.size(); j++) {
                    auto method = type.methods[j];

                    wcscpy(p.types[i].methods[j].name, method.name.c_str());
                    p.types[i].methods[j].entry = method.entry;
                }

                p.types[i].parents_len = type.parents.size();
                if (type.parents.size() > 0) {
                    p.types[i].parents = (uint32_t*)malloc(sizeof(uint32_t) * type.parents.size());
                    int offset = 0;
                    for (auto &parent : type.parents) {
                        p.types[i].parents[offset] = sig2hash(parent);
                        offset++;
                    }
                }

                i++;
            }
        }
        if (exception_handlers.size() > 0) {
            p.exception_handlers = (exception_handler*)malloc(sizeof(exception_handler) * exception_handlers.size());
            memcpy((wchar_t*)p.exception_handlers, &exception_handlers[0], sizeof(exception_handler) * exception_handlers.size());
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
        if (ids.size() > 0) {
            p.lookups = (wchar_t*)malloc(sizeof(wchar_t) * ids.size());
            memcpy((wchar_t*)p.lookups, ids.fin(), sizeof(wchar_t) * ids.size());
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
        for (uint32_t i = 0; i < instructions.size(); i++) {
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

                    rklog("Resolve %S %d\n", method_name.c_str(), eidx);
                }
            }
        }
    }

    int find_entry(const std::wstring &signature) {
        for (uint32_t i = 0; i<entries.size(); i++) {
            if (signature == entries[i].signature)
                return i;
        }
        return -1;
    }

private:
    compile_context &ctx;

    string_pool spool;

    class_node *current_class;

    uint32_t ep;

    std::vector<program_entry> entries;
    std::vector<exception_handler> exception_handlers;
    std::vector<instruction>   instructions;
    std::vector<unsigned int>  instruction_indexes;

    std::vector<std::wstring>  defered_calls;

    unsigned int codeindex;
};

class code_gen {
public:
    code_gen(compile_context &ctx, calltable_builder &syscalls) :
        scope(ctx, syscalls),
        ctx(ctx),
        emitter(ctx) {
    }

    program *generate(root_node *_root) {
        root = _root;
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
            auto parents = _class->parents();
            if (parents != nullptr) {
                for (auto _p : parents->children) {
                    auto p = (ident_node*)_p;
                    tdata.parents.push_back(p->ident);
                }
            }

            ctx.types[tdata.name] = tdata;
        }

        auto m = astr::find_method_with_annotation(root, L"main");
        if (m.size() == 0)
            ctx.push_error(codegen_error(L"No entrypoint found."));
        else if (m.size() >= 2)
            ctx.push_error(codegen_error(L"More than 2 entries for `main`"));
        else if (!(m[0]->attr & method_attr::method_static))
            ctx.push_error(codegen_error(L"`main` is not a static method."));

        emit_bootstrap(root);
        emit(root);

        if (m.size() == 0)
            return nullptr;
        return emitter.fin(m[0]);
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

    void emit_bootstrap(syntax_node *node) {
        emitter.begin_bootstrap();

        for (auto klass : astr::all_classes(node)) {
            for (auto field : klass->fields) {
                emitter.emit(opcode::op_ldtype, sig2hash(klass->ident_str()));
                emit(field->children[1]);
                emitter.emit(opcode::op_stfld, sig2hash(field->ident_str()));
            }
        }

        emitter.emit(opcode::op_call, 0);
        emitter.emit(opcode::op_nop);
        emitter.emit(opcode::op_ret);
        emitter.fin_method();
    }
    void emit(syntax_node *node) {
        if (node == nullptr) return;

        if (node->is_virtual)
            ; // incomplete vnode transformation

        emitter.set_codeindex(node->token().dbg_codeidx);

        rklog("%s %d, %d\n", typeid(*node).name(), node->type, node->token().line);
        switch (node->type) {
            _route(root);
            _route(class);
            _route(field);
            _route(method);
            _route(pop);
            _route(callmember);
            _route(call);
            _route(return);
            _route(try);
            _route(catch);
            _route(block);
            _route(this);
            _route(memberaccess);
            _route(ident);
            _route(literal);
            _route(null);
            _route(bool);
            _route(newobj);
            _route(newarr);
            _route(newdic);
            _route(op);
            _route(assignment);
            _route(if);
            _route(else);
            _route(throw);
            _route(for);
            _route(foreach);
            _route(while);
        }
    }

    void emit_root(root_node *node) {
        for (uint32_t i = 0; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_pop(pop_node *pop) {
        emitter.emit(opcode::op_pop);
    }
    void emit_class(class_node *node) {
        current_class = node;
        scope.set_class(node);
        emitter.emit_class(node);

        for (uint32_t i = 1; i < node->children.size(); i++)
            emit(node->children[i]);
    }
    void emit_field(field_node *node) {

    }
    void emit_method(method_node *node) {
        current_method = node;
        scope.set_method(node);
        emitter.emit_method(node);

        // Default params
        for (auto p : node->params()->children) {
            if (p->type == syntax_type::syn_assignment) {
                auto id = (ident_node*)p->children[0];
				scope.push_block(node->body());
                auto lookup = scope.lookup_variable(id->ident);
				scope.pop_block();

                emitter.emit(opcode::op_ldloc, lookup.index);
                emitter.emit(opcode::op_ldempty);
                emitter.emit(opcode::op_eq);
                auto jmp_false = emitter.emit(opcode::op_jmp_false);

                emit(p->children[1]);
                emitter.emit(opcode::op_stloc, lookup.index);

                emitter.modify_operand(jmp_false, emitter.get_cursor());
            }
        }

        if (node->is_va_args()) {
            emitter.emit(opcode::op_param_to_arr);
            emitter.emit(opcode::op_dup);
            emitter.emit(opcode::op_stloc, node->params()->children.size() - 1);
        }

        emit(node->body());

        emitter.emit(opcode::op_ldnull);
        emitter.emit(opcode::op_ret);
        emitter.fin_method();
    }

    /*
    void emit_callpadding(call_node *caller, method_node *callee) {
        if (callee == nullptr) return;

        auto params_required = callee->params()->children.size();
        auto params_given = (caller->children.size() - 1);

        if (params_required < params_given)
                return;

        auto arg_diff = params_required - params_given;

        for (int i = 0; i < arg_diff; i++)
            emitter.emit(opcode::op_ldempty);
    }
    */

    void emit_callsuper(call_node *node) {
        auto method_name = node->declaring_method()->ident_str();
        auto parents = node->declaring_class()->parents();

        std::deque<class_node*> p;

        if (parents != nullptr) {
            for (auto _parent_ident : parents->children) {
                auto parent_ident = (ident_node*)_parent_ident;
                p.push_back(astr::find_class(root, parent_ident->ident));
            }
        }

        std::vector<method_node *> supers;
        while (p.empty() == false) {
            auto c = p.back();
            p.pop_back();

            for (auto method : c->methods) {
                if (method->ident_str() == method_name) {
                    supers.push_back(method);
                    goto end_loop;
                }
            }

            if (c->parents() == nullptr)
                continue;

            for (auto _parent_ident : c->parents()->children) {
                auto parent_ident = (ident_node*)_parent_ident;
                p.push_back(astr::find_class(root, parent_ident->ident));
            }

        end_loop:;
        }

        if (supers.size() == 1) {
            emitter.emit_defer(opcode::op_call,
                supers[0]->declaring_class()->ident_str() + L"::" + supers[0]->ident_str(),
                node->args());
        }
        else if (supers.size() >= 2)
            ctx.push_error(codegen_error(method_name + L" has more than 2 supercalls."));
        else
            ctx.push_error(codegen_error(method_name + L" doesn't have a super method."));
    }
    void emit_call(call_node *node) {
        for (auto it = node->begin_args(); it != node->end_args(); ++it)
            emit(*it);

        if (node->ident_str() == L"super") {
            emit_callsuper(node);
            return;
        }

        auto lookup = scope.lookup_method(node->ident_str());
        
        //if (lookup.type == lookup_type::not_exist) {
            //ctx.push_error(undeclared_method_error(node->token(), node->ident_str()));
        //    return;
        //}
        if (lookup.type == lookup_type::mtd_syscall)
            emitter.emit(opcode::op_syscall, callsite(node->args(), lookup.index));
        else {
            if (lookup.method != nullptr &&
                lookup.method->attr & method_attr::method_static) {

                emitter.emit_defer(opcode::op_call,
                    node->declaring_class()->ident_str() + L"::" + node->ident_str(),
                    node->args());
            }
            else {
                if (current_method->attr & method_attr::method_static)
                    ctx.push_error(codegen_error(L"Cannot call non-static method inside static-method."));

                emitter.emit(opcode::op_ldthis);
                emitter.emit(opcode::op_setcallee);

                emitter.emit(opcode::op_vcall, sig2hash(node->ident_str()), node->args());
            }
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
                callsite(node->args(), lookup.index),
                node->args());
        }
        else {
            emitter.emit_defer(opcode::op_call,
                callee_name + L"::" + node->ident_str(),
                node->args());
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
        emitter.emit(opcode::op_vcall, sig2hash(node->ident_str()), node->args());
    }
    void emit_return(return_node *node) {
        auto val = node->value();
        if (val != nullptr)
            emit(val);
        emitter.emit(opcode::op_ret);
    }
    void emit_try(try_node *node) {
        emitter.begin_try();
        emit(node->children[0]);
        emitter.end_try();
    }
    void emit_catch(catch_node *node) {
        emitter.begin_catch();

        scope.push_block(node->block());
        auto lookup = scope.lookup_variable(node->exception()->ident);
        emitter.emit(opcode::op_stloc, lookup.index);
        scope.pop_block();

        emit(node->block());
        emitter.end_catch();
    }
    void emit_block(block_node *node) {
        scope.push_block(node);

        for (auto child : node->children)
            emit(child);
        emitter.emit(opcode::op_nop);

        scope.pop_block();
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

        do {
            if (node->children[0]->type == syntax_type::syn_ident) {
                auto id = ((ident_node*)node->children[0])->ident;

                if (scope.is_typename(id)) {
                    emitter.emit(opcode::op_ldfld, sig2hash(node->property_name()));
                    break;
                }
            }
            emitter.emit(opcode::op_ldprop, sig2hash(node->property_name()));
        } while (false);
    }
    void emit_ident(ident_node *node) {
        if (node->ident == L"this") {
            emitter.emit(opcode::op_ldthis);
            return;
        }

        auto lookup = scope.lookup_variable(node->ident);

        if (node->ident[0] == L'@') {
            emitter.emit(opcode::op_ldthis);
            emitter.emit(opcode::op_ldprop, sig2hash(node->ident, 1));
        }
        else {
            if (lookup.type == lookup_type::not_exist) {
                ctx.push_error(codegen_error(L"Unassigned local variable: " + node->ident));
            }
            else if (lookup.type == lookup_type::var_typename)
                emitter.emit(opcode::op_ldtype, sig2hash(node->ident));
            else if (lookup.type == lookup_type::var_local)
                emitter.emit(opcode::op_ldloc, lookup.index);
            else if (lookup.type == lookup_type::var_field) {
                emitter.emit(opcode::op_ldtype, sig2hash(current_class->ident_str()));
                emitter.emit(opcode::op_ldfld, sig2hash(node->ident));
            }
        }
    }
    void emit_literal(literal_node *node) {
        if (node->literal_type == literal_type::boolean) {
            if (node->integer == 1)
                emitter.emit(opcode::op_ldtrue);
            else
                emitter.emit(opcode::op_ldfalse);
        }
        else if (node->literal_type == literal_type::integer)
            emitter.emit(opcode::op_ldi, node->integer);
        else if (node->literal_type == literal_type::decimal)
            emitter.emit_f(opcode::op_ldf, node->decimal);
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
    void emit_newdic(newdic_node *node) {
        for (int i=0;i<node->children.size();i+=2) {
            auto key = (ident_node*)node->children[i];
            emitter.emit(opcode::op_ldstr, key->ident);
            emit(node->children[i + 1]);
        }
        emitter.emit(opcode::op_newdic, node->children.size() / 2);
    }
    void emit_op(op_node *node) {
        if (node->op == L"is") {
            emit(node->left());
            emitter.emit(opcode::op_ldi, sig2hash(node->right()->token().raw));
        }
        else {
            emit(node->left());
            emit(node->right());
        }

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
        else if (node->op == L"!=")
            emitter.emit(opcode::op_neq);
        else if (node->op == L"&&")
            emitter.emit(opcode::op_and);
        else if (node->op == L"||")
            emitter.emit(opcode::op_or);
        else if (node->op == L"!")
            emitter.emit(opcode::op_not);
        else if (node->op == L"is")
            emitter.emit(opcode::op_eqtype, sig2hash(node->right()->token().raw));
    }
    void emit_assignment(assignment_node *node) {
        emit(node->right());

        auto ident = dynamic_cast<ident_node*>(node->left());
        if (ident != nullptr) {
            auto lookup = scope.lookup_variable(ident->ident);
            if (lookup.type == lookup_type::not_exist) {
                // Cannot be happend, maybe bug?
                //ctx.push_error(undefined_variable_error(ident->token()));
                //return;
            }

            if (ident->ident[0] == L'@') {
                emitter.emit(opcode::op_ldthis);
                emitter.emit(opcode::op_stprop, sig2hash(ident->ident, 1));
            }
            else 
                emitter.emit(opcode::op_stloc, lookup.index);

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
        int end_then = emitter.emit(opcode::op_jmp);

        emitter.modify_operand(jmp, emitter.get_cursor());
        for (auto it = node->begin_else(); it != node->end_else(); ++it) {
            emit(*it);
        }

        emitter.modify_operand(end_then, emitter.get_cursor());
    }
    void emit_else(else_node *node) {
        emit(node->then());
    }
    void emit_throw(throw_node *node) {
        emit(node->exception());
        emitter.emit(opcode::op_throw);
    }
    void emit_for(for_node *node) {
        emit(node->init());
        int jmp = emitter.emit(opcode::op_jmp);
        int body = emitter.get_cursor();
        emit(node->body());
        emit(node->increment());
        emitter.emit(opcode::op_nop);
        int cond = emitter.get_cursor();
        emit(node->cond());
        emitter.emit(opcode::op_jmp_true, body);

        emitter.modify_operand(jmp, cond);
    }
    void emit_foreach(foreach_node *node) {
        emit(node->right());
        emitter.emit(opcode::op_setcallee);
        emitter.emit(opcode::op_vcall, sig2hash(L"get_iterator"));

        auto jmpsite = emitter.emit(opcode::op_dup);
        emitter.emit(opcode::op_setcallee);
        emitter.emit(opcode::op_vcall, sig2hash(L"current"));

        // Most common foreach with 1 variable
        //    for (item : ary) { }
        if (node->children.size() == 3) {
            auto ident = ((ident_node*)node->children[0])->ident;
            auto lookup = scope.lookup_variable(ident);
            emitter.emit(opcode::op_stloc, lookup.index);
        }
        // More than 2 variables
        //    for (key, value : dictionary) { }
        else {
            for (auto it = std::next(node->begin_vars()); it != node->end_vars(); ++it) {
                emitter.emit(opcode::op_dup);
            }
            int i = 0;
            for (auto it = node->begin_vars(); it != node->end_vars(); ++it) {
                auto ident = ((ident_node*)*it)->ident;
                auto lookup = scope.lookup_variable(ident);
                emitter.emit(opcode::op_setcallee);
                emitter.emit(opcode::op_ldi, i);
                emitter.emit(opcode::op_vcall, sig2hash(rk_id_getitem));
                emitter.emit(opcode::op_stloc, lookup.index);

                ++i;
            }
        }

        emit(node->body());

        emitter.emit(opcode::op_dup);
        emitter.emit(opcode::op_setcallee);
        emitter.emit(opcode::op_vcall, sig2hash(L"move_next"));
        emitter.emit(opcode::op_jmp_true, jmpsite);
        emitter.emit(opcode::op_pop);
    }
    void emit_while(while_node *node) {
        emitter.emit(opcode::op_nop);
        auto cond = emitter.get_cursor();
        emit(node->cond());
        auto jmp_false = emitter.get_cursor();
        emitter.emit(opcode::op_jmp_false, 0);
        emit(node->body());
        emitter.emit(opcode::op_jmp, cond);
        emitter.modify_operand(jmp_false, emitter.get_cursor());
    }

private:
    compile_context &ctx;

    scope scope;
    program_builder emitter;

    root_node *root;
    class_node *current_class;
    method_node *current_method;    
};