#pragma once

#include "binding.h"
#include "value_object.h"

class rookie_array : public rkobject<rookie_array> {
public:
    static void import(binding &b) {
        auto type = type_builder("array");

        type.method("new", create_instance);

        type.method("push", [](value v) {
            printf("PUSH %d\n", v.integer);

            return rknull;
        });
        type.method("remove", [](value v) {
            printf("REMOVE %d\n", v.integer);

            return rknull;
        });

        b.add_type(type);
    }

    void push(value v) {
        printf("PUSH \n");
        ary.push_back(v);
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
