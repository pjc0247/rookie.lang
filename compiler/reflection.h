#pragma once

#include "value.h"
#include "object.h"
#include "sig2hash.h"

#define rktype(name) sig2hash(name)

__forceinline bool rk_istypeof(value_cref v, uint32_t sighash) {
    return v.type == value_type::object ?
        v.objref->sighash == sighash :
        false;
}
__forceinline bool rk_istypeof(value_cref v, const wchar_t *name) {
    return rk_istypeof(sig2hash(name), v);
}