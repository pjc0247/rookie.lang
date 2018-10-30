#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkiterator : public rkobject<rkiterator> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"iterator");

        method(type, L"current", &rkiterator::current);
        method(type, L"move_next", &rkiterator::move_next);

        b.add_type(type);
    }

    rkiterator(std::vector<value> &ary) :
        aryref(ary) {
    }

    value current() {
        return aryref[ptr];
    }
    value move_next() {
        ptr++;
        return value::mkboolean(aryref.size() != ptr);
    }

private:
    std::vector<value> &aryref;
    int ptr = 0;
};