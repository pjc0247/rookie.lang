#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "binding.h"
#include "object.h"

#include "c_interface.h"

class rkfile : public rkobject<rkfile> {
public:
    TYPENAME(L"file")

    static void import(binding &b) {
        auto type = type_builder(L"file");

        static_method(type, L"read_text", read_text);
        static_method(type, L"write_text", write_text);

        b.add_type(type);
    }

    static value read_text(value_cref filename) {
#ifndef RK_ENV_WEB
        std::wifstream t(rkwstr(filename));
        std::wstringstream buffer;
        buffer << t.rdbuf();
        return str2rk(buffer.str());
#endif
    }
    static value write_text(value_cref filename) {
        return rknull;
    }
};