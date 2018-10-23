#include "stdafx.h"

#include "libs/stdlib.h"
#include "libs/array.h"
#include "libs/dictionary.h"
#include "libs/string.h"
#include "libs/debugger.h"
#include "libs/gc.h"
#include "libs/object.h"

#include "binding.h"

binding binding::default_binding() {
    binding b;

    b.import<rkstdlib>();
    b.import<rkarray>();
    b.import<rkdictionary>();
    b.import<rkstring>();
    b.import<rkgc>();
    b.import<rkdebugger>();
    b.import<rkscriptobject>();

    return b;
}