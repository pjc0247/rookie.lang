#include "stdafx.h"

#include "libs/string.h"

#include "value.h"

value::value() :
    type(value_type::empty) {
}
value::value(value_type type) :
    type(type) {
}

value value::mkstring2(const wchar_t *str) {
    return rkctx()->newobj(L"string", value::mkstring(str));
}