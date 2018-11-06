#include "stdafx.h"

#include "binding.h"
#include "sig2hash.h"

#include "libs/array.h"
#include "libs/dictionary.h"
#include "libs/type.h"
#include "libs/integer.h"

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

#define _pop2_int(a, b) \
    auto b = pop(); auto a = pop(); 

struct primitive_cache {
    runtime_typedata integer, string, object, boolean;
    runtime_typedata array, dictionary;
};
struct type_cache {
    std::map<uint32_t, rktype*> table;
};

runner::runner(const program &p, ::binding &binding) :
    p(p), binding(binding),
    dbger(nullptr),
    callee_ptr(nullptr),
    ptype(nullptr), typecache(nullptr),
    gc(*this),
    sp(stack) {

    build_runtime_data();
    build_primitive_cache();
    build_type_cache();
}
runner::~runner() {
    delete ptype;

    for (auto t : typecache->table)
        delete t.second;
    delete typecache;
}

void runner::execute() {
    if (p.header.entry_len == 0)
        throw new std::invalid_argument("program does not have any entries.");

    execute(&p.entries[p.header.main_entry]);
}
void runner::execute(program_entry *_entry) {
    assert(_entry != nullptr);

#ifdef RK_HALT_ON_LONG_EXECUTION
    halt_counter = 0;
#endif

    current_entry = _entry;
    pc = _entry->entry;
    bp = 0;
    endflag = false;
    errflag = false;

    push_callframe(*_entry);

    exectx = new exe_context(*this, sp);

    if (dbger) dbger->on_begin_program(*this, p);

    run_entry(_entry);

#if _DEBUG
    if (stack.size() > 1) {
        printf("STACK SIZE : %d\n", stack.size());

        dbger->dumpstack();
    }
#endif

    delete exectx;
}
void runner::run_entry(program_entry *_entry) {
    int ss = stack.size();
    int depth = callstack.size();

    while (!endflag && callstack.size() >= depth) {
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
        case opcode::op_ldf:
            push(float2rk(inst.f32));
            break;
        case opcode::op_ldstr:
            // FIX: Performance
            push(value::mkstring(p.rdata + inst.operand));
            _newobj_systype(sighash_string, sp);
            break;
        case opcode::op_ldnull:
            push(rknull);
            break;
        case opcode::op_ldempty:
            push(rkempty);
            break;
        case opcode::op_ldtrue:
            push(rktrue);
            break;
        case opcode::op_ldfalse:
            push(rkfalse);
            break;
        case opcode::op_ldthis:
            push(stack[bp - 1]);
            break;
        case opcode::op_ldtype:
            push(value::mkobjref(typecache->table[inst.operand]));
            break;

        case opcode::op_ldloc:
            push(get_local(inst.operand));
            break;
        case opcode::op_stloc:
            stack[bp + inst.operand] = top();

#if _DEBUG
            stack[bp + inst.operand].ld_pc = pc;
#endif

            pop();
            break;

        case opcode::op_ldprop:
            op_ldprop();
            break;
        case opcode::op_stprop:
            op_stprop();
            break;
        case opcode::op_ldfld:
            op_ldfld();
            break;
        case opcode::op_stfld:
            op_stfld();
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

        case opcode::op_eq:
            op_eq();
            break;
        case opcode::op_neq:
            op_neq();
            break;
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
        case opcode::op_ret:
            op_ret();
            break;

        case opcode::op_setcallee:
            callee_ptr = &stack.back();
            break;

        case opcode::op_jmp:
            pc = inst.operand;
            break;
        case opcode::op_jmp_true:
            if (pop().integer != 0)
                pc = inst.operand;
            break;
        case opcode::op_jmp_false:
            if (pop().integer == 0)
                pc = inst.operand;
            break;

        default:
            break;
        }

        if (errflag) goto error;

        if (inst.opcode == opcode::op_l) {
            _pop2_int(left, right);
            push(value::mkboolean(left.integer < right.integer));
        }
        else if (inst.opcode == opcode::op_g) {
            _pop2_int(left, right);
            push(value::mkboolean(left.integer > right.integer));
        }
        else if (inst.opcode == opcode::op_le) {
            _pop2_int(left, right);
            push(value::mkboolean(left.integer <= right.integer));
        }
        else if (inst.opcode == opcode::op_ge) {
            _pop2_int(left, right);
            push(value::mkboolean(left.integer >= right.integer));
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

        //else
        //    throw invalid_program_exception("unknown instruction.");

        //printf("ss %d\n", stack.size());

        continue;
    error:
        printf("[EXCEPTION]\n");
        printf("%s\n", exception.what());

        assert(0);

        break;
    }
}

