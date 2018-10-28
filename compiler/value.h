#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>

#include "errors.h"

enum class call_type {
    ct_programcall_direct,
    ct_programcall_name,
    ct_syscall_direct,
    et_syscall_name
};
struct callinfo {
    int sighash;

    call_type type;
    int entry;
};
class calltable_builder {
public:
    void add_programcall(const std::wstring &signature, int entry) {

    }
    int add_syscall(const std::wstring &signature) {
        int entry = table.size();

        callinfo ci;
        ci.entry = entry;
        ci.type = call_type::ct_syscall_direct;
        table.push_back(ci);

        lookup[signature] = entry;
        return entry;
    }

    bool try_get(const std::wstring &signature, callinfo &callinfo) {
        auto it = lookup.find(signature);
        if (it == lookup.end()) return false;
        callinfo = table[(*it).second];
        return true;
    }
    callinfo &get(int index) {
        return table[index];
    }

private:
    std::map<std::wstring, int> lookup;
    std::vector<callinfo> table;
};
struct calltable : public std::map<uint32_t, callinfo> {
};
class stack_provider;
struct syscalltable {
    std::vector<std::function<void(stack_provider&)>> table;
};

enum class value_type : char {
    empty,
    null,
    boolean,
    integer, character, string, object, array
};

typedef struct object;
typedef struct program_entry;

struct callframe {
    program_entry *entry;
    short pc;
    short bp;

    callframe(short pc, short bp, program_entry *entry) :
        pc(pc), bp(bp), entry(entry) {
    }
};

struct value {
    value_type type;

    union {
        object *objref;

        int integer;
        unsigned int uinteger;
        wchar_t character;
        const wchar_t *str;
    };

    value();
    value(value_type type);

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
};

const value rknull = value(value_type::null);
const value rkfalse = value::_false();
const value rktrue = value::_true();

#include "value_op.h"