#pragma once

#include <algorithm>

#include "binding.h"
#include "object.h"
#include "conout.h"

class rkdebugger : public rkobject<rkdebugger> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"debugger");

        static_method(type, L"dumpstack", dumpstack);

        b.add_type(type);
    }

    static value dumpstack() {
        auto dbger = rkctx()->debugger();

        if (dbger == nullptr)
            printf("No active debuggers. \n");

        dbger->dumpstack();

        return rknull;
    }
};