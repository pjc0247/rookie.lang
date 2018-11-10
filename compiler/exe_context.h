#pragma once

#include <string>

#include "stack_provider.h"
#include "debugger.h"
#include "gc.h"

class  runner;
struct value;

class  rktype;

class exe_context {
public:
    exe_context(runner &r, stack_provider &sp);

    const std::wstring &get_name(uint32_t sighash);

    value init_obj(uint32_t sighash, object *obj);
    value init_obj_nogc(uint32_t sighash, object *obj);

    template<typename... ArgTypes>
    value newobj(const std::wstring &name, ArgTypes... args);
    template<typename T, typename... ArgTypes>
    value newobj(const std::wstring &name, T t, ArgTypes... args) {
        sp.push(t);
        return newobj(name, args...);
    }

    void push_newobj(const std::wstring &name);

    rktype *get_type(const std::wstring &name);

    value &get_this();
    value call(const value &obj, uint32_t sighash);
    value call(const value &obj, const std::wstring &name);

    value next_param();

    gc &gc();
    debugger *debugger();

private:
    runner &r;
    stack_provider &sp;
};

extern "C" { 
    exe_context *rkctx();
}
void set_rkctx(exe_context *exe);

