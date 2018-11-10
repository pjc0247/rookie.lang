#pragma once

#include "dbg.h"

class runner;

class debugger {
public:
    debugger(pdb &pdb);

    void on_begin_program(runner &r, const program &p);
    void on_pre_exec(runner &r, const instruction &inst);

    void dumpstack();

private:
    runner *r;

    runtime_pdb pdb;
};