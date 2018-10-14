#pragma once

#include "binding.h"
#include "value_object.h"

class rookie_array {
public:
	void import(binding &b) {
		auto type = type_builder("array");

		type.method("push", [](value v) {
			printf("PUSH %d\n", v.integer);
		});
		type.method("remove", [](value v) {
			printf("REMOVE %d\n", v.integer);
		});

		b.add_type(type);
	}
};

/*
_rookie_library(rookie_array)

_rookie_function("array_push", (value _this, value v) {
	auto ary = (rkarray*)_this.objref;
	ary->ary.push_back(v);
});

_end_rookie_library
*/
