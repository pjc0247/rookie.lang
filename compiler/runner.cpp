#include "stdafx.h"

#include "value_object.h"
#include "binding.h"

#include "libs/array.h"
#include "libs/dictionary.h"

#include "runner.h"

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

runner::runner(const program &p, ::binding &binding) :
    p(p), binding(binding),
    dbger(nullptr),
    callee_ptr(nullptr),
    gc(*this),
    sp(stack) {

    build_runtime_data();
}
void runner::execute() {
    if (p.header.entry_len == 0)
        throw new std::invalid_argument("program does not have any entries.");

    execute(&p.entries[p.header.main_entry]);
}
void runner::execute(program_entry *_entry) {
#ifdef RK_HALT_ON_LONG_EXECUTION
    halt_counter = 0;
#endif

    current_entry = _entry;
    pc = _entry->entry;
    bp = 0;

    push_callframe(*_entry);

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

        // Prepare for next execution
        inst = p.code[pc++];
        errflag = false;

        if (dbger) dbger->on_pre_exec(*this, inst);

        switch (inst.opcode) {
        case opcode::op_nop:
            break;
        case opcode::op_ldi:
            push(int2rk(inst.i32));
            break;
        case opcode::op_ldstr:
            // FIX: Performance
            push(value::mkstring(p.rdata + inst.operand));
            _newobj_systype(sighash_string, sp);
            break;
        case opcode::op_ldnull:
            push(rknull);
            break;
        case opcode::op_ldtrue:
            push(value::_true());
            break;
        case opcode::op_ldfalse:
            push(value::_false());
            break;
        case opcode::op_ldthis:
            push(stack[bp - 1]);
            break;

        case opcode::op_ldloc:
            push(get_local(inst.operand));
            break;
        case opcode::op_stloc:
            stack[bp + inst.operand] = top();
            pop();
            break;
        case opcode::op_ldprop:
            op_ldprop();
            break;
        case opcode::op_stprop:
            op_stprop();
            break;

        case opcode::op_dup:
            push(top());
            break;
        case opcode::op_pop:
            pop();
            break;
        case opcode::op_eqtype:
            op_eqtype();
            break;

        case opcode::op_eq: {
            _pop2_int(left, right);
            push(value::mkboolean(left.integer == right.integer));
            break;
        }
        case opcode::op_add:
            op_add();
            break;

        case opcode::op_newobj:
            op_newobj();
            break;
        case opcode::op_newarr:
            op_newarr();
            break;
        case opcode::op_newdic:
            op_newdic();
            break;

        case opcode::op_call:
            programcall(inst.cs.index);
            break;
        case opcode::op_syscall:
            syscall(inst.cs.index, sp);
            break;
        case opcode::op_vcall:
            op_vcall();
            break;

        case opcode::op_setcallee:
            callee_ptr = &stack.back();
            break;
        }

        if (errflag) goto error;

        if (inst.opcode == opcode::op_l) {
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

        else if (inst.opcode == opcode::op_ret) {
            auto ret = pop();
            auto callframe = pop_callframe(*current_entry);
            pc = callframe.pc;
            bp = callframe.bp;
            current_entry = callframe.entry;

            push(ret);

            if (callstack.empty()) break;
        }

        else if (inst.opcode == opcode::op_jmp_true) {
            if (pop().integer != 0)
                pc = inst.operand;
        }
        else if (inst.opcode == opcode::op_jmp_false) {
            if (pop().integer == 0)
                pc = inst.operand;
        }

        //else
        //    throw invalid_program_exception("unknown instruction.");

    end_loop:
        continue;
    error:
        printf("[EXCEPTION]\n");
        printf("%s\n", exception.what());
        break;
    }

    delete exectx;
}

