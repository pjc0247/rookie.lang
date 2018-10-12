#pragma once

#include <map>
#include <string>

enum class call_type {
    ct_programcall_direct,
    ct_programcall_name,
    ct_syscall_direct,
    et_syscall_name
};
struct callinfo {
    call_type type;
    int entry;
};
class calltable {
private:
    std::map<std::string, callinfo> mapping;
};

enum class value_type : char {
    empty,
    callframe,
    integer, string, object
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
        const char *str;

        callframe *cframe;
    };

    value() :
        type(value_type::empty) {
    }

    static value mkobjref(object *objref) {
        value v;
        v.type = value_type::object;
        v.objref = objref;
        return v;
    }
    static value mkcallframe(short pc, short bp, program_entry *entry) {
        value v;
        v.type = value_type::callframe;
        v.cframe = new callframe(pc, bp, entry);
        return v;
    }
    static value mkinteger(int n) {
        value v;
        v.type = value_type::integer;
        v.integer = n;
        return v;
    }
    static value mkstring(const char *str) {
        value v;
        v.type = value_type::string;
        v.str = str;
        return v;
    }
};
struct object {
    calltable *ctable;

    std::map<std::string, value> properties;
};