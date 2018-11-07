#include "stdafx.h"

#include "sig2hash.h"
#include "array.h"

#include "json.h"

void rkjson::import(binding &b) {
    auto type = type_builder(L"json");

    static_method(type, L"stringify", stringify);
    static_method(type, L"parse", parse);

    b.add_type(type);
}

value rkjson::stringify(value_cref obj) {
    auto wstr = _stringify(obj);
    return str2rk(wstr);
}
value rkjson::parse() {
    return rknull;
}

std::wstring rkjson::_stringify(value_cref obj) {
    if (obj.type == value_type::integer)
        return std::to_wstring(rk2int(obj));
    else if (obj.type == value_type::boolean)
        return rk2int(obj) ? L"true" : L"false";
    else if (obj.type == value_type::object) {
        if (obj.objref->sighash == sighash_array) {
            auto ary = rk2obj(obj, rkarray*);

            std::wstring str = L"[";
            for (auto &v : *ary) {
                str += _stringify(v);
                str += L",";
            }
            str.pop_back();
            str += L"]";

            return str;
        }
    }
}