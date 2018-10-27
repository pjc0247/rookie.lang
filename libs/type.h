#pragma once

#include <algorithm>
#include <string>
#include <map>

#include "binding.h"
#include "runner.h"

#include "string.h"
#include "object.h"

class rktype : public rkobject<rktype> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"type");

        b.add_type(type);
    }

    rktype(uint32_t sighash, runtime_typedata &rtype) :
        sighash(sighash),
        rtype(rtype) {

    }

public:
    uint32_t sighash;
    runtime_typedata &rtype;
};