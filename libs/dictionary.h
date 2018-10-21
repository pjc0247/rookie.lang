#pragma once

#include <algorithm>
#include <string>
#include <map>

#include "binding.h"
#include "value_object.h"
#include "string.h"
#include "object.h"

class rkdictionary : public rkobject<rkdictionary> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"dictionary");

        type.method(L"new", create_array);

        method(type, L"__getitem__", &rkdictionary::get);
        method(type, L"__setitem__", &rkdictionary::set);
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

    value static create_array(value &idx) {
        return value::mkobjref(new rkdictionary(0));
    }

    value get(value &key) {
        auto h = sig2hash(rkwstr(key));
        return dic[h];
    }
    value set(value &key, value &v) {
        auto h = sig2hash(rkwstr(key));
        dic[h] = v;
        return rknull;
    }
    
    value size() {
        return value::mkinteger(dic.size());
    }
    value contains(value &key) {
        auto h = sig2hash(rkwstr(key));
        if (dic.find(h) == dic.end())
            return value::_false();
        return value::_true();
    }
    value remove(value &key) {
        auto h = sig2hash(rkwstr(key));
        if (dic.erase(h))
            return value::_true();
        return value::_false();
    }

private:
    std::map<uint32_t, value> dic;
};