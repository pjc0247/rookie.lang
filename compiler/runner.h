/*  runner.h
    
    A Standard implementation of Rookie::VM
    (Without JIT)
 */
#pragma once

#include <string>
#include <map>
#include <deque>
#include <exception>

#include "errors.h"

#include "gc.h"
#include "debugger.h"

struct program;
struct instruction;

struct value;
class  object;
class  binding;
class  exe_context;
class  vstack;

struct primitive_cache;
struct type_cache;

struct reflection_typedata;
struct runtime_typedata;

enum class runtime_typekind {
    tk_systype,
    tk_programtype
};
struct runtime_typedata {
    std::wstring name;
    uint32_t sighash;

    std::map<uint32_t, value> fields;
    std::vector<uint32_t> parents;

    runtime_typekind typekind;

    calltable vtable;

    reflection_typedata *reflection;
};
struct reflection_typedata {
    std::wstring name;

    std::vector<reflection_typedata*> parents;
    std::vector<std::wstring> fields;
};

struct callframe {
    program_entry *entry;
    short pc;
    short bp;

    callframe(short pc, short bp, program_entry *entry) :
        pc(pc), bp(bp), entry(entry) {
    }
};
struct exception_frame {
    exception_handler exh;
    uint16_t bp;
};

class runner {
    friend gc;
    friend exe_context;
    friend debugger;
public:
    runner(const program &p, binding &binding);
    virtual ~runner();

    runner &attach_debugger(debugger &_dbger) {
        dbger = &_dbger;
        return *this;
    }

    void execute();
    void execute(program_entry *_entry);
    void run_entry(program_entry *_entry);

    const std::wstring &hash_to_string(uint32_t hash);

    void  set_callee_as_top();
    void  _vcall(int sighash, stack_provider &sp);
    void  _newobj_systype(int sighash, stack_provider &sp);
    value _initobj_systype(int sighash, object *objref);
    value _initobj_systype_nogc(int sighash, object *objref);

private:
    void build_runtime_data();
    void build_primitive_cache();
    void build_type_cache();

    void load_all_systypes();
    void load_all_programtypes();
    void load_programtype(uint32_t sighash);

    void op_eqtype();
    void op_eq();
    void op_neq();
    void op_and();
    void op_or();
    void op_not();

    void op_add();

    void op_newobj();
    void op_newarr();
    void op_newdic();

    void op_vcall();
    void op_ret();

    void op_ldprop();
    void op_stprop();
    void op_ldfld();
    void op_stfld();

    value get_local(int n);
    runtime_typedata get_type(const value &v);
    runtime_typedata get_type(uint32_t sighash);
    rktype *get_rktype(uint32_t sighash);

    bool handle_exception();

    void syscall(int index, stack_provider &sp);
    void programcall(int index);

    value &top();
    value pop();
    void  push(const value &v);
    void  replace_top(const value &v);

    void push_callframe(program_entry &entry);
    callframe pop_callframe(program_entry &entry);

private:
    const program &p;
    const binding &binding;

    exe_context *exectx;

    syscalltable syscalls;
    std::map<uint32_t, runtime_typedata> types;
    std::map<uint32_t, std::wstring> id_pool;

    gc gc;
    debugger *dbger;

    primitive_cache *ptype;
    type_cache      *typecache;

    // REGISTERS
    program_entry  *current_entry;
    instruction     inst;
    bool            errflag;
    bool            endflag;
    stack_provider  sp;
                   
    // REGISTERS   
    uint16_t        pc; // program counter
    uint16_t        bp; // base stack pointer
    value          *callee_ptr; // .this

    base_exception *exception;

    // STACKS
    std::deque<callframe> callstack;
    std::deque<exception_frame> exh_stack;

#if RK_USE_STDSTACK
    std::deque<value> stack;
#else
    vstack stack;
#endif

#ifdef RK_HALT_ON_LONG_EXECUTION
    int halt_counter;
#endif
};