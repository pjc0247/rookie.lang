#pragma once

#include "binding.h"

class rkscriptobject : public rkobject<rkscriptobject> {
public:
    static void import(binding &b);

    value all_properties() {
        return value::mkinteger(1);
    }
    value set_property(const std::wstring &key, value_cref value);
    value get_property(const std::wstring &key);
    value has_property(const std::wstring &key);

    value to_string();
};