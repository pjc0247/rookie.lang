#include "stdafx.h"

#include "libs/string.h"

#include "value_object.h"

value value::mkstring2(const wchar_t *str) {
    return rkctx()->newobj(L"string", value::mkstring(str));
}