#pragma once

#include <map>
#include <vector>

#include "value.h"
#include "call.h"

#define TYPENAME(n) \
    virtual const wchar_t *type_name() { return n; }

typedef const std::function<void(value_cref)> & gc_mark_func;

class object {
public:
    virtual const wchar_t *type_name() { return L"object"; }

    __forceinline void set_property(uint32_t keyhash, const value &v) {
        properties[keyhash] = v;
    }
    __forceinline const value &get_property(uint32_t keyhash) {
        auto it = properties.find(keyhash);
        if (it == properties.end())
            return rknull;
        return (*it).second;
    }

    virtual void gc_visit(gc_mark_func mark) {
    }

public:
    const wchar_t *name_ptr;

    uint32_t sighash;

    std::map<uint32_t, callinfo> *vtable;
    std::map<uint32_t, value>    properties;
    std::vector<value>           gc_refs;
};