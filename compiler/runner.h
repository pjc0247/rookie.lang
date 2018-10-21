﻿#pragma once

#include <string>
#include <map>
#include <deque>
#include <exception>

#include "program.h"
#include "errors.h"

#include "value_object.h"
#include "gc.h"
#include "binding.h"
#include "debugger.h"
#include "exe_context.h"

#include "libs/array.h"
#include "libs/dictionary.h"

#define _invalid_stackitem \
    throw new invalid_program_exception("invalid stackitem, expected integer");

#define _ensure_int(a) \
    if (a.type != value_type::integer) \
        _invalid_stackitem;
#define _ensure_int(a,b) \
    if (a.type != value_type::integer || \
        b.type != value_type::integer) \
        _invalid_stackitem;

#define _pop1_int(a) \
    autoa = pop(); 
#define _pop2_int(a, b) \
    auto b = pop(); auto a = pop(); 

#define _stacktop() stack[stack.size() - 1]

enum class runtime_typekind {
    tk_systype,
    tk_programtype
};
struct runtime_typedata {
    std::wstring name;

    runtime_typekind typekind;

    calltable vtable;
};

class runner {
    friend gc;
    friend exe_context;
    friend debugger;
public:
    runner(const program &p, binding &binding) :
        p(p), binding(binding),
        dbger(nullptr),
        callee_ptr(nullptr),
        gc(*this) {

        build_runtime_data();
    }

    runner &attach_debugger(debugger &_dbger) {
        dbger = &_dbger;
        return *this;
    }