void runner::op_eqtype() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  LEFT
     *  RIGHT
     *  OP_EQTYPE
     */
    _pop2_int(left, right);

    auto type = get_type(left);

    if (type.sighash == right.uinteger)
        push(rktrue);
    else {
        for (auto hash : type.parents) {
            if (right.uinteger == hash) {
                push(rktrue);
                return;
            }
        }

        push(rkfalse);
    }
}
void runner::op_eq() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  LEFT
     *  RIGHT
     *  OP_EQ
     */
    _pop2_int(left, right);

    if (left.type == right.type) {
        if (left.uinteger == right.uinteger)
            push(rktrue);
        else if(left.type == value_type::object) {
            push(left);
            callee_ptr = &left;
            push(right);
            _vcall(sighash_equal, sp);
        }
        else 
            push(rkfalse);
    }
    else
        push(rkfalse);
}
void runner::op_neq() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  LEFT
     *  RIGHT
     *  OP_NEQ
     */
    _pop2_int(left, right);

    if (left.type == right.type) {
        if (left.uinteger == right.uinteger)
            push(rkfalse);
        else if (left.type == value_type::object) {
            push(left);
            callee_ptr = &left;
            push(right);
            _vcall(sighash_equal, sp);

            if (top() == rktrue)
                replace_top(rkfalse);
            else
                replace_top(rktrue);
        }
        else
            push(rktrue);
    }
    else
        push(rktrue);
}

void runner::op_add() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  LEFT
     *  RIGHT
     *  OP_ADD          
     */
    auto right = pop();
    auto left  = top();

    if (left.type == value_type::integer) {
        left.integer += right.integer;
        replace_top(left);
    }
    else if (is_rkstr(left)) {
        auto str = new rkstring(rkwstr(left) + rkwstr(right));
        replace_top(_initobj_systype(sighash_string, str));
    }
    else {
        push(left);
        callee_ptr = &left;
        push(right);
        _vcall(sig2hash(L"__add__"), sp);
    }
}
void runner::op_newobj() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  ARGS (0..n)
     *  OP_NEWOBJ          SIGHASH
     */
#if _RK_STRICT_CHECK
    if (types.find(inst.operand) == types.end())
        throw invalid_program_exception("No such type");
#endif

    // FIXME

    if (types[inst.operand].typekind == runtime_typekind::tk_programtype) {
        auto objref = new rkscriptobject();
        push(value::mkobjref(objref));

        objref->vtable = &types[inst.operand].vtable;
        objref->sighash = inst.operand;

        if (objref->vtable->find(sighash__ctor) !=
            objref->vtable->end()) {

            set_callee_as_top();
            _vcall(sighash__ctor, sp);
            pop();

            push(value::mkobjref(objref));
        }

        gc.add_object(objref);
    }
    else {
        // FIXME
        auto newcall = types[inst.operand].vtable[sighash_new];

        if (newcall.type == call_type::ct_syscall_direct) {
            syscall(newcall.entry, sp);
            auto obj = stack[stack.size() - 1];
            obj.objref->vtable = &types[inst.operand].vtable;
            obj.objref->sighash = inst.operand;
            gc.add_object(obj.objref);
        }
        else
            throw invalid_access_exception("invalid calltype for .new");
    }
}
void runner::op_newarr() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  KEY   (0..n)
     *  VALUE (0..n)
     *  OP_NEWARR          INTEGER
     */
    set_rkctx(exectx);
    auto aryref = new rkarray(inst.operand);
    // FIXME
    aryref->vtable = &ptype->array.vtable;
    aryref->sighash = sighash_array;
    push(value::mkobjref(aryref));

    gc.add_object(aryref);
}
void runner::op_newdic() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  KEY   (0..n)
     *  VALUE (0..n)
     *  OP_NEWDIC          INTEGER
     */
    set_rkctx(exectx);
    auto dicref = new rkdictionary(inst.operand);
    // FIXME
    dicref->vtable = &ptype->dictionary.vtable;
    dicref->sighash = sighash_dictionary;
    push(value::mkobjref(dicref));

    gc.add_object(dicref);
}

