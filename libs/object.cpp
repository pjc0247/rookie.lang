#include "stdafx.h"

#include "string.h"
#include "sig2hash.h"

#include "object.h"

value rkscriptobject::set_property(value_cref key, value_cref value) {
    auto _key = rkwstr(key);
    this->properties[sig2hash(_key)] = value;
    return rknull;
}
value rkscriptobject::get_property(value_cref key) {
    auto _key = rkwstr(key);
    return this->properties[sig2hash(_key)];
}

value rkscriptobject::to_string() {
    std::wstring str = L"#<";
    str += name_ptr;

    for (auto &p : properties) {
        str += L" @" + rk_id2str(p.first) + L": ";
        str += rk_call_tostring_w(p.second);
    }

    str += L">";
    return str2rk(str);
}