#pragma once

#include <string>

#include "binding.h"

class rktype : public rkobject<rktype> {
public:
    TYPENAME(L"type")

    static void import(binding &b);

    rktype(uint32_t sighash, runtime_typedata &rtype);
    
    value fields();
    value methods();
    value ancestors();

    value to_string();

public:
    uint32_t sighash;
    runtime_typedata &rtype;
};