void runner::op_vcall() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  THIS(object)
     *  ARGS (0..n)
     *  OP_LDVCALL
     */
    _vcall(inst.operand, sp);
}
void runner::op_ret() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  OP_RET
     */
    auto ret = pop();
    auto callframe = pop_callframe(*current_entry);
    pc = callframe.pc;
    bp = callframe.bp;
    current_entry = callframe.entry;

    push(ret);
}

void runner::op_ldprop() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  THIS(object)
     *  OP_LDPROP          SIGHASH
     */
    auto obj = pop();

    if (obj.type != value_type::object)
        throw invalid_program_exception("target is not an object");

    push(obj.objref->get_property(inst.operand));
}
void runner::op_stprop() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  VALUE
     *  .THIS(object)
     *  OP_STPROP          SIGHASH
     */
    auto obj = pop();
    auto value = pop();

    if (obj.type != value_type::object)
        throw invalid_program_exception("target is not a object");

    obj.objref->properties[inst.operand] = value;
}
void runner::op_ldfld() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  TYPE
     *  OP_LDFLD           SIGHASH
     */
    auto type = pop();
    auto value = rk2obj(type, rktype*)->rtype
        .fields[inst.operand];

    push(value);
}
void runner::op_stfld() {
    // [STACK-LAYOUT   |   OPERAND]
    /*  TYPE
     *  VALUE
     *  OP_STFLD           SIGHASH
     */
    auto value = pop();
    auto type = pop();

    rk2obj(type, rktype*)->rtype
        .fields[inst.operand] = value;
}

__forceinline
value runner::get_local(int n) {
    assert(stack.size() > bp + n);

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
        return ptype->integer;
    else if (v.type == value_type::boolean)
        return ptype->boolean;
    else
        return types[v.objref->sighash];
}
__forceinline
runtime_typedata runner::get_type(uint32_t sighash) {
    assert(types.find(sighash) != types.end());

    return types[sighash];
}
rktype *runner::get_rktype(uint32_t sighash) {
    auto it = typecache->table.find(sighash);
    if (it == typecache->table.end())
        return nullptr;
    return (*it).second;
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

    run_entry(current_entry);
}

void runner::set_callee_as_top() {
    if (stack.size() == 0)
        throw rkexception("stack.empty");

    callee_ptr = &stack[stack.size() - 1];
}

// Performs vcall, internal use only.
void runner::_vcall(int sighash, stack_provider &sp) {
    std::map<uint32_t, callinfo> *vtable;

    if (callee_ptr->type == value_type::integer) {
        vtable = &ptype->integer.vtable;

        auto rkint = new rkinteger(rk2int(top()));
        auto v = _initobj_systype(sighash_integer, rkint);
        replace_top(v);
    }
    else if (callee_ptr->type == value_type::boolean) {
        vtable = &ptype->boolean.vtable;
        push(top());
    }
    else {
        auto calleeobj = callee_ptr->objref;
        vtable = calleeobj->vtable;
    }

    auto _callinfo = vtable->find(sighash);
    if (_callinfo == vtable->end()) {
        exception = rkexception("No such method: ");
        errflag = true;

        assert(0);
    }
    else {
        auto callinfo = (*_callinfo).second;
        if (callinfo.type == call_type::ct_syscall_direct)
            syscall(callinfo.entry, sp);
        else if (callinfo.type == call_type::ct_programcall_direct) {
            programcall(callinfo.entry);
            stack[stack.size() - 2] = stack[stack.size() - 1];
            pop();
        }
    }
}
// internal use only.
void runner::_newobj_systype(int sighash, stack_provider &sp) {
    auto newcall = types[sighash].vtable[sighash_new];

#if _RK_STRICT_CHECK
    if (newcall.type != call_type::ct_syscall_direct)
        throw invalid_access_exception("invalid calltype for .new");
#endif

    syscall(newcall.entry, sp);
    auto obj = top();
    obj.objref->vtable = &types[sighash].vtable;
    obj.objref->sighash = sighash;

    gc.add_object(obj.objref);
}
value runner::_initobj_systype(int sighash, object *objref) {
    auto obj = value::mkobjref(objref);
    obj.objref->vtable = &types[sighash].vtable;
    obj.objref->sighash = sighash;

    gc.add_object(obj.objref);

    return obj;
}
value runner::_initobj_systype_nogc(int sighash, object *objref) {
    auto obj = value::mkobjref(objref);
    obj.objref->vtable = &types[sighash].vtable;
    obj.objref->sighash = sighash;

    return obj;
}

