#pragma once

#include "binding.h"

class rkscriptobject : public rkobject<rkscriptobject> {
public:
    static void _import(binding &b);

    value all_properties();
    value set_property(const std::wstring &key, value_cref value);
    value get_property(const std::wstring &key);
    value has_property(const std::wstring &key);

    value type();

    value equals(value_cref other);
    value to_string();
};