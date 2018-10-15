#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "value_object.h"

class rkstring : public rkobject<rkstring> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"string");

        type.method(L"new", create_instance2);

        method(type, L"at", &rkstring::at);
        method(type, L"length", &rkstring::length);

        method(type, L"append", &rkstring::append);
        method(type, L"to_string", &rkstring::to_string);

        b.add_type(type);
    }

    static value create_instance2(value &str) {
        auto ptr = new rkstring();
        ptr->str = str.str;
        return value::mkobjref(ptr);
    }
    value at(value &idx) {
        return value::mkchar(str[rkint(idx)]);
    }
    value length() {
        return value::mkinteger(str.size());
    }
    value append(value &other) {
        auto r = value::mkstring((str + other.str).c_str());
        return create_instance2(r);
    }

    value to_string() {
        return value::mkstring(str.c_str());
    }

    const wchar_t *c_str() const {
        return str.c_str();
    }

private:
    std::wstring str;
};