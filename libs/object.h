#pragma once

#include <algorithm>

#include "binding.h"
#include "value_object.h"

#include "array.h"

template <typename T>
class rkobject : public rkobjectbase<T> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"object");

        type.method(L"new", rkobjectbase<T>::create_object);

        method(type, L"properties", &rkobject::properties);

        b.add_type(type);
    }

    value properties() {
        return value::mkinteger(1);
    }

private:
    std::vector<value> ary;
};

class rkscriptobject : public rkobject<rkscriptobject> {};