#include "stdafx.h"

#include "string.h"
#include "sig2hash.h"

#include "object.h"

void rkscriptobject::import(binding &b) {
    auto type = type_builder(L"object");

    method(type, L"properties", &rkscriptobject::all_properties);
    method(type, L"__set_prop", &rkscriptobject::set_property);
    method(type, L"__get_prop", &rkscriptobject::get_property);
    method(type, L"has_property", &rkscriptobject::has_property);
    method(type, rk_id_tostring, &rkscriptobject::to_string);

    b.add_type(type);
}

value rkscriptobject::set_property(const std::wstring &key, value_cref value) {
    this->properties[sig2hash(key)] = value;
    return rknull;
}
value rkscriptobject::get_property(const std::wstring &key) {
    return this->properties[sig2hash(key)];
}
value rkscriptobject::has_property(const std::wstring &key) {
    return value::mkboolean(properties.find(sig2hash(key)) != properties.end());
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