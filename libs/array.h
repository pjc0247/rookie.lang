#pragma once

#include <algorithm>

#include "binding.h"
#include "object.h"
#include "string.h"

#include "iterator.h"

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

    value get_iterator();

    std::vector<value>::iterator begin();
    std::vector<value>::iterator end();

private:
    std::vector<value> ary;
};