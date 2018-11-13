#pragma once

#include <string>

#include "binding.h"
#include "string.h"
#include "object.h"

#include "runner.h"

class rkcaller : public rkobject<rkcaller> {
public:
    TYPENAME(L"caller")

    static void import(binding &b) {
        auto type = type_builder(L"caller");

        method(type, L"name", &rkcaller::get_name);

        b.add_type(type);

        b.function(L"caller", []() {
            auto c = new rkcaller(rkctx()->get_caller_entry(1));
            return obj2rk(c);
        });
    }

    rkcaller(program_entry *entry) :
        entry(entry) {

        set_property(rkid(L"name"), str2rk(entry->signature));
    }

    value get_name() {
        return str2rk(entry->signature);
    }

private:
    program_entry *entry;
};