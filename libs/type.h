#pragma once

#include <algorithm>
#include <string>
#include <map>

#include "binding.h"
#include "runner.h"

#include "string.h"
#include "object.h"

class rktype : public rkobject<rktype> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"type");

        method(type, L"fields", &rktype::fields);

        b.add_type(type);
    }

    rktype(uint32_t sighash, runtime_typedata &rtype) :
        sighash(sighash),
        rtype(rtype) {
    }

    value fields() {
        auto ary = new rkarray(0);
        for (auto field : rtype.fields) {
            auto v = int2rk(field.first);
            ary->push(v);
        }
        
        return obj2rk(ary, L"array");
    }

public:
    uint32_t sighash;
    runtime_typedata &rtype;
};