#pragma once

#include <string>

#include "binding.h"
#include "object.h"

#include "string.h"

class rkexception : public rkobject<rkexception> {
public:
    TYPENAME(L"exception")

    static void _import(binding &b) {
        auto type = type_builder(L"exception");

        type.method(rk_id_new, create_object);

        method(type, rk_id_tostring, &rkexception::to_string);

        b.add_type(type);
    }

    static value create_object(value_cref msg) {
        return value::mkobjref(new rkexception(rkwstr(msg)));
    }

    rkexception(const base_exception &ex) {
        msg = str2wstr(ex.what());
    }
    rkexception(const char *msg) :
        msg(str2wstr(msg)) {
    }
    rkexception(const std::wstring &msg) :
        msg(msg) {
    }

    void set_callstack(const std::deque<callframe> &cs) {
        callstack = cs;
    }

    const std::wstring &what() {
        return msg;
    }

    value to_string() {
        std::wstring str;
        str = L"#<exception: " + msg + L">\r\n";
        str += L"[[STACKTRACE]]\r\n";
        for (int i=callstack.size()-1; i>= 0; i--) {
            auto &c = callstack[i];
            str += L"   * " + std::wstring(c.entry->signature) + L"\r\n";
        }
        return str2rk(str);
    }

private:
    std::wstring msg;

    std::deque<callframe> callstack;
};

class null_pointer_exception : public rkexception {
public:
    null_pointer_exception() :
        rkexception("Nullpointer exception") {
    }
    null_pointer_exception(const std::wstring &name) :
        rkexception(L"Nullpointer exception. (null." + name + L")") {
    }
};
class out_of_range_exception : public rkexception {
public:
    out_of_range_exception() :
        rkexception("out of range") {
    }
    out_of_range_exception(int given) :
        rkexception(L"out of range (given: " + std::to_wstring(given) + L")") {
    }
};
class argument_exception : public rkexception {
public:
    argument_exception() :
        rkexception("Argument is invalid") {
    }
    argument_exception(const std::wstring &msg) :
        rkexception(msg) {
    }
};
class memory_overflow_exception : public rkexception {
public:
    memory_overflow_exception() :
        rkexception("memory_overflow_exception") {
    }
};
class method_not_found_exception : public rkexception {
public:
    method_not_found_exception(const std::wstring &given_name) :
        given_name(given_name),
        rkexception(L"No such method: " + given_name) {
    }

public:
    std::wstring given_name;
}; 

class not_implemented_exception : public rkexception {
public:
    not_implemented_exception() :
        rkexception("Method is not implemented.") {
    }
    not_implemented_exception(const std::wstring &msg) :
        rkexception(msg) {
    }
};

class overflow_exception : public rkexception {
public:
    overflow_exception() :
        rkexception("Operation overflows") {
    }
};
class divide_by_zero_exception : public rkexception {
public:
    divide_by_zero_exception() :
        rkexception("Divide by zero") {
    }
};

// Exception shortcuts
class e {
public:
    static not_implemented_exception *not_avaliable_in_this_platform() {
        return new not_implemented_exception(L"Method is not avaliable in this platform.");
    }
};