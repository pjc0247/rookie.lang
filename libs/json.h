#pragma once

#include "binding.h"
#include "object.h"

class rkjson : public rkobject<rkjson> {
public:
    TYPENAME(L"json")

    static void import(binding &b);

    // AnyObject -> String
    static value stringify(value_cref obj);
    // String -> Object
    static value parse(value_cref);

private:
    static std::wstring _stringify(value_cref obj);
    static std::wstring _stringify_array(value_cref obj);
    static std::wstring _stringify_dictionary(value_cref obj);
    static std::wstring _stringify_object(value_cref obj);
};