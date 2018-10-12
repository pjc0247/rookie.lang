#pragma once

#include <string>

enum class literal_type {
    integer,
    string
};
enum class token_type {
    none,

    ident,
    literal,

    keyword,
    op,

    dot, comma, semicolon,

    left_paren, right_paren,
    left_bracket, right_bracket,

    left_sq_bracket, right_sq_bracket
};
enum class stoken_type {
    none,

    comma, endl,

    ident,
    st_literal,

    op,

    st_newobj,
    st_class, st_defmethod,
    st_if, st_for,

    st_begin_call, st_end_call,
    st_begin_param, st_end_param,
    begin_block, end_block
};

inline const char *to_string(stoken_type type) {
    switch (type) {
    case stoken_type::none: return "none";
    case stoken_type::comma: return "comma";
    case stoken_type::endl: return "endl";
    case stoken_type::ident: return "ident";
    case stoken_type::st_literal: return "st_literal";
    case stoken_type::op: return "op";
    case stoken_type::st_class: return "st_class";
    case stoken_type::st_newobj: return "st_newobj";
    case stoken_type::st_defmethod: return "st_defmethod";
    case stoken_type::st_if: return "st_if";
    case stoken_type::st_for: return "st_for";
    case stoken_type::st_begin_call: return "st_begin_call";
    case stoken_type::st_end_call: return "st_end_call";
    case stoken_type::st_begin_param: return "st_begin_param";
    case stoken_type::st_end_param: return "st_end_param";
    case stoken_type::begin_block: return "begin_block";
    case stoken_type::end_block: return "end_block";
    default: return "st_unknown";
    }
}

struct token {
    std::string raw;
    token_type type;
    literal_type literal_type;
    int priority;

    int line, cols;

    stoken_type stype;
    
    static token padding() {
        return token();
    }
    token() :
        raw(""), type(token_type::none), priority(0),
        line(0), cols(0),
        stype(stoken_type::none) {
    }
    token &preparsed(stoken_type type) {
        stype = type;
        return *this;
    }
};
struct stoken {
    std::string raw;
    stoken_type type;

    token source;

    stoken(const token &token) :
        source(token),
        raw(token.raw),
        type(stoken_type::none) {
    }
    std::string to_string() const {
        return std::string(::to_string(type)) + " (" + raw + ")";
    }
};