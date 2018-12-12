#include "stdafx.h"

#include "thirdparty/json.hpp"
#include "string.h"
#include "array.h"
#include "dictionary.h"
#include "exception.h"

#include "json.h"

using json = nlohmann::json;

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
static value _parse(json &j) {
    if (j.type() == json::value_t::null)
        return rknull;
    if (j.type() == json::value_t::boolean)
        return j.get<bool>() ? rktrue : rkfalse;
    if (j.type() == json::value_t::string)
        return str2rk(str2wstr(j.get<std::string>().c_str()));
    if (j.type() == json::value_t::number_integer)
        return int2rk(j.get<int32_t>());
    if (j.type() == json::value_t::number_unsigned)
        return uint2rk(j.get<uint32_t>());

    if (j.type() == json::value_t::object) {
        auto obj = new rkjson();

        for (auto &item : j.items()) {
            obj->set_property(sig2hash(str2wstr(item.key().c_str())), _parse(item.value()));
        }

        return obj2rk(obj);
    }
    if (j.type() == json::value_t::array) {
        auto ary = new rkarray();

        for (auto &item : j) {
            ary->push(_parse(item));
        }

        return obj2rk(ary);
    }
}
value rkjson::parse(const std::wstring &str) {
    auto j = json::parse(str);
    return _parse(j);
}
std::wstring rkjson::_stringify(value_cref obj) {
	if (obj.type == value_type::integer)
		return std::to_wstring(rk2int(obj));
	else if (obj.type == value_type::boolean)
		return rk2int(obj) ? L"true" : L"false";
	else if (obj.type == value_type::object) {
		if (obj.objref->sighash == sighash_array)
			return _stringify_array(obj);
		else if (obj.objref->sighash == sighash_dictionary)
			return _stringify_dictionary(obj);
		else if (obj.objref->sighash == sighash_string)
			return L"\"" + rkwstr(obj) + L"\"";
		else
			return _stringify_object(obj);
	}
	else
		throw new rkexception("Not Supported");
}
std::wstring rkjson::_stringify_array(value_cref obj) {
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
std::wstring rkjson::_stringify_dictionary(value_cref obj) {
    auto dic = rk2obj(obj, rkdictionary*);

    std::wstring str = L"{";
    for (auto &v : *dic) {
        str += L"\"" + v.first + L"\"";
        str += L":" + _stringify(v.second);
        str += L",";
    }
    str.pop_back();
    str += L"}";

    return str;
}
std::wstring rkjson::_stringify_object(value_cref obj) {
    std::wstring str = L"{";
    for (auto &p : obj.objref->properties) {
        str += L"\"" + rk_id2wstr(p.first) + L"\"";
        str += L":" + _stringify(p.second);
        str += L",";
    }
    str.pop_back();
    str += L"}";

    return str;
}