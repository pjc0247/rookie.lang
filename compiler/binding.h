#pragma once

#include <map>
#include <string>
#include <deque>
#include <functional>

#include "value.h"
#include "object.h"
#include "sig2hash.h"
#include "stack_provider.h"
#include "exe_context.h"

class rkstring;

#define _bind(b, signature, lambda, params) \
    b[signature] = bind_info(lambda, params)

#define _rookie_library(name) \
    class name { \
    public: \
        static void import(binding &b) {
#define _end_rookie_library \
    } };
#define _rookie_function(name, body) \
    b.function(name, [] body );

#define rk_id2wstr(x) (rkctx()->get_name(x))
#define rk_id2str(x) (str2rk(rkctx()->get_name(x))) 

#define rk_call_tostring_w(x) rkwstr(rkctx()->call(x, rk_id_tostring))
#define rk_call_tostring_c(x) rkcstr(rkctx()->call(x, rk_id_tostring))
#define rk_call_tostring(x) rk_call_tostring_w(x)

#define is_rkint(v) v.type == value_type::integer
#define is_rkdecimal(v) v.type == value_type::decimal
#define is_rkstr(v) (v.type == value_type::object && v.objref->sighash == sighash_string)
#define is_rkchar(v) v.type == value_type::character
#define is_rkbool(v) v.type == value_type::boolean

#define rkcstr(v) (((rkstring*)v.objref)->c_str())
#define rkwstr(v) (((rkstring*)v.objref)->w_str())

#define bool2rk(v) value::mkboolean(v)
#define rk2bool(v) (v.integer == 1 ? true : false)

#define int2rk(v) value::mkinteger(v)
#define rk2int(v) v.integer
#define uint2rk(v) value::mkuinteger(v);
#define rk2uint(V) v.uinteger

#define float2rk(v) value::mkdecimal(v)
#define rk2float(v) v.decimal

#define str2rk(v) value::mkstring2(v)
#define rk2str(v) rkwstr(v)
#define rk2cstr(v) rkcstr(v)

#define obj2rk(v) (rkctx()->init_obj(sig2hash(v->type_name()), v))
#define obj2rk_nogc(v, name) (rkctx()->init_obj_nogc(sig2hash_c(name), v))
#define rk2obj(v, type) ((type)v.objref)

#define rkthis (rkctx()->get_this())
#define rkvcall(obj, name) (rkctx()->call(obj, sig2hash_c(name)))

#define rkid(id) sig2hash_c(id)

#define _begin_no_gc() rkctx()->gc().begin_no_gc()
#define _end_no_gc()   rkctx()->gc().end_no_gc()

struct bind_info {
    int params;
    std::function<void(stack_provider&)> func;

    bind_info() {
    }
    bind_info(const std::function<void(stack_provider&)> &func, int params) :
        func(func), params(params) {
    }
};

typedef std::map<std::wstring, bind_info> bindmap;

class cvt {
public:
    template <typename T>
    static
    std::enable_if_t<std::is_same<value_cref, T>::value, T>
        value_to_ctype(value_cref v) {
        return v;
    }
    template <typename T>
    static
    std::enable_if_t<std::is_same<uint32_t, T>::value, T>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::integer)
            return v.uinteger;
        handle_cast_failure<T>();
    }
    template <typename T>
    static
    std::enable_if_t<std::is_same<int32_t, T>::value, T>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::integer)
            return v.integer;
        handle_cast_failure<T>();
    }
    template <typename T>
    static
    std::enable_if_t<std::is_same<float, T>::value, T>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::decimal)
            return v.decimal;
        handle_cast_failure<T>();
    }
    template <typename T>
    static
    std::enable_if_t<std::is_same<bool, T>::value, T>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::boolean)
            return v.integer ? true : false;
        handle_cast_failure<T>();
    }
    template <typename T>
    static
    std::enable_if_t<std::is_same<const std::wstring &, T>::value, std::wstring>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::object) {
            if (v.objref->sighash == sighash_string)
                return value_to_std_wstring(v);
        }
        handle_cast_failure<T>();
    }
    static std::wstring value_to_std_wstring(value_cref v);

    template <typename T>
    static
    std::enable_if_t<std::is_base_of<object, T>::value, T>
    value_to_ctype(value_cref v) {
        if (v.type == value_type::object) {
            return rk2obj(v, T);
        }
        handle_cast_failure<T>();
    }
    template <typename T>
    [[noreturn]] static void handle_cast_failure() {
        throw_invalid_casting(typeid(T).name());
    }
    static void throw_invalid_casting(const char *expected);
};

class type_builder {
public:
    type_builder(const std::wstring &name) :
        name(name) {

        method(rk_id_tostring, default_to_string);
    }

    static value default_to_string(value_cref _this);

    type_builder &static_method(const std::wstring &signature,
        const std::function<value()> &function) {

        _bind(static_methods, signature, [function](stack_provider &sp) {
            sp.push(function());
        }, 0);
        return *this;
    }
    type_builder &static_method(const std::wstring &signature,
        const std::function<value(value_cref)> &function) {

        _bind(static_methods, signature, [function](stack_provider &sp) {
            sp.replace<0>(function(sp.get(0)));
        }, 1);
        return *this;
    }
    type_builder &static_method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref)> &function) {

        _bind(static_methods, signature, [function](stack_provider &sp) {
            sp.replace<1>(function(sp.get(1), sp.get(0)));
            sp.drop<1>();
        }, 2);
        return *this;
    }

    type_builder &method(const std::wstring &signature,
        const std::function<value()> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.push(function());
        }, 0);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<0>(function(sp.get(0)));
        }, 1);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<1>(function(sp.get(1), sp.get(0)));
            sp.drop<1>();
        }, 2);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<2>(function(sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<2>();
        }, 3);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<3>(function(sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<3>();
        }, 4);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<4>(function(sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<4>();
        }, 5);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<5>(function(sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<5>();
        }, 6);
        return *this;
    }
    type_builder &method(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<6>(function(sp.get(6), sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<6>();
        }, 7);
        return *this;
    }
    
    const std::wstring &get_name() const {
        return name;
    }
    const bindmap &get_methods() const {
        return methods;
    }
    const bindmap &get_static_methods() const {
        return static_methods;
    }

