#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkstring : public rkobject<rkstring> {
public:
    TYPENAME(L"string")

    static void import(binding &b) {
        auto type = type_builder(L"string");

        type.method(L"new", create_instance);

        method(type, rk_id_getitem, &rkstring::at);

        method(type, L"at", &rkstring::at);
        method(type, L"length", &rkstring::length);
        method(type, L"starts_with", &rkstring::starts_with);
        method(type, L"ends_with", &rkstring::ends_with);

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

    static value create_instance(value_cref str) {
        auto ptr = new rkstring();
        ptr->str = str.str;

        return obj2rk(ptr);
    }
    value at(uint32_t idx) {
        return value::mkchar(str[idx]);
    }
    value length() {
        return value::mkinteger(str.size());
    }
    value starts_with(value_cref other) {
        auto wstr = rkwstr(other);
        if (wstr.length() > str.length())
            return rkfalse;
        for (int i = 0; i < wstr.length(); i++) {
            if (wstr[i] != str[i])
                return rkfalse;
        }
        return rktrue;
    }
    value ends_with(value_cref other) {
        auto wstr = rkwstr(other);
        auto strlen = str.length();
        auto wstrlen = wstr.length();

        if (wstrlen > strlen)
            return rkfalse;
        for (int i = 0; i < wstrlen; i++) {
            if (wstr[wstrlen - i - 1] != str[strlen - i - 1])
                return rkfalse;
        }
        return rktrue;
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