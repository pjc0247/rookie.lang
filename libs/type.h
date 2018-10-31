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
    TYPENAME(L"type")

    static void import(binding &b) {
        auto type = type_builder(L"type");

        method(type, rk_id_tostring, &rktype::to_string);
        method(type, L"fields", &rktype::fields);
        method(type, L"ancestors", &rktype::ancestors);

        b.add_type(type);
    }

    rktype(uint32_t sighash, runtime_typedata &rtype) :
        sighash(sighash),
        rtype(rtype) {
    }

    value to_string() {
        std::wstring str;
        str = L"<#type " + rtype.name + L">";
        return str2rk(str);
    }
    value fields() {
        auto ary = new rkarray();
        for (auto field : rtype.fields) {
            ary->push(int2rk(field.first));
        }
        
        return obj2rk(ary);
    }
    value ancestors() {
        auto ary = new rkarray();
        for (auto p : rtype.reflection->parents) {
            ary->push(str2rk(p->name));
        }

        return obj2rk(ary);
    }

public:
    uint32_t sighash;
    runtime_typedata &rtype;
};