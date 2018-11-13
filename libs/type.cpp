#include "stdafx.h"

#include "array.h"

#include "type.h"

void rktype::import(binding &b) {
    auto type = type_builder(L"type");

    method(type, L"fields", &rktype::fields);
    method(type, L"methods", &rktype::methods);
    method(type, L"ancestors", &rktype::ancestors);

    method(type, rk_id_tostring, &rktype::to_string);

    b.add_type(type);
}

rktype::rktype(uint32_t sighash, runtime_typedata &rtype) :
    sighash(sighash),
    rtype(rtype) {
}
value rktype::fields() {
    auto ary = new rkarray();
    for (auto field : rtype.fields) {
        ary->push(rk_id2str(field.first));
    }

    return obj2rk(ary);
}
value rktype::methods() {
    auto ary = new rkarray();
    for (auto field : rtype.vtable) {
        ary->push(rk_id2str(field.first));
    }

    return obj2rk(ary);
}
value rktype::ancestors() {
    auto ary = new rkarray();
    for (auto p : rtype.reflection->parents) {
        ary->push(str2rk(p->name));
    }

    return obj2rk(ary);
}

value rktype::to_string() {
    std::wstring str;
    str = L"<#type " + rtype.name + L">";
    return str2rk(str);
}