    void execute() {
        if (p.header.entry_len == 0)
            throw new std::invalid_argument("program does not have any entries.");

        execute(&p.entries[p.header.main_entry]);
    }
    void execute(program_entry *_entry) {
#ifdef RK_HALT_ON_LONG_EXECUTION
        halt_counter = 0;
#endif
         
        current_entry = _entry;
        pc = _entry->entry;
        bp = 0;

        push_callframe(*_entry);

        stack_provider sp(stack);

        exectx = new exe_context(*this, sp);

        if (dbger) dbger->on_begin_program(*this, p);

        while (true) {
#ifdef RK_HALT_ON_LONG_EXECUTION
            halt_counter++;
            if (halt_counter >= 10000) {
                printf("Force break due to `RK_HALT_ON_LONG_EXECUTION` flag.\n");
                break;
            }
#endif

            if (pc >= p.header.code_len)
                throw invalid_program_exception("unexpected end of program.");

            auto inst = p.code[pc++];

            if (dbger) dbger->on_pre_exec(*this, inst);

            if (inst.opcode == opcode::op_nop);
            else if (inst.opcode == opcode::op_ldi)
                stack.push_back(value::mkinteger(inst.operand));
            else if (inst.opcode == opcode::op_ldstr) {
                // [TODO] Performance consideration:
                stack.push_back(value::mkstring(p.rdata + inst.operand));
                _newobj_systype(sighash_string, sp);
            }
            else if (inst.opcode == opcode::op_ldnull)
                push(value::null());
            else if (inst.opcode == opcode::op_ldtrue)
                push(value::_true());
            else if (inst.opcode == opcode::op_ldfalse)
                push(value::_false());

            //else if (inst.opcode == opcode::op_ldprop)
            //    stack.push_back(value::mkstring(p.rdata + inst.operand));

            else if (inst.opcode == opcode::op_ldthis)
                stack.push_back(stack[bp-1]);

            else if (inst.opcode == opcode::op_pop)
                stack.pop_back();
            else if (inst.opcode == opcode::op_dup)
                stack.push_back(stack.back());

            else if (inst.opcode == opcode::op_eq) {
                _pop2_int(left, right);
                push(value::mkboolean(left.integer == right.integer));
            }
            else if (inst.opcode == opcode::op_l) {
                _pop2_int(left, right);
                push(value::mkboolean(left.integer > right.integer));
            }
            else if (inst.opcode == opcode::op_g) {
                _pop2_int(left, right);
                push(value::mkboolean(left.integer < right.integer));
            }
            else if (inst.opcode == opcode::op_le) {
                _pop2_int(left, right);
                push(value::mkboolean(left.integer >= right.integer));
            }
            else if (inst.opcode == opcode::op_ge) {
                _pop2_int(left, right);
                push(value::mkboolean(left.integer <= right.integer));
            }

            else if (inst.opcode == opcode::op_add) {
                _pop2_int(left, right);

                if (left.type == value_type::integer) {
                    left.integer += right.integer;
                    push(left);
                }
                else if (is_rkstr(left)) {
                    auto str = new rkstring(rkwstr(left) + rkwstr(right));
                    push(_initobj_systype(sighash_string, str));
                }
            }
            else if (inst.opcode == opcode::op_sub) {
                _pop2_int(left, right);
                left.integer -= right.integer;
                push(left);
            }
            else if (inst.opcode == opcode::op_div) {
                _pop2_int(left, right);
                // TODO: check left is zero
                left.integer /= right.integer;
                push(left);
            }
            else if (inst.opcode == opcode::op_mul) {
                _pop2_int(left, right);
                left.integer *= right.integer;
                push(left);
            }

            else if (inst.opcode == opcode::op_newobj) {
#if _RK_STRICT_CHECK
                if (types.find(inst.operand) == types.end())
                    throw invalid_program_exception("No such type");
#endif

                // FIXME

                if (types[inst.operand].typekind == runtime_typekind::tk_programtype) {
                    auto objref = new rkscriptobject();
                    push(value::mkobjref(objref));

                    objref->vtable = &types[inst.operand].vtable.table;
                    objref->sighash = inst.operand;

                    gc.add_object(objref);
                }
                else {
                    // FIXME
                    auto newcall = types[inst.operand].vtable.table[sighash_new];

                    if (newcall.type == call_type::ct_syscall_direct) {
                        syscall(newcall.entry, sp);
                        auto obj = stack[stack.size() - 1];
                        obj.objref->vtable = &types[inst.operand].vtable.table;
                        obj.objref->sighash = inst.operand;
                        gc.add_object(obj.objref);
                    }
                    else
                        throw invalid_access_exception("invalid calltype for .new");
                }
            }
            else if (inst.opcode == opcode::op_newarr) {
                set_rkctx(exectx);
                auto aryref = new rkarray(inst.operand);
                // FIXME
                aryref->vtable = &types[sighash_array].vtable.table;
                push(value::mkobjref(aryref));

                gc.add_object(aryref);
            }
            else if (inst.opcode == opcode::op_newdic) {
                set_rkctx(exectx);
                auto aryref = new rkdictionary(inst.operand);
                // FIXME
                aryref->vtable = &types[sighash_dictionary].vtable.table;
                push(value::mkobjref(aryref));

                gc.add_object(aryref);
            }

            else if (inst.opcode == opcode::op_setcallee)
                callee_ptr = &stack.back();

            else if (inst.opcode == opcode::op_call)
                programcall(inst.cs.index);
            else if (inst.opcode == opcode::op_syscall)
                syscall(inst.cs.index, sp);
            else if (inst.opcode == opcode::op_vcall) {
                auto calleeobj = callee_ptr->objref;
                auto sighash = inst.operand;

                auto callinfo = calleeobj->vtable->at(sighash);
                if (callinfo.type == call_type::ct_syscall_direct)  
                    syscall(callinfo.entry, sp);
                else if (callinfo.type == call_type::ct_programcall_direct)
                    programcall(callinfo.entry);
            }
            else if (inst.opcode == opcode::op_ret) {
                auto ret = pop();
                auto callframe = pop_callframe(*current_entry);
                pc = callframe.pc;
                bp = callframe.bp;
                current_entry = callframe.entry;

                push(ret);

                if (callstack.empty()) break;
            }

            else if (inst.opcode == opcode::op_ldloc) 
                stack.push_back(get_local(inst.operand));
            else if (inst.opcode == opcode::op_stloc) {
                stack[bp + inst.operand] = stack.back();
                stack.pop_back();
            }
            else if (inst.opcode == opcode::op_ldprop) {
                auto obj = pop();

                if (obj.type != value_type::object)
                    throw invalid_program_exception("target is not an object");

                push(obj.objref->get_property(inst.operand));
            }
            else if (inst.opcode == opcode::op_stprop) {
                auto obj = pop(); 
                auto value = pop();
                
                if (obj.type != value_type::object)
                    throw invalid_program_exception("target is not a object");

                obj.objref->properties[inst.operand] = value;
            }

            else if (inst.opcode == opcode::op_jmp_true) {
                if (pop().integer != 0)
                    pc = inst.operand;
            }
            else if (inst.opcode == opcode::op_jmp_false) {
                if (pop().integer == 0)
                    pc = inst.operand;
            }

            else
                throw invalid_program_exception("unknown instruction.");

//                printf("STACK %d\n", stack.size());
        }

        delete exectx;
    }

    // Performs vcall, internal use only.
    __forceinline void _vcall(int sighash, stack_provider &sp) {
        auto calleeobj = callee_ptr->objref;

        auto callinfo = calleeobj->vtable->at(sighash);
        if (callinfo.type == call_type::ct_syscall_direct)
            syscall(callinfo.entry, sp);
        else if (callinfo.type == call_type::ct_programcall_direct)
            programcall(callinfo.entry);
    }
    // internal use only.
    __forceinline void _newobj_systype(int sighash, stack_provider &sp) {
        auto newcall = types[sighash].vtable.table[sighash_new];

#if _RK_STRICT_CHECK
        if (newcall.type != call_type::ct_syscall_direct)
            throw invalid_access_exception("invalid calltype for .new");
#endif

        syscall(newcall.entry, sp);
        auto obj = _stacktop();
        obj.objref->vtable = &types[sighash].vtable.table;
        obj.objref->sighash = sighash;

        gc.add_object(obj.objref);
    }
    __forceinline value _initobj_systype(int sighash, object *objref) {
        auto obj = value::mkobjref(objref);
        obj.objref->vtable = &types[sighash].vtable.table;
        obj.objref->sighash = sighash;

        gc.add_object(obj.objref);

        return obj;
    }

private:
    void build_runtime_data() {
        // SYS FUNCTIONS
        for (auto &b : binding.get_functions()) {
            syscalls.table.push_back(b.second);
        }
        // STATIC METHODS
        for (auto &type : binding.get_types()) {
            for (auto &static_method : type.get_static_methods())
                syscalls.table.push_back(static_method.second);
        }

        // SYS METHODS
        for (auto &type : binding.get_types()) {
            auto typesighash = sig2hash(type.get_name());

            auto methods = type.get_methods();
            calltable vtable;

            for (auto &method : methods) {
                auto sighash = sig2hash(method.first);
                syscalls.table.push_back(method.second);

                vtable.table[sighash].type = call_type::ct_syscall_direct;
                vtable.table[sighash].entry = syscalls.table.size() - 1;
            }

            runtime_typedata tdata;
            tdata.typekind = runtime_typekind::tk_systype;
            tdata.vtable = vtable;
            types[typesighash] = tdata;
        }

        load_all_systypes();
        load_all_programtypes();
    }

