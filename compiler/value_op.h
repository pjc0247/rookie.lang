#pragma once

#include "sig2hash.h"
#include "exe_context.h"

inline value operator+(const value &a, const value &b) {
    return value::mkinteger(a.integer + b.integer);
}
inline value operator-(const value &a, const value &b) {
    return value::mkinteger(a.integer - b.integer);
}
inline value operator*(const value &a, const value &b) {
    return value::mkinteger(a.integer * b.integer);
}
inline value operator/(const value &a, const value &b) {
    return value::mkinteger(a.integer / b.integer);
}

inline bool operator==(const value& lhs, object *rhs) {
    if (rhs == nullptr &&
        (lhs.type == value_type::null || lhs.type == value_type::empty))
        return true;

    if (lhs.objref == rhs)
        return true;

    // In this case, `rhs` doesn't need to be fully initialized.
    auto rhsobj = value::mkobjref(rhs);
    auto r = rkctx()->vcall(lhs, sig2hash_c(L"equal"), rhsobj);

    if (r.integer) return true;
    return false;
}
inline bool operator==(const value& lhs, const value& rhs) {
    if (lhs.type != rhs.type)
        return false;

    if (lhs.type == value_type::integer)
        return lhs.integer == rhs.integer;
    if (lhs.type == value_type::boolean)
        return lhs.integer == rhs.integer;
    if (lhs.type == value_type::string)
        return lhs.str == rhs.str;
    if (lhs.type == value_type::object) {
        if (lhs.objref == rhs.objref)
            return true;

        auto r = rkctx()->vcall(lhs, sig2hash_c(L"equal"), rhs);

        if (r.integer) return true;
        return false;
    }

    throw base_exception("unimplemented operator");
}
inline bool operator!=(const value& lhs, const value& rhs) {
    if (lhs.type != rhs.type)
        return true;
    return !(lhs == rhs);
}