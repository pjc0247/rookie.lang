#pragma once

#include <algorithm>
#include <string>
#include <map>

#include "binding.h"
#include "string.h"
#include "object.h"

#include "iterator.h"

class rkdictionary : public rkobject<rkdictionary> {
public:
    TYPENAME(L"dictionary")

    static void import(binding &b) {
        auto type = type_builder(L"dictionary");

        type.method(rk_id_new, create_array);

        method(type, rk_id_getitem, &rkdictionary::get);
        method(type, rk_id_setitem, &rkdictionary::set);
        method(type, rk_id_tostring, &rkdictionary::to_string);
        method(type, L"size", &rkdictionary::size);
        method(type, L"contains", &rkdictionary::contains);
        method(type, L"clear", &rkdictionary::clear);
        method(type, L"remove", &rkdictionary::remove);

        method(type, L"get_iterator", &rkdictionary::get_iterator);

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
            str += rk_call_tostring(value);
        }
        str += L"}";

        return str2rk(str);
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
    value clear() {
        dic.clear();
        return rknull;
    }
    value remove(value_cref key) {
        auto h = rkwstr(key);
        if (dic.erase(h))
            return value::_true();
        return value::_false();
    }

    value get_iterator() {
        auto it = new rkdictionary_iterator(dic);
        return obj2rk(it);
    }

    std::map<std::wstring, value>::iterator begin() {
        return dic.begin();
    }
    std::map<std::wstring, value>::iterator end() {
        return dic.end();
    }

private:
    std::map<std::wstring, value> dic;
};