void runner::op_eqtype() {
    _pop2_int(left, right);

    auto type = get_type(left);

    if (type.sighash == right.uinteger)
        push(value::mkboolean(true));
    else {
        for (auto hash : type.parents) {
            if (right.uinteger == hash) {
                push(value::mkboolean(true));
                return;
            }
        }

        push(value::mkboolean(false));
    }
}
void runner::op_add() {
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
void runner::op_newobj() {
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
void runner::op_newarr() {
    set_rkctx(exectx);
    auto aryref = new rkarray(inst.operand);
    // FIXME
    aryref->vtable = &types[sighash_array].vtable.table;
    push(value::mkobjref(aryref));

    gc.add_object(aryref);
}
void runner::op_newdic() {
    set_rkctx(exectx);
    auto aryref = new rkdictionary(inst.operand);
    // FIXME
    aryref->vtable = &types[sighash_dictionary].vtable.table;
    push(value::mkobjref(aryref));

    gc.add_object(aryref);
}

void runner::op_vcall() {
    auto sighash = inst.operand;
    std::map<uint32_t, callinfo> *vtable;

    if (callee_ptr->type == value_type::integer) {
        vtable = &types[sighash_integer].vtable.table;
        push(top());
    }
    else {
        auto calleeobj = callee_ptr->objref;
        vtable = calleeobj->vtable;
    }

    auto _callinfo = vtable->find(sighash);
    if (_callinfo == vtable->end()) {
        exception = rkexception("No such method");
        errflag = true;
    }

    auto callinfo = (*_callinfo).second;
    if (callinfo.type == call_type::ct_syscall_direct)
        syscall(callinfo.entry, sp);
    else if (callinfo.type == call_type::ct_programcall_direct)
        programcall(callinfo.entry);
}

void runner::op_ldprop() {
    auto obj = pop();

    if (obj.type != value_type::object)
        throw invalid_program_exception("target is not an object");

    push(obj.objref->get_property(inst.operand));
}
void runner::op_stprop() {
    auto obj = pop();
    auto value = pop();

    if (obj.type != value_type::object)
        throw invalid_program_exception("target is not a object");

    obj.objref->properties[inst.operand] = value;
}

__forceinline
value runner::get_local(int n) {
    auto v = stack[bp + n];
#if _RK_STRICT_CHECK
    if (v.type == value_type::empty)
        throw invalid_access_exception("Accessed to the unassigned slot.");
#endif
    return v;
}
__forceinline
runtime_typedata runner::get_type(const value &v) {
    if (v.type == value_type::integer)
        return types[sighash_integer];
    else
        return types[v.objref->sighash];
}
__forceinline
runtime_typedata runner::get_type(uint32_t sighash) {
    return types[sighash];
}

__forceinline
void runner::syscall(int index, stack_provider &sp) {
    set_rkctx(exectx);
    syscalls.table[index](sp);
}
__forceinline
void runner::programcall(int index) {
    auto &entry = p.entries[index];
    auto stacksize = stack.size();
    push_callframe(entry);
    pc = entry.entry;
    bp = stacksize - entry.params;
    current_entry = &entry;
}

// Performs vcall, internal use only.
void runner::_vcall(int sighash, stack_provider &sp) {
    auto calleeobj = callee_ptr->objref;

    auto callinfo = calleeobj->vtable->at(sighash);
    if (callinfo.type == call_type::ct_syscall_direct)
        syscall(callinfo.entry, sp);
    else if (callinfo.type == call_type::ct_programcall_direct)
        programcall(callinfo.entry);
}
// internal use only.
void runner::_newobj_systype(int sighash, stack_provider &sp) {
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
value runner::_initobj_systype(int sighash, object *objref) {
    auto obj = value::mkobjref(objref);
    obj.objref->vtable = &types[sighash].vtable.table;
    obj.objref->sighash = sighash;

    gc.add_object(obj.objref);

    return obj;
}

value &runner::top() {
    return stack.back();
}
value runner::pop() {
    if (stack.empty())
        throw invalid_program_exception("stack underflow");
    auto item = stack.back();
    stack.pop_back();
    return item;
}
void runner::push(const value &v) {
    stack.push_back(v);
}
void runner::push_callframe(program_entry &entry) {
    callstack.push_back(callframe(pc, bp, current_entry));
    for (int i = 0; i < entry.locals - entry.params; i++)
        stack.push_back(value());
}
callframe runner::pop_callframe(program_entry &entry) {
    for (int i = 0; i < entry.locals; i++)
        stack.pop_back();

    auto callframe = callstack.back();
    callstack.pop_back();

    return callframe;
}

void runner::build_runtime_data() {
    // SYS FUNCTIONS
    for (auto &b : binding.get_functions()) {
        syscalls.table.push_back(b.second);
    }
    // STATIC METHODS
    for (auto &type : binding.get_types()) {
        for (auto &static_method : type.get_static_methods())
            syscalls.table.push_back(static_method.second);
    }

    load_all_systypes();
    load_all_programtypes();
}

void runner::load_all_systypes() {
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
        tdata.sighash = typesighash;

        // TODO
        tdata.parents.push_back(sighash_object);

        types[typesighash] = tdata;
    }
}
void runner::load_all_programtypes() {
    for (uint32_t i = 0; i < p.header.types_len; i++) {
        load_programtype(sig2hash(p.types[i].name));
    }
}
void runner::load_programtype(uint32_t sighash) {
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

            tdata.parents.push_back(sighash_object);
        }
        else {
            for (int j = 0; j < p.types[i].parents_len; j++) {
                uint32_t parent_hash = p.types[i].parents[j];

                load_programtype(parent_hash);

                for (auto &method : types[parent_hash].vtable.table) {
                    vtable.table[method.first] = method.second;
                }

                tdata.parents.push_back(parent_hash);
                for (auto parent : types[parent_hash].parents) {
                    tdata.parents.push_back(parent);
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
        tdata.sighash = sighash;

        types[sig2hash(type.name)] = tdata;
    }
}