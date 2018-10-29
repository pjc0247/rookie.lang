#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkstring : public rkobject<rkstring> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"string");

        type.method(L"new", create_instance2);

        method(type, rk_getitem, &rkstring::at);

        method(type, L"at", &rkstring::at);
        method(type, L"length", &rkstring::length);
        method(type, L"length2", &rkstring::length2);

        method(type, L"equal", &rkstring::equal);

        method(type, L"append", &rkstring::append);
        method(type, L"to_string", &rkstring::to_string);

        b.add_type(type);
    }

    rkstring() {
    }
    rkstring(const std::wstring &str) :
        str(str) {
    }

    static value create_instance2(value_cref str) {
        auto ptr = new rkstring();
        ptr->str = str.str;
        return value::mkobjref(ptr);
    }
    value at(value_cref idx) {
        return value::mkchar(str[rkint(idx)]);
    }
    value length() {
        return value::mkinteger(str.size());
    }
    value length2() {
        return rkvcall(rkthis, L"length");
        //return value::mkinteger(str.size());
    }
    value equal(value_cref v) {
        return value::mkboolean(str == (rk2obj(v, rkstring*)->str));
    }
    value append(value_cref other) {
        auto appended = (str + rkwstr(other));
        auto r = value::mkstring(appended.c_str());
        return rkctx()->newobj(L"string", r);
    }

    value to_string() {
        return value::mkobjref(this);
    }

    const wchar_t *c_str() const {
        return str.c_str();
    }
    const std::wstring w_str() const {
        return str;
    }

private:
    std::wstring str;
};