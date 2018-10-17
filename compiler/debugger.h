#pragma once

#include "value_object.h"
#include "stack_provider.h"

#include "dbg.h"

class runner;

class debugger {
public:
    debugger(pdb &pdb) :
        pdb(pdb) {
    }

    void on_begin_program(runner &r, const program &p);
    void on_pre_exec(runner &r, const instruction &inst);

private:
    runtime_pdb pdb;
};