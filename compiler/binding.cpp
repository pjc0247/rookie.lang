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
#include "libs/type.h"
#include "libs/datetime.h"
#include "libs/reflection.h"
#include "libs/kvpair.h"
#include "libs/iterator.h"
#include "libs/file.h"
#include "libs/boolean.h"
#include "libs/json.h"

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
    b.import<rktype>();
    b.import<rkdatetime>();
    b.import<rkreflection>();
    b.import<rkarray_iterator>();
    b.import<rkdictionary_iterator>();
    b.import<rkkvpair>();
    b.import<rkfile>();
    b.import<rkboolean>();
    b.import<rkjson>();

    return b;
}