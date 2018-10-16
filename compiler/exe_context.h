#pragma once

#include <string>

#include "value_object.h"
#include "stack_provider.h"

class runner;
class value;

class exe_context {
public:
    exe_context(runner &r, stack_provider &sp);

    template<typename... ArgTypes>
    value newobj(const std::wstring &name, ArgTypes... args);
    template<typename T, typename... ArgTypes>
    value newobj(const std::wstring &name, T t, ArgTypes... args) {
        sp.push(t);
        return newobj(name, args...);
    }

    void push_newobj(const std::wstring &name);

    value call(value &obj, const std::wstring &name);

    value next_param();

private:
    runner &r;
    stack_provider &sp;
};

extern "C" { 
    exe_context *rkctx();
}
void set_rkctx(exe_context *exe);

