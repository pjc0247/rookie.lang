#include "stdafx.h"

#include "exception.h"

#include "string.h"

void rkstring::_import(binding &b) {
    auto type = type_builder(L"string");

    type.method(L"new", create_instance);

    method(type, rk_id_getitem, &rkstring::at);

    method(type, L"at", &rkstring::at);
    method(type, L"length", &rkstring::length);
    method(type, L"starts_with", &rkstring::starts_with);
    method(type, L"ends_with", &rkstring::ends_with);
    method(type, L"substr", &rkstring::substr);
    method(type, L"lower", &rkstring::lower);
    method(type, L"upper", &rkstring::upper);

    method(type, L"equal", &rkstring::equal);

    method(type, L"append", &rkstring::append);
    method(type, L"to_string", &rkstring::to_string);

    b.add_type(type);
}

rkstring::rkstring() {
}
rkstring::rkstring(const std::wstring &str) :
    str(str) {
}

value rkstring::create_instance(value_cref str) {
    auto ptr = new rkstring();
    ptr->str = str.str;

    return obj2rk(ptr);
}
value rkstring::at(uint32_t idx) {
    if (idx < 0 || idx >= str.length())
        throw new out_of_range_exception(idx);

    auto obj = new rkstring(str.substr(idx, 1));
    return obj2rk(obj);
}
value rkstring::length() {
    return int2rk(str.size());
}
value rkstring::contains(const std::wstring &str) {
    if (str.find(str) == std::wstring::npos)
        return rkfalse;
    return rktrue;
}
value rkstring::starts_with(value_cref other) {
    auto wstr = rkwstr(other);
    if (wstr.length() > str.length())
        return rkfalse;
    for (int i = 0; i < wstr.length(); i++) {
        if (wstr[i] != str[i])
            return rkfalse;
    }
    return rktrue;
}
value rkstring::ends_with(value_cref other) {
    auto wstr = rkwstr(other);
    auto strlen = str.length();
    auto wstrlen = wstr.length();

    if (wstrlen > strlen)
        return rkfalse;
    for (int i = 0; i < wstrlen; i++) {
        if (wstr[wstrlen - i - 1] != str[strlen - i - 1])
            return rkfalse;
    }
    return rktrue;
}

value rkstring::substr(int start, int len) {
    if (start < 0) throw new out_of_range_exception();
    if (len < 0 || len >= str.length()) throw new out_of_range_exception();

    auto new_str = str.substr(start, len);
    return str2rk(new_str);
}
value rkstring::lower() {
    auto new_str = str;
    std::transform(
        str.begin(), str.end(),
        new_str.begin(),
        ::tolower);
    return str2rk(new_str);
}
value rkstring::upper() {
    auto new_str = str;
    std::transform(
        str.begin(), str.end(),
        new_str.begin(),
        ::toupper);
    return str2rk(new_str);
}

value rkstring::equal(value_cref v) {
    return value::mkboolean(str == (rk2obj(v, rkstring*)->str));
}
value rkstring::append(value_cref other) {
    auto appended = (str + rk_call_tostring(other));
    auto r = value::mkstring(appended.c_str());
    return rkctx()->newobj(L"string", r);
}

value rkstring::to_string() {
    return value::mkobjref(this);
}