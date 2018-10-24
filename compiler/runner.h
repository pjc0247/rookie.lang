#pragma once

#include <string>
#include <map>
#include <deque>
#include <exception>

#include "program.h"
#include "errors.h"

#include "gc.h"
#include "debugger.h"
#include "exe_context.h"

#include "libs/array.h"
#include "libs/dictionary.h"

struct value;
class object;
class binding;

enum class runtime_typekind {
    tk_systype,
    tk_programtype
};
struct runtime_typedata {
    std::wstring name;
    uint32_t sighash;

    std::vector<uint32_t> parents;

    runtime_typekind typekind;

    calltable vtable;
};

class runner {
    friend gc;
    friend exe_context;
    friend debugger;
public:
    runner(const program &p, binding &binding);

    runner &attach_debugger(debugger &_dbger) {
        dbger = &_dbger;
        return *this;
    }

    void execute();
    void execute(program_entry *_entry);

    void  _vcall(int sighash, stack_provider &sp);
    void  _newobj_systype(int sighash, stack_provider &sp);
    value _initobj_systype(int sighash, object *objref);

private:
    void build_runtime_data();
    void load_all_systypes();
    void load_all_programtypes();
    void load_programtype(uint32_t sighash);

    value get_local(int n);
    runtime_typedata get_type(const value &v);
    runtime_typedata get_type(uint32_t sighash);

    void syscall(int index, stack_provider &sp);
    void programcall(int index);

    value &top();
    value pop();
    void  push(const value &v);

    void push_callframe(program_entry &entry);
    callframe pop_callframe(program_entry &entry);

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

    rkexception exception;

    // STACKS
    std::deque<callframe> callstack;
    std::deque<value> stack;

#ifdef RK_HALT_ON_LONG_EXECUTION
    int halt_counter;
#endif
};