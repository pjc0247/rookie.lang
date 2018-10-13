#pragma once

#include <map>
#include <string>
#include <functional>

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
	void add_programcall(const std::string &signature, int entry) {

	}
	int add_syscall(const std::string &signature) {
		int entry = table.size();

		callinfo ci;
		ci.entry = entry;
		ci.type = call_type::ct_syscall_direct;
		table.push_back(ci);

		lookup[signature] = entry;
		return entry;
	}

	bool try_get(const std::string &signature, callinfo &callinfo) {
		auto it = lookup.find(signature);
		if (it == lookup.end()) return false;
		callinfo = table[(*it).second];
		return true;
	}
	callinfo &get(int index) {
		return table[index];
	}

private:
	std::map<std::string, int> lookup;
    std::vector<callinfo> table;
};
struct calltable {
	callinfo *table;
};
class stack_provider;
struct syscalltable {
	std::vector<std::function<void(stack_provider&)>> table;
};

enum class value_type : char {
    empty,
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
    calltable_builder *ctable;

    std::map<std::string, value> properties;
};