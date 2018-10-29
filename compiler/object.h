#pragma once

#include <map>
#include <vector>

#include "value.h"
#include "call.h"

class object {
public:
    __forceinline void set_property(uint32_t keyhash, const value &v) {
        properties[keyhash] = v;
    }
    __forceinline const value &get_property(uint32_t keyhash) {
        auto it = properties.find(keyhash);
        if (it == properties.end())
            return rknull;
        return (*it).second;
    }

public:
    int sighash;

    std::map<uint32_t, callinfo> *vtable;
    std::map<uint32_t, value> properties;
    std::vector<value> gc_refs;
};