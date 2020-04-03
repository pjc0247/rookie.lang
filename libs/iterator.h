#pragma once

#include <string>
#include <functional>

#include "binding.h"
#include "object.h"
#include "kvpair.h"

class rkyield_iterator : public rkobject<rkyield_iterator> {
public:
    TYPENAME(L"yield_iterator");

    static void _import(binding &b) {
        auto type = type_builder(L"yield_iterator");

        method(type, L"current", &rkyield_iterator::current);
        method(type, L"move_next", &rkyield_iterator::move_next);

        b.add_type(type);
    }

    rkyield_iterator(const std::function<void()> &func) {

    }

    value current() {
        return rknull;
    }
    value move_next() {
        return rknull;
    }
};

class rkarray_iterator : public rkobject<rkarray_iterator> {
public:
    TYPENAME(L"array_iterator");

    static void _import(binding &b) {
        auto type = type_builder(L"array_iterator");

        method(type, L"current", &rkarray_iterator::current);
        method(type, L"move_next", &rkarray_iterator::move_next);

        b.add_type(type);
    }

    rkarray_iterator(std::vector<value> &ary) :
        aryref(ary) {
    }

    value current() {
        return aryref[ptr];
    }
    value move_next() {
        ptr++;
        return value::mkboolean(aryref.size() != ptr);
    }

private:
    std::vector<value> &aryref;
    int ptr = 0;
};

class rkdictionary_iterator : public rkobject<rkdictionary_iterator> {
public:
    TYPENAME(L"dictionary_iterator");

    static void _import(binding &b) {
        auto type = type_builder(L"dictionary_iterator");

        method(type, L"current", &rkdictionary_iterator::current);
        method(type, L"move_next", &rkdictionary_iterator::move_next);

        b.add_type(type);
    }

    rkdictionary_iterator(std::map<std::wstring, value> &dic) :
        dicref(dic) {

        it = dic.begin();
    }

    value current() {
        auto k = new rkkvpair(str2rk((*it).first), (*it).second);
        return obj2rk(k);
    }
    value move_next() {
        ++it;
        return value::mkboolean(it != dicref.end());
    }

private:
    std::map<std::wstring, value> &dicref;
    std::map<std::wstring, value>::iterator it;
};
