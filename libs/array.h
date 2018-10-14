#pragma once

#include "binding.h"
#include "value_object.h"

class rookie_array : public rkobject<rookie_array> {
public:
    static void import(binding &b) {
        auto type = type_builder("array");

        type.method("new", create_instance);

		type.method("at", [](value _this, value idx) {
			printf("%s\n", typeid(*_this.objref).name());
			auto obj = ((rookie_array*)_this.objref);
			printf("at %d, %d\n", idx.integer, obj->ary[idx.integer].integer);
			return obj->ary[idx.integer];
		});
        type.method("push", push);
        type.method("remove", [](value v) {
            printf("REMOVE %d\n", v.integer);

            return rknull;
        });

        b.add_type(type);
    }

    static value push(value _this, value v) {
		auto obj = ((rookie_array*)_this.objref);
		obj->ary.push_back(v);
		printf("push %d\n", v.integer);
		return rknull;
    }

private:
    std::vector<value> ary;
};

/*
_rookie_library(rookie_array)

_rookie_function("array_push", (value _this, value v) {
    auto ary = (rkarray*)_this.objref;
    ary->ary.push_back(v);
});

_end_rookie_library
*/