    void load_all_systypes() {
        for (auto &type : binding.get_types()) {
            auto typesighash = sig2hash(type.get_name());

            auto methods = type.get_methods();
            calltable vtable;

            for (auto &method : methods) {
                auto sighash = sig2hash(method.first);
                syscalls.table.push_back(method.second);

                vtable.table[sighash].type = call_type::ct_syscall_direct;
                vtable.table[sighash].entry = syscalls.table.size() - 1;
            }

            runtime_typedata tdata;
            tdata.typekind = runtime_typekind::tk_systype;
            tdata.vtable = vtable;
            types[typesighash] = tdata;
        }
    }
    void load_all_programtypes() {
        for (uint32_t i = 0; i < p.header.types_len; i++) {
            load_programtype(sig2hash(p.types[i].name));
        }
    }
    void load_programtype(uint32_t sighash) {
        // Already loaded
        if (types.find(sighash) != types.end())
            return;

        for (uint32_t i = 0; i < p.header.types_len; i++) {
            auto type = p.types[i];

            if (sig2hash(type.name) != sighash)
                continue;

            runtime_typedata tdata;
            tdata.typekind = runtime_typekind::tk_programtype;

            calltable vtable;

            if (p.types[i].parents_len == 0) {
                auto basevtable = types[sighash_object].vtable.table;
                for (auto &method : basevtable) {
                    vtable.table[method.first] = method.second;
                }
            }
            else {
                for (int j = 0; j < p.types[i].parents_len; j++) {
                    uint32_t parent_hash = p.types[i].parents[j];

                    load_programtype(parent_hash);

                    for (auto &method : types[parent_hash].vtable.table) {
                        vtable.table[method.first] = method.second;
                    }
                }
            }

            for (uint32_t j = 0; j < type.methods_len; j++) {
                auto method = type.methods[j];
                auto methodhash = sig2hash(method.name);

                vtable.table[methodhash].type = call_type::ct_programcall_direct;
                vtable.table[methodhash].entry = method.entry;
            }
            tdata.vtable = vtable;
            tdata.name = type.name;

            types[sig2hash(type.name)] = tdata;
        }
    }

    __forceinline value get_local(int n) {
        auto v = stack[bp + n];
#if _RK_STRICT_CHECK
        if (v.type == value_type::empty)
            throw invalid_access_exception("Accessed to the unassigned slot.");
#endif
        return v;
    }
    __forceinline void syscall(int index, stack_provider &sp) {
        set_rkctx(exectx);
        syscalls.table[index](sp);
    }
    __forceinline void programcall(int index) {
        auto &entry = p.entries[index];
        auto stacksize = stack.size();
        push_callframe(entry);
        pc = entry.entry;
        bp = stacksize - entry.params;
        current_entry = &entry;
    }

    value pop() {
        if (stack.empty())
            throw invalid_program_exception("stack underflow");
        auto item = stack.back();
        stack.pop_back();
        return item;
    }
    void push(const value &v) {
        stack.push_back(v);
    }

    void push_callframe(program_entry &entry) {
        callstack.push_back(callframe(pc, bp, current_entry));
        for (int i = 0; i < entry.locals - entry.params; i++)
            stack.push_back(value());
    }
    callframe pop_callframe(program_entry &entry) {
        for (int i = 0; i < entry.locals; i++)
            stack.pop_back();

        auto callframe = callstack.back();
        callstack.pop_back();

        return callframe;
    }

private:
    const program &p;
    const binding &binding;

    exe_context *exectx;

    syscalltable syscalls;
    std::map<uint32_t, runtime_typedata> types;

    gc gc;
    debugger *dbger;

    // REGISTERS
    program_entry *current_entry;
    uint16_t pc; // program counter
    uint16_t bp; // base stack pointer
    value *callee_ptr; // .this

    // STACKS
    std::deque<callframe> callstack;
    std::deque<value> stack;

#ifdef RK_HALT_ON_LONG_EXECUTION
    int halt_counter;
#endif
};