#pragma once

#include <algorithm>

#include "binding.h"
#include "value_object.h"

class rkarray : public rkobject<rkarray> {
public:
    static void import(binding &b) {
        auto type = type_builder("array");

        type.method("new", create_instance);

		bind(type, "at", &rkarray::at);
		bind(type, "push", &rkarray::push);
		bind(type, "remove", &rkarray::remove);
		bind(type, "length", &rkarray::length);

        b.add_type(type);
    }

	value at(value idx) {
		return ary[rkint(idx)];
	}
    value push(value v) {
		ary.push_back(v);
		return rknull;
    }
	value remove(value v) {
		ary.erase(std::remove(
			ary.begin(), ary.end(), v),
			ary.end());
		return rknull;
	}
	value length() {
		return value::mkinteger(ary.size());
	}

private:
    std::vector<value> ary;
};