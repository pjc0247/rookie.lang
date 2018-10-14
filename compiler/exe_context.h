#pragma once

#include <string>

class runner;
class stack_provider;
class value;

class exe_context {
public:
    exe_context(runner &r, stack_provider &sp);
    value call(value &obj, const std::string &name);

private:
    runner &r;
    stack_provider &sp;
};

extern thread_local exe_context *rkctx;