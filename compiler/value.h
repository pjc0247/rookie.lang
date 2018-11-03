#pragma once

#include <string>
#include <functional>

#include "errors.h"

enum class value_type : char {
    empty,
    null,
    boolean,
    integer, character, string, object, array
};

class object;
struct program_entry;

struct value {
    value_type type;

    union {
        object *objref;

        int integer;
        unsigned int uinteger;
        wchar_t character;
        const wchar_t *str;
    };

#if _DEBUG
    uint32_t ld_pc;
#endif

    static value empty() {
        value v;
        v.type = value_type::empty;
        v.objref = nullptr;
        return v;
    }
    static value null() {
        value v;
        v.type = value_type::null;
        v.objref = nullptr;
        return v;
    }
    static value _true() {
        value v;
        v.type = value_type::boolean;
        v.integer = 1;
        return v;
    }
    static value _false() {
        value v;
        v.type = value_type::boolean;
        v.integer = 0;
        return v;
    }
    static value mkboolean(bool b) {
        value v;
        v.type = value_type::boolean;
        v.integer = b ? 1 : 0;
        return v;
    }
    static value mkobjref(object *objref) {
        value v;
        v.type = value_type::object;
        v.objref = objref;
        return v;
    }
    static value mkinteger(int n) {
        value v;
        v.type = value_type::integer;
        v.integer = n;
        return v;
    }
    static value mkchar(wchar_t c) {
        value v;
        v.type = value_type::character;
        v.character = c;
        return v;
    }
    static value mkstring(const wchar_t *str) {
        value v;
        v.type = value_type::string;
        v.str = str;
        return v;
    }
    static value mkstring2(const wchar_t *str);
    static value mkstring2(const std::wstring &str);
};

const value rknull   = value::null();
const value rkempty  = value::empty();
const value rkfalse  = value::_false();
const value rktrue   = value::_true();

typedef const value & value_cref;

#include "value_op.h"