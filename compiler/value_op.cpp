#include "stdafx.h"

#include "value_object.h"
#include "value_op.h"

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