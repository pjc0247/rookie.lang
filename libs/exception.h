#pragma once

#include "binding.h"
#include "object.h"

class rkexception : public rkobject<rkexception> {
public:
    TYPENAME(L"exception")

    static void import(binding &b) {
        auto type = type_builder(L"exception");

        b.add_type(type);
    }
};