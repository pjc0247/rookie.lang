#include "stdafx.h"

#include "string.h"
#include "object.h"

value rkscriptobject::set_property(value &key, value &value) {
    auto _key = rkwstr(key);
    this->properties[sig2hash(_key)] = value;
    return rknull;
}
value rkscriptobject::get_property(value &key) {
    auto _key = rkwstr(key);
    return this->properties[sig2hash(_key)];
}