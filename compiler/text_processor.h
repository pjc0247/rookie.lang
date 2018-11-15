/*  text_processer.h

    Rookie's Lexer and S-exper.
 */
#pragma once

#include <string>
#include <regex>
#include <vector>
#include <list>

#include "token.h"

struct lexer_token {
    std::wstring raw;
    token_type type;
    int priority;

    lexer_token(const std::wstring &raw, token_type type, int priority = 0);
};

class lexer {
public:
    lexer(compile_context &ctx);

    std::vector<token> lex(const std::wstring &_src);

private:
    bool is_ignorable(wchar_t c);
    bool is_number(wchar_t c);
    bool is_ident_acceptible(wchar_t c);

    token parse(const std::wstring &raw);

private:
    compile_context &ctx;

    string_pool *spool;

	uint32_t line, cols;
};

enum class semantic_position {
    sp_root,
    sp_class,
    sp_methodbody
};
enum class sexp_state {
    ss_none,
    ss_param_list,
};

class sexper {
public:
    sexper(compile_context &ctx);

    std::vector<stoken> sexp(const std::vector<token> &_tokens);

private:
    std::vector<token> preprocess(const std::vector<token> &_tokens);

    void sexp_root(const token &token);
    void sexp_class(token &token);
    void sexp_methodbody(const token &token);

    stoken parse(const token &token);
    void parse_keyword(const token &token, stoken &stoken);
    
    void flush_single_line();
    void flush_until_priority(int priority);
    token flush_until_type(token_type type);

    stoken pop_and_parse() {
        if (stack.empty())
            return stoken::empty();

        auto token = stack.back();
        stack.pop_back();
        auto p = parse(token);
        return p;
    }

    token prev_token() const {
        if (cursor == 0) 
            return token::padding();
        return tokens[cursor - 1];
    }
    token next_token() const {
        if (cursor == tokens.size() - 1)
            return token::padding();
        return tokens[cursor + 1];
    }
    semantic_position s_pos() const {
        if (depth == 0) return semantic_position::sp_root;
        if (depth == 1) return semantic_position::sp_class;
        return semantic_position::sp_methodbody;
    }

private:
    compile_context &ctx;

    uint32_t cursor;
    uint32_t depth;

    bool next_is_at;
    bool has_inherit_list;

    sexp_state state;

    std::vector<token> tokens;
    std::vector<stoken> result;
    std::vector<token> stack;
};