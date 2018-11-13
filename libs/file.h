#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "stdfs.h"

#include "binding.h"
#include "object.h"
#include "exception.h"

#include "c_interface.h"

class rkfile : public rkobject<rkfile> {
public:
    TYPENAME(L"file")

    static void import(binding &b) {
        auto type = type_builder(L"file");

        static_method(type, L"read_text", read_text);
        static_method(type, L"write_text", write_text);
        static_method(type, L"size", size);
        static_method(type, L"exists", exists);

        b.add_type(type);
    }

    static value read_text(const std::wstring &filename) {
#ifndef RK_ENV_WEB
        std::wifstream t(filename);
        std::wstringstream buffer;
        buffer << t.rdbuf();
        return str2rk(buffer.str());
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value write_text(const std::wstring &filename, value_cref obj) {
#ifndef RK_ENV_WEB
        auto content = rk_call_tostring_w(obj);

        std::wofstream t(filename);
        //t.write >> content;
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value size(const std::wstring &filename) {
#ifndef RK_ENV_WEB
        return int2rk(fs::file_size(filename));
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value exists(const std::wstring &filename) {
#ifndef RK_ENV_WEB
        return value::mkboolean(fs::exists(filename));
#endif
        throw e::not_avaliable_in_this_platform();
    }
};