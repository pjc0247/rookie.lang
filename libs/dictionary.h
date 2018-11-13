#pragma once

#include <string>
#include <map>

#include "binding.h"

class rkdictionary : public rkobject<rkdictionary> {
public:
    TYPENAME(L"dictionary")

    static void import(binding &b);

    rkdictionary(int n);

    static value create_dictionary(value_cref idx);

    value get(value_cref key);
    value set(value_cref key, value_cref v);
    value to_string();
    
    value size();
    value contains(value_cref key);
    value clear();
    value remove(value_cref key);

    value get_iterator();

    std::map<std::wstring, value>::iterator begin();
    std::map<std::wstring, value>::iterator end();

    virtual void gc_visit(gc_mark_func mark);

private:
    std::map<std::wstring, value> dic;
};