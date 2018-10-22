#pragma once

#include <string>

enum class literal_type {
    integer,
    string,
    string_with_interpoloation
};
enum class token_type {
    none,

    ident,
    literal,

    keyword,
    annotation,
    op,

    dot, comma, colon, semicolon,

    left_paren, right_paren,
    left_bracket, right_bracket,

    left_sq_bracket, right_sq_bracket
};
enum class stoken_type {
    // none: not specified yet
    // nothing: specified as `nothing`
    none, nothing,
    
    st_include,

    st_null,
    st_true, st_false,
    comma, endl, 
    st_colon,

    ident,
    st_literal,

    op,

    st_memberaccess,

    st_this,
    st_newobj,
    st_annotation,
    st_class,
    st_begin_inherit, st_end_inherit,
    st_defmethod,
    st_static,
    st_if, st_for,
    st_return,

    st_try, st_catch, st_finally,

    st_arraccess,

    st_begin_arr, st_end_arr,
    st_begin_call, st_end_call,
    st_begin_param, st_end_param,
    begin_block, end_block
};

inline const wchar_t *to_string(stoken_type type) {
    switch (type) {
    case stoken_type::none: return L"none";
    case stoken_type::nothing: return L"nothing";
    case stoken_type::st_include: return L"st_include";
    case stoken_type::st_null: return L"st_null";
    case stoken_type::st_true: return L"st_true";
    case stoken_type::st_false: return L"st_false";
    case stoken_type::comma: return L"comma";
    case stoken_type::endl: return L"endl";
    case stoken_type::st_this: return L"st_this";
    case stoken_type::ident: return L"ident";
    case stoken_type::st_literal: return L"st_literal";
    case stoken_type::op: return L"op";
    case stoken_type::st_memberaccess: return L"st_memberaccess";
    case stoken_type::st_class: return L"st_class";
    case stoken_type::st_begin_inherit: return L"st_begin_inherit";
    case stoken_type::st_end_inherit: return L"st_end_inherit";
    case stoken_type::st_newobj: return L"st_newobj";
    case stoken_type::st_static: return L"st_static";
    case stoken_type::st_defmethod: return L"st_defmethod";
    case stoken_type::st_annotation: return L"st_annotation";
    case stoken_type::st_return: return L"st_return";
    case stoken_type::st_if: return L"st_if";
    case stoken_type::st_for: return L"st_for";
    case stoken_type::st_arraccess: return L"st_arraccess";
    case stoken_type::st_begin_arr: return L"st_begin_arr";
    case stoken_type::st_end_arr: return L"st_end_arr";
    case stoken_type::st_begin_call: return L"st_begin_call";
    case stoken_type::st_end_call: return L"st_end_call";
    case stoken_type::st_begin_param: return L"st_begin_param";
    case stoken_type::st_end_param: return L"st_end_param";
    case stoken_type::begin_block: return L"begin_block";
    case stoken_type::end_block: return L"end_block";
    default: return L"st_unknown";
    }
}

struct token {
    std::wstring raw;
    token_type type;
    literal_type literal_type;
    int priority;

    int line, cols;

    stoken_type stype;
    stoken_type hint_stype;

    int dbg_codeidx;

    static token padding() {
        return token();
    }
    token() :
        raw(L""), type(token_type::none), priority(0),
        line(0), cols(0),
        stype(stoken_type::none), 
        hint_stype(stoken_type::none),
        dbg_codeidx(-1) {
    }
    token &preparsed(stoken_type type) {
        stype = type;
        return *this;
    }
    token &for_stackdelim() {
        stype = stoken_type::nothing;
        return *this;
    }
    token &with_hint(stoken_type type) {
        hint_stype = type;
        return *this;
    }
    token &with_priority(int _priority) {
        priority = _priority;
        return *this;
    }
};
struct stoken {
    std::wstring raw;
    stoken_type type;

    token source;

    static stoken empty() {
        return stoken(token());
    }

    stoken(const token &token) :
        source(token),
        raw(token.raw),
        type(stoken_type::none) {
    }
    std::wstring to_string() const {
        return std::wstring(::to_string(type)) + L" (" + raw + L")";
    }
};