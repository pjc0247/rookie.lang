#include "stdafx.h"

#include "array.h"

void rkarray::import(binding &b) {
    auto type = type_builder(L"array");

    type.method(rk_id_new, create_array);

    method(type, rk_id_getitem, &rkarray::get);
    method(type, rk_id_setitem, &rkarray::set);
    method(type, rk_id_tostring, &rkarray::to_string);

    method(type, L"__add__", &rkarray::op_add);

    method(type, L"at", &rkarray::get);
    method(type, L"push", &rkarray::push);
    method(type, L"clear", &rkarray::clear);
    method(type, L"remove", &rkarray::remove);
    method(type, L"remove_at", &rkarray::remove);
    method(type, L"length", &rkarray::length);
    method(type, L"reverse", &rkarray::reverse);
    method(type, L"equal", &rkarray::equal);

    method(type, L"get_iterator", &rkarray::get_iterator);

    b.add_type(type);
}

rkarray::rkarray() {
}
rkarray::rkarray(int n) {
    for (int i = 0; i < n; i++) {
        value v = rkctx()->next_param();
        push(v);
    }
}

value rkarray::create_array(const value &idx) {
    return value::mkobjref(new rkarray(0));
}

value rkarray::get(value_cref idx) {
    return ary[rkint(idx)];
}
value rkarray::set(value_cref idx, value_cref v) {
    ary[rkint(idx)] = v;
    return rknull;
}
value rkarray::to_string() {
    std::wstring str;

    str += L"[";
    for (auto it = ary.begin(); it != ary.end(); ++it) {
        if (it != ary.begin())
            str += L", ";

        str += rk_call_tostring(*it);
    }
    str += L"]";

    return str2rk(str);
}

value rkarray::op_add(value_cref other) {
    auto new_ary = new rkarray();

    // FIXME
    if (other.objref->sighash == sig2hash(L"array")) {
        auto other_ary = rk2obj(other, rkarray*);

        new_ary->ary.insert(
            new_ary->ary.end(),
            ary.begin(), ary.end());
        new_ary->ary.insert(
            new_ary->ary.end(),
            other_ary->ary.begin(), other_ary->ary.end());
    }

    return obj2rk(new_ary);
}
value rkarray::equal(value_cref other) {
    auto other_ary = rk2obj(other, rkarray*);

    if (other_ary->ary.size() != ary.size())
        return rkfalse;

    for (int i = 0; i<ary.size(); i++) {
        if (ary[i] != other_ary->ary[i])
            return rkfalse;
    }

    return rktrue;
}

value rkarray::push(value_cref v) {
    ary.push_back(v);
    return rknull;
}
value rkarray::clear() {
    ary.clear();
    return rknull;
}
value rkarray::remove(value_cref v) {
    ary.erase(std::remove(
        ary.begin(), ary.end(), v),
        ary.end());
    return rknull;
}
value rkarray::remove_at(value_cref v) {
    ary.erase(ary.begin() + rk2int(v));
    return rknull;
}
value rkarray::length() {
    return int2rk(ary.size());
}
value rkarray::reverse() {
    auto new_ary = new rkarray();

    for (int i = ary.size() - 1; i >= 0; i--)
        new_ary->push(ary[i]);

    return obj2rk(new_ary);
}

value rkarray::get_iterator() {
    auto it = new rkarray_iterator(ary);
    return obj2rk(it);
}

std::vector<value>::iterator rkarray::begin() {
    return ary.begin();
}
std::vector<value>::iterator rkarray::end() {
    return ary.end();
}