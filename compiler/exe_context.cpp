#include "stdafx.h"

#include "binding.h"
#include "sig2hash.h"
#include "gc.h"
#include "runner.h"

#include "libs/exception.h"

#include "exe_context.h"

thread_local exe_context *_rkctx = nullptr;

exe_context *rkctx() {
    return _rkctx;
}
void set_rkctx(exe_context *v) {
    _rkctx = v;
}

exe_context::exe_context(runner &r, stack_provider &sp) :
    r(r), sp(sp) {
}

const std::wstring &exe_context::get_name(uint32_t sighash) {
    return r.hash_to_string(sighash);
}

void exe_context::set_exception(rkexception *exception) {
    r.exception = exception;
}

value exe_context::init_obj(uint32_t sighash, object *obj) {
    return r._initobj_systype(sighash, obj);
}
value exe_context::init_obj_nogc(uint32_t sighash, object *obj) {
    return r._initobj_systype_nogc(sighash, obj);
}

template <>
value exe_context::newobj(const std::wstring &name) {
    push_newobj(name);
    return sp.pop();
}
void exe_context::push_newobj(const std::wstring &name) {
    r._newobj_systype(sig2hash(name), sp);
}

rktype *exe_context::get_type(const std::wstring &name) {
    return r.get_rktype(sig2hash(name));
}

value &exe_context::get_this() {
    return *r.callee_ptr;
}
value exe_context::vcall(const value &obj, uint32_t sighash, value_cref a) {
    sp.push(obj);
    r.set_callee_as_top();
    sp.push(a);
    r._vcall(sighash, 0, sp);
    return sp.pop();
}
value exe_context::call(const value &obj, uint32_t sighash) {
    sp.push(obj);
    r.set_callee_as_top();
    r._vcall(sighash, 0, sp);
    return sp.pop();
}
value exe_context::call(const value &obj, const std::wstring &name) {
    sp.push(obj);
    r.set_callee_as_top();
    r._vcall(sig2hash(name), 0, sp);
    return sp.pop();
}
value exe_context::next_param() {
    return sp.pop();
}

program_entry *exe_context::get_caller_entry(int offset) {
    return r.callstack[r.callstack.size() - offset - 1].entry;
}
gc &exe_context::gc() {
    return r.gc;
}
debugger *exe_context::debugger() {
    return r.dbger;
}