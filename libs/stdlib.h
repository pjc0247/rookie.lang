#pragma once

#include "binding.h"

_rookie_library(rookie_stdlib)

_rookie_function("print", (value v) {
    if (is_rkint(v))
        printf("%d\n", rkint(v));
    else if (is_rkstr(v))
        printf("%s\n", rkcstr(v));

	return rknull;
});

_end_rookie_library
