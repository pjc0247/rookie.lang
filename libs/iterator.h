#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkarray_iterator : public rkobject<rkarray_iterator> {
public:
    static void import(binding &b) {
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
    static void import(binding &b) {
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
        return (*it).second;
    }
    value move_next() {
        ++it;
        return value::mkboolean(it != dicref.end());
    }

private:
    std::map<std::wstring, value> &dicref;
    std::map<std::wstring, value>::iterator it;
};