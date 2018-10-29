#pragma once

#include "binding.h"

#include "sig2hash.h"
#include "string.h"

_rookie_library(rkstdlib)

// TODO
// print is 'puts' currently
_rookie_function(L"print", (value_cref v) {
    if (v == nullptr)
        printf("null\n");

    else if (is_rkbool(v))
        printf("%s\n", rkbool(v) ? "true" : "false");
    else if (is_rkint(v))
        printf("%d\n", rkint(v));
    else if (is_rkstr(v))
        wprintf(L"%s\n", rkcstr(v));
    else if (is_rkchar(v))
        wprintf(L"%c\n", rkchar(v));

    return rknull;
});
_rookie_function(L"puts", (value_cref v) {
    if (v == nullptr)
        printf("null\n");

    else if (is_rkbool(v))
        printf("%s\n", rkbool(v) ? "true" : "false");
    else if (is_rkint(v))
        printf("%d\n", rkint(v));
    else if (is_rkstr(v)) {
        wprintf(L"%s\n", rkcstr(v));
    }
    else if (is_rkchar(v))
        wprintf(L"%c\n", rkchar(v));

    return rknull;
});

_end_rookie_library
