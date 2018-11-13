#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "stdfs.h"

#include "binding.h"
#include "object.h"
#include "array.h"
#include "exception.h"

#include "c_interface.h"

class rkdir : public rkobject<rkdir> {
public:
    TYPENAME(L"dir")

    static void import(binding &b) {
        auto type = type_builder(L"dir");

        static_method(type, L"size", create);
        static_method(type, L"exists", exists);
        static_method(type, L"glob", glob);

        b.add_type(type);
    }

    static value create(const std::wstring &path) {
#ifndef RK_ENV_WEB
        return value::mkboolean(fs::create_directory(path));
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value exists(const std::wstring &path) {
#ifndef RK_ENV_WEB
        return value::mkboolean(fs::is_directory(path));
#endif
        throw e::not_avaliable_in_this_platform();
    }

    // FIXME : this is not a `glob` actually.
    static value glob(const std::wstring &path) {
#ifndef RK_ENV_WEB
        _begin_no_gc();

        auto ary = new rkarray();
        for (auto &&x : fs::recursive_directory_iterator(path)) {
            ary->push(str2rk(x.path()));
        }

        _end_no_gc();
        return obj2rk(ary);
#endif
        throw e::not_avaliable_in_this_platform();
    }
};