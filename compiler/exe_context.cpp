#include "stdafx.h"

#include "binding.h"
#include "runner.h"
#include "value_object.h"

#include "exe_context.h"

thread_local exe_context *rkctx = nullptr;

exe_context::exe_context(runner &r, stack_provider &sp) :
    r(r), sp(sp) {
}
value exe_context::call(value &obj, const std::wstring &name) {
    sp.push(obj);
    r._vcall(sig2hash(name), sp);
    return sp.pop();
}
value exe_context::next_param() {
    return sp.pop();
}