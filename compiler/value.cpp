#include "stdafx.h"

#include "libs/string.h"

#include "value.h"

value value::mkstring2(const wchar_t *str) {
    return rkctx()->newobj(L"string", value::mkstring(str));
}
value value::mkstring2(const std::wstring &str) {
    return rkctx()->newobj(L"string", value::mkstring(str.c_str()));
}