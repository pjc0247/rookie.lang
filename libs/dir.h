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

    static void _import(binding &b) {
        auto type = type_builder(L"dir");

        static_method(type, L"create", create);
        static_method(type, L"exists", exists);
        static_method(type, L"remove", remove);
        static_method(type, L"glob", glob);

        b.add_type(type);
    }

    static value create(const std::wstring &path) {
#if !defined(RK_ENV_WEB) && !defined(RK_NO_IO)
        return value::mkboolean(fs::create_directory(path));
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value exists(const std::wstring &path) {
#if !defined(RK_ENV_WEB) && !defined(RK_NO_IO)
        return value::mkboolean(fs::is_directory(path));
#endif
        throw e::not_avaliable_in_this_platform();
    }
    static value remove(const std::wstring &path) {
#if !defined(RK_ENV_WEB) && !defined(RK_NO_IO)
        if (fs::is_directory(path)) {
            return fs::remove(path) ? rktrue : rkfalse;
        }
        return rkfalse;
#endif
        throw e::not_avaliable_in_this_platform();
    }

    // FIXME : this is not a `glob` actually.
    static value glob(const std::wstring &path) {
#if !defined(RK_ENV_WEB) && !defined(RK_NO_IO)
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