#pragma once

#include <string>

#include "binding.h"

class rkstring : public rkobject<rkstring> {
public:
    TYPENAME(L"string")

    static void import(binding &b);

    rkstring();
    rkstring(const std::wstring &str);

    static value create_instance(value_cref str);

    value at(uint32_t idx);
    value length();
    value contains(const std::wstring &str);
    value starts_with(value_cref other);
    value ends_with(value_cref other);
    value equal(value_cref v);
    value append(value_cref other);
    value to_string();

    inline const wchar_t *c_str() const {
        return str.c_str();
    }
    inline const std::wstring w_str() const {
        return str;
    }

private:
    std::wstring str;
};