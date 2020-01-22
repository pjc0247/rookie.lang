#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkfetch {
public:
    static void import(binding &b) {
        b.function(L"fetch", fetch);
    }
    static value fetch() {

    }
};