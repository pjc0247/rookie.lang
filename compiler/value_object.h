#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>

#include "sig2hash.h"
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
struct calltable {
    std::map<int, callinfo> table;
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
        wchar_t character;
        const wchar_t *str;
    };

    value() :
        type(value_type::empty) {
    }
    value(value_type type) :
        type(type) {
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
};
inline bool operator==(const value& lhs, const void *rhs) {
    if (lhs.type == value_type::null) return true;
    return false;
}
inline bool operator==(const value& lhs, const value& rhs) {
    if (lhs.type != rhs.type)
        return false;

    if (lhs.type == value_type::integer)
        return lhs.integer == rhs.integer;
    if (lhs.type == value_type::boolean)
        return lhs.integer == rhs.integer;
    if (lhs.type == value_type::string)
        return lhs.str == rhs.str;
    if (lhs.type == value_type::object)
        return lhs.objref == rhs.objref;

    throw rkexception("unimplemented operator");
}

const value rknull = value(value_type::null);

class object {
public:
    int sighash;

    std::map<int, callinfo> *vtable;
    std::map<int, value> properties;
};