value &runner::top() {
    if (stack.empty())
        throw invalid_program_exception("stack underflow");
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

#if _DEBUG
    top().ld_pc = pc;
#endif
}
void runner::replace_top(const value &v) {
    assert(stack.size() > 0);

    stack[stack.size()-1] = v;
}

void runner::push_callframe(program_entry &entry) {
    callstack.push_back(callframe(pc, bp, current_entry));
    for (uint16_t i = 0; i < entry.locals - entry.params; i++)
        stack.push_back(value::empty());
}
callframe runner::pop_callframe(program_entry &entry) {
    assert(callstack.size() > 0);

    for (uint16_t i = 0; i < entry.locals; i++)
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
void runner::build_primitive_cache() {
    assert(ptype == nullptr);

    ptype = new primitive_cache();
    ptype->integer = types[sighash_integer];
    ptype->string = types[sighash_string];
    ptype->object = types[sighash_object];
    ptype->array = types[sighash_array];
    ptype->dictionary = types[sighash_dictionary];
    ptype->boolean = types[sighash_boolean];
}
void runner::build_type_cache() {
    assert(typecache == nullptr);

    typecache = new type_cache();

    for (auto &type : types) {
        typecache->table[type.first] =
            new rktype(type.first, type.second);
    }
}

void runner::load_all_systypes() {
    for (auto &type : binding.get_types()) {
        auto typesighash = sig2hash(type.get_name());

        auto methods = type.get_methods();
        calltable vtable;

        for (auto &method : methods) {
            auto sighash = sig2hash(method.first);
            syscalls.table.push_back(method.second);

            vtable[sighash].type = call_type::ct_syscall_direct;
            vtable[sighash].entry = syscalls.table.size() - 1;
        }

        runtime_typedata tdata;
        tdata.typekind = runtime_typekind::tk_systype;
        tdata.vtable = vtable;
        tdata.sighash = typesighash;

        // TODO
        tdata.parents.push_back(sighash_object);

        tdata.reflection = new reflection_typedata();
        tdata.reflection->name = tdata.name;

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
    //if (types.find(sighash) != types.end())
    //    return;

    for (uint32_t i = 0; i < p.header.types_len; i++) {
        auto type = p.types[i];

        if (sig2hash(type.name) != sighash)
            continue;

        runtime_typedata tdata;
        if (types.find(sighash) != types.end())
            tdata = types[sighash];

        tdata.typekind = runtime_typekind::tk_programtype;

        calltable &vtable = tdata.vtable;

        tdata.reflection = new reflection_typedata();

        // Inherit from `object`
        if (p.types[i].parents_len == 0) {
            auto basevtable = types[sighash_object].vtable;
            for (auto &method : basevtable) {
                vtable[method.first] = method.second;
            }

            tdata.reflection->parents.push_back(types[sighash_object].reflection);

            tdata.parents.push_back(sighash_object);
        }
        // Inherit from parents
        else {
            for (uint32_t j = 0; j < p.types[i].parents_len; j++) {
                uint32_t parent_hash = p.types[i].parents[j];

                load_programtype(parent_hash);

                for (auto &method : types[parent_hash].vtable) {
                    vtable[method.first] = method.second;
                }

                tdata.reflection->parents.push_back(types[parent_hash].reflection);

                tdata.parents.push_back(parent_hash);
                for (auto parent : types[parent_hash].parents) {
                    tdata.parents.push_back(parent);
                }
            }
        }

        for (uint32_t j = 0; j < type.methods_len; j++) {
            auto method = type.methods[j];
            auto methodhash = sig2hash(method.name);

            vtable[methodhash].type = call_type::ct_programcall_direct;
            vtable[methodhash].entry = method.entry;
        }
        tdata.vtable = vtable;
        tdata.name = type.name;
        tdata.sighash = sighash;

        tdata.reflection->name = type.name;

        types[sig2hash(type.name)] = tdata;
    }
}