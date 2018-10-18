#include "stdafx.h"

#include "binding.h"
#include "runner.h"
#include "value_object.h"

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

template <>
value exe_context::newobj(const std::wstring &name) {
    push_newobj(name);
    return sp.pop();
}
void exe_context::push_newobj(const std::wstring &name) {
    r._newobj_systype(sig2hash(name), sp);
}

value exe_context::call(value &obj, const std::wstring &name) {
    sp.push(obj);
    r._vcall(sig2hash(name), sp);
    return sp.pop();
}
value exe_context::next_param() {
    return sp.pop();
}

gc &exe_context::gc() {
    return r.gc;
}
debugger *exe_context::debugger() {
    return r.dbger;
}