#pragma once

#include <algorithm>
#include <string>
#include <map>

#include "binding.h"
#include "string.h"
#include "object.h"

class rkdictionary : public rkobject<rkdictionary> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"dictionary");

        type.method(rk_id_new, create_array);

        method(type, rk_id_getitem, &rkdictionary::get);
        method(type, rk_id_setitem, &rkdictionary::set);
        method(type, rk_id_tostring, &rkdictionary::to_string);
        method(type, L"size", &rkdictionary::size);
        method(type, L"contains", &rkdictionary::contains);
        method(type, L"remove", &rkdictionary::remove);

        b.add_type(type);
    }

    rkdictionary(int n) {
        for (int i = 0; i < n; i++) {
            value v = rkctx()->next_param();
            value k = rkctx()->next_param();
            set(k, v);
        }
    }

    value static create_array(value_cref idx) {
        return value::mkobjref(new rkdictionary(0));
    }

    value get(value_cref key) {
        auto h = rkwstr(key);
        return dic[h];
    }
    value set(value_cref key, value_cref v) {
        auto h = rkwstr(key);
        dic[h] = v;
        return rknull;
    }
    value to_string() {
        std::wstring str;

        str += L"{";
        for (auto &v : dic) {
            auto &key = v.first;
            auto &value = v.second;

            if (v != *dic.begin())
                str += L", ";

            str += key + L" : ";
            str += rk_towstring(value);
        }
        str += L"}";

        return value::mkstring2(str.c_str());
    }
    
    value size() {
        return value::mkinteger(dic.size());
    }
    value contains(value_cref key) {
        auto h = rkwstr(key);
        if (dic.find(h) == dic.end())
            return value::_false();
        return value::_true();
    }
    value remove(value_cref key) {
        auto h = rkwstr(key);
        if (dic.erase(h))
            return value::_true();
        return value::_false();
    }

private:
    std::map<std::wstring, value> dic;
};