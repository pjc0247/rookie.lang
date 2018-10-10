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

    comma, semicolon,

    left_paren, right_paren,
    left_bracket, right_bracket,

    left_sq_bracket, right_sq_bracket
};
enum class stoken_type {
    none,

    endl,

    ident,
    st_literal,

    op,

    st_class, st_defmethod,
    keyword_if,

    begin_block, end_block
};
struct token {
    std::string raw;
    token_type type;
    int priority;

    token() :
        raw(""), type(token_type::none), priority(0) {
    }
};
struct stoken {
    std::string raw;
    stoken_type type;
};