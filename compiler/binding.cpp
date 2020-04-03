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

    b._import<rkstdlib>();
    b._import<rkarray>();
    b._import<rkinteger>();
    b._import<rkdictionary>();
    b._import<rkstring>();
    b._import<rkgc>();
    b._import<rkdebugger>();
    b._import<rkscriptobject>();
    b._import<rkmath>();
    b._import<rktype>();
    b._import<rkdatetime>();
    b._import<rkreflection>();
    b._import<rkarray_iterator>();
    b._import<rkdictionary_iterator>();
    b._import<rkkvpair>();
    b._import<rkfile>();
    b._import<rkdir>();
    b._import<rkboolean>();
    b._import<rkjson>();
    b._import<rkexception>();
    b._import<rkcaller>();

    return b;
}

void cvt::throw_invalid_casting(const char *expected) {
    throw new rkexception(std::string("Invalid casting, expected: ") + expected);
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