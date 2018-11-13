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
#include "libs/dir.h"
#include "libs/boolean.h"
#include "libs/json.h"
#include "libs/exception.h"
#include "libs/caller.h"

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
    b.import<rkdir>();
    b.import<rkboolean>();
    b.import<rkjson>();
    b.import<rkexception>();
    b.import<rkcaller>();

    return b;
}

void cvt::throw_invalid_casting() {
    throw new rkexception("Invalid casting");
}
std::wstring cvt::value_to_std_wstring(value_cref v) {
    return rkwstr(v);
}

value type_builder::default_to_string(value_cref _this) {
    auto objthis = (object*)_this.objref;
    std::wstring str = L"#<";
    str += objthis->name_ptr;

    for (auto &p : objthis->properties) {
        str += L" @" + rk_id2wstr(p.first) + L": ";
        str += rk_call_tostring_w(p.second);
    }

    str += L">";
    return str2rk(str);
}