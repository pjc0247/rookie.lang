#pragma once

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

inline bool operator==(const value& lhs, const void *rhs) {
    if (rhs == nullptr &&
        (lhs.type == value_type::null || lhs.type == value_type::empty))
        return true;
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
    if (lhs.type == value_type::object)
        return lhs.objref == rhs.objref;

    throw rkexception("unimplemented operator");
}