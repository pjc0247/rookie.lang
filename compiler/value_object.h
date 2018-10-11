#pragma once

#include <map>
#include <string>

enum class call_type {
	ct_internal,
	ct_external
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

struct value {
	value_type type;

	union {
		object *objref;

		int integer;
		const char *str;

		short bp_pc[2];
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
	static value mkcallframe(short pc, short bp) {
		value v;
		v.type = value_type::callframe;
		v.bp_pc[0] = bp;
		v.bp_pc[1] = pc;
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