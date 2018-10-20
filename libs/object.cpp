#include "stdafx.h"

#include "string.h"
#include "object.h"

value rkscriptobject::get_property(value &key) {
    auto _key = rkcstr(key);

    return this->properties[sig2hash(_key)];
}