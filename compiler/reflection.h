#pragma once

#include "value.h"
#include "object.h"

#define rktype(name) sig2hash(name)

__forceinline bool rk_istypeof(uint32_t sighash, const value &v) {
    return v.type == value_type::object ?
        v.objref->sighash == sighash :
        false;
}
__forceinline bool rk_istypeof(const wchar_t *name, const value &v) {
    return rk_istypeof(sig2hash(name), v);
}