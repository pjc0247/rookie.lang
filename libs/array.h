#pragma once

#include <algorithm>

#include "binding.h"

class rkarray : public rkobject<rkarray> {
public:
    TYPENAME(L"array")

    static void import(binding &b);

    rkarray();
    rkarray(int n);
    
    value static create_array(const value &idx);

    value get(uint32_t idx);
    value set(uint32_t idx, value_cref v);

    value to_string();

    value op_add(value_cref other);
    value equal(value_cref other);

    value push(value_cref v);
    value clear();
    value remove(value_cref v);
    value remove_at(uint32_t v);
    value length();
    value reverse();

    value join(const std::wstring &delim);

    value get_iterator();

    std::vector<value>::iterator begin();
    std::vector<value>::iterator end();

    void gc_visit(gc_mark_func mark);

private:
    std::vector<value> ary;
};