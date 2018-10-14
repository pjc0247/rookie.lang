#pragma once

#include "binding.h"
#include "value_object.h"

class rkarray : public rkobject<rkarray> {
public:
    static void import(binding &b) {
        auto type = type_builder("array");

        type.method("new", create_instance);

		bind(type, "at", &rkarray::at);
		bind(type, "push", &rkarray::push);

        b.add_type(type);
    }

	value at(value idx) {
		return ary[rkint(idx)];
	}
    value push(value v) {
		ary.push_back(v);
		return rknull;
    }

private:
    std::vector<value> ary;
};