private:
    std::wstring name;
    bindmap methods;
    bindmap static_methods;
};

class binding {
public:
    static binding default_binding();

    void add_type(type_builder &type) {
        types.push_back(type);
    }

    void function(const std::wstring &signature,
        const std::function<value()> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.push(function());
        }, 0);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<0>(function(sp.get(0)));
        }, 1);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref,value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<1>(function(sp.get(1), sp.get(0)));
            sp.drop<1>();
        }, 2);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<2>(function(sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<2>();
        }, 3);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<3>(function(sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<3>();
        }, 4);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<4>(function(sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<4>();
        }, 5);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<5>(function(sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<5>();
        }, 6);
    }
    void function(const std::wstring &signature,
        const std::function<value(value_cref, value_cref, value_cref, value_cref, value_cref, value_cref, value_cref)> &function) {

        _bind(methods, signature, [function](stack_provider &sp) {
            sp.replace<6>(function(sp.get(6), sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
            sp.drop<6>();
        }, 7);
    }

    template <typename T>
    binding &import() {
        rklog("[import] %s\n", typeid(T).name());
        T::import(*this);
        return *this;
    }

    const std::vector<type_builder> &get_types() const {
        return types;
    }
    const bindmap &get_functions() const {
        return methods;
    }

private:
    std::vector<type_builder> types;

    bindmap methods;
};

template <typename T>
class rkobject : public object {
public:
    TYPENAME(L"object")
        
    rkobject() {
        name_ptr = type_name();
    }

    static value create_instance() {
        return value::mkobjref(new T());
    }

    // Defines a static method with zero parameter.
    static void static_method(type_builder &type,
        const wchar_t *name, value(*function)()) {

        type.static_method(name, [function]() {
            return stdinvoke(function);
        });
    }
    // Defines a static method with 1 parameter.
    template <typename P1>
    static void static_method(type_builder &type,
        const wchar_t *name, value(*function)(P1)) {

        type.static_method(name, [function](value_cref a) {
            return stdinvoke(function, cvt::value_to_ctype<P1>(a));
        });
    }
    // Defines a static method with 2 parameters.
    template <typename P1, typename P2>
    static void static_method(type_builder &type,
        const wchar_t *name, value(*function)(P1, P2)) {

        type.static_method(name, [function](value_cref a, value_cref b) {
            return stdinvoke(function, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b));
        });
    }
    // Defines a static method with 3 parameters.
    template <typename P1, typename P2, typename P3>
    static void static_method(type_builder &type,
        const wchar_t *name, value(*function)(P1, P2, P3)) {

        type.static_method(name, [function](value_cref a, value_cref b, value_cref c) {
            return stdinvoke(function, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b), cvt::value_to_ctype<P3>(c));
        });
    }

    // Defines an instance method with zero parameter.
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)()) {

        type.method(name, [function](value_cref _this) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj);
        });
    }
    // Defines an instance method with 1 parameter.
    template <typename P1>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1)) {

        type.method(name, [function](value_cref _this, value_cref a) {
            auto obj = ((T*)_this.objref);
            auto bb = cvt::value_to_ctype<P1>(a);
            return stdinvoke(function, obj, bb);
        });
    }
    // Defines an instance method with 2 parameters.
    template <typename P1, typename P2>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1, P2)) {

        type.method(name, [function](value_cref _this, value_cref a, value_cref b) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b));
        });
    }
    // Defines an instance method with 3 parameters.
    template <typename P1, typename P2, typename P3>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1, P2, P3)) {

        type.method(name, [function](value_cref _this, value_cref a, value_cref b, value_cref c) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b), cvt::value_to_ctype<P3>(c));
        });
    }
    // Defines an instance method with 4 parameters.
    template <typename P1, typename P2, typename P3, typename P4>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1, P2, P3, P4)) {

        type.method(name, [function](value_cref _this, value_cref a, value_cref b, value_cref c, value_cref d) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b), cvt::value_to_ctype<P3>(c), cvt::value_to_ctype<P4>(d));
        });
    }
    // Defines an instance method with 5 parameters.
    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1, P2, P3, P4, P5)) {

        type.method(name, [function](value_cref _this, value_cref a, value_cref b, value_cref c, value_cref d, value_cref e) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b), cvt::value_to_ctype<P3>(c), cvt::value_to_ctype<P4>(d), cvt::value_to_ctype<P5>(e));
        });
    }
    // Defines an instance method with 6 parameters.
    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    static void method(type_builder &type,
        const wchar_t *name, value(T::*function)(P1, P2, P3, P4, P5, P6)) {

        type.method(name, [function](value_cref _this, value_cref a, value_cref b, value_cref c, value_cref d, value_cref e, value_cref f) {
            auto obj = ((T*)_this.objref);
            return stdinvoke(function, obj, cvt::value_to_ctype<P1>(a), cvt::value_to_ctype<P2>(b), cvt::value_to_ctype<P3>(c), cvt::value_to_ctype<P4>(d), cvt::value_to_ctype<P5>(e), cvt::value_to_ctype<P6>(f));
        });
    }
};