#include "stdafx.h"

#include "libs/stdlib.h"
#include "libs/integer.h"
#include "libs/array.h"
#include "libs/dictionary.h"
#include "libs/string.h"
#include "libs/debugger.h"
#include "libs/gc.h"
#include "libs/object.h"
#include "libs/math.h"

#include "binding.h"

binding binding::default_binding() {
    binding b;

    b.import<rkstdlib>();
    b.import<rkarray>();
    b.import<rkinteger>();
    b.import<rkdictionary>();
    b.import<rkstring>();
    b.import<rkgc>();
    b.import<rkdebugger>();
    b.import<rkscriptobject>();
    b.import<rkmath>();

    return b;
}