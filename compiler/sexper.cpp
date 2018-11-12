#include "stdafx.h"

#include "errors.h"

#include "text_processor.h"

#define _mark_as_parsed(stoken) stoken.type = stoken_type::nothing

sexper::sexper(compile_context &ctx) :
    ctx(ctx) {
}

std::vector<stoken> sexper::sexp(const std::vector<token> &_tokens) {
    result.clear();
    depth = 0;

#if _DEBUG
    for (auto &token : _tokens)
        rklog("%ls ", token.raw.c_str());
    rklog("\n\n\r\n");
#endif

    tokens = preprocess(_tokens);

#if _DEBUG
    for (auto &token : tokens)
        rklog("%ls ", token.raw.c_str());
    rklog("\n\n");
#endif

    next_is_at = false;
    has_inherit_list = false;
    state = sexp_state::ss_none;

    for (cursor = 0; cursor < tokens.size(); cursor++) {
        auto token = tokens[cursor];

        rklog("%S / %d / %d / %d\n", token.raw.c_str(), token.type, token.priority, depth);

        if (token.type == token_type::left_bracket) {
            flush_until_priority(INT_MIN);
            depth--;
        }
        else if (token.type == token_type::right_bracket)
            depth++;

        switch (s_pos()) {
        case semantic_position::sp_root:
            sexp_root(token);
            break;
        case semantic_position::sp_class:
            sexp_class(token);
            break;
        case semantic_position::sp_methodbody:
            sexp_methodbody(token);
            break;
        }
    }

    //flush_until_priority(-9999);

    std::reverse(result.begin(), result.end());

#if _DEBUG
    printf("\r\n\r\nSTOKEN\r\n");
    for (auto &stoken : result)
        printf("%S\r\n", stoken.to_string().c_str());
#endif

    return result;
}

std::vector<token> sexper::preprocess(const std::vector<token> &_tokens) {
    std::list<token> tokens(_tokens.begin(), _tokens.end());

    // a.foo(1, 2, 3) -> foo(a, 1, 2, 3)
    /*
    for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it) {
    if ((*it).type == token_type::ident &&
    ((*std::prev(it)).type != token_type::dot &&
    ((*std::next(it)).type == token_type::dot))) {

    int depth = 0;
    for (auto it2 = it; it2 != tokens.end(); ++it2) {
    if ((*it2).type == token_type::left_paren) {
    auto inserted = tokens.insert(std::next(it2), *it);
    it = tokens.erase(it);
    break;
    }
    }
    }
    }
    */

    // foo[1] -> [1]foo
    for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it) {
        if ((*it).type == token_type::left_sq_bracket &&
            ((*std::prev(it)).type == token_type::ident ||
            (*std::prev(it)).type == token_type::keyword)) {

            int depth = 0;
            for (auto it2 = it; it2 != tokens.end(); ++it2) {
                if ((*it2).type == token_type::left_sq_bracket)
                    depth += 1;
                else if ((*it2).type == token_type::right_sq_bracket) {
                    depth -= 1;

                    if (depth == 0) {
                        auto inserted = tokens.insert(std::next(it2), *std::prev(it));
                        tokens.erase(std::prev(it));

                        //(*inserted).priority = -10000;

                        break;
                    }
                }
            }
        }
    }

    // foo(1, 2, 3) -> (1, 2, 3)foo
    for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it) {
        if ((*it).type == token_type::left_paren &&
            ((*std::prev(it)).type == token_type::ident ||
            (*std::prev(it)).type == token_type::keyword)) {

            int depth = 0;
            for (auto it2 = it; it2 != tokens.end(); ++it2) {
                if ((*it2).type == token_type::left_paren)
                    depth += 1;
                else if ((*it2).type == token_type::right_paren) {
                    depth -= 1;

                    if (depth == 0) {
                        bool modify_p = false;
                        if ((*std::prev(it, 2)).type != token_type::dot)
                            modify_p = true;

                        auto inserted = tokens.insert(std::next(it2), *std::prev(it));
                        tokens.erase(std::prev(it));

                        if (modify_p)
                            (*inserted).priority = std::min(-6000, (*inserted).priority);

                        break;
                    }
                }
            }
        }
    }

    // a@ -> @a
    for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
        if ((*it).type == token_type::annotation) {
            std::iter_swap(it, std::prev(it));
        }
    }

    return std::vector<token>(tokens.rbegin(), tokens.rend());
}

void sexper::sexp_root(const token &token) {
    stoken stoken(token);

    if (token.type == token_type::keyword) {
        if (token.raw == L"class")
            stoken.type = stoken_type::st_class;
        else if (token.raw == L"include")
            stoken.type = stoken_type::st_include;
    }
    else if (token.type == token_type::ident)
        stoken.type = stoken_type::ident;
    else if (token.type == token_type::literal)
        stoken.type = stoken_type::st_literal;
    else if (token.type == token_type::colon) {
        stoken.type = stoken_type::st_begin_inherit;
    }
    else if (token.type == token_type::comma) {
        _mark_as_parsed(stoken);
    }
    else if (token.type == token_type::left_bracket) {
        _mark_as_parsed(stoken);
        stoken.type = stoken_type::st_end_inherit;
    }
    else if (token.type == token_type::right_bracket ||
        token.type == token_type::semicolon)
        _mark_as_parsed(stoken);

    if (stoken.type == stoken_type::none)
        ctx.push_error(unexpected_token_error(token));
    else if (stoken.type != stoken_type::nothing)
        result.push_back(stoken);
}
void sexper::sexp_class(token &token) {
    stoken stoken(token);

    if (token.type == token_type::semicolon) {
        _mark_as_parsed(stoken);
        flush_until_priority(token.priority);
        stoken = parse(token);
    }
    else if (token.type == token_type::op) {
        _mark_as_parsed(stoken);

        if (state == sexp_state::ss_param_list) {
            auto &last_ident = stack.back();
            if (last_ident.type == token_type::ident)
                last_ident.raw = L"*" + last_ident.raw;
            else
                ctx.push_error(parsing_error(L"unexpected `*`."));
        }
        else {
            flush_until_priority(token.priority);
            stack.push_back(token);
        }
    }
    else if (token.type == token_type::annotation) {
        _mark_as_parsed(stoken);
        next_is_at = true;
    }
    else if (token.type == token_type::keyword) {
        // Keywords with trailing ident.
        //   ex) def METHOD_NAME
        if (token.raw == L"def")
            result.push_back(pop_and_parse());

        if (token.raw == L"def")
            stoken.type = stoken_type::st_defmethod;
        if (token.raw == L"static")
            stoken.type = stoken_type::st_static;

        else if (token.raw == L"true")
            stoken.type = stoken_type::st_true;
        else if (token.raw == L"false")
            stoken.type = stoken_type::st_false;

        // Method decorator token
        if (token.raw == L"static" &&
            prev_token().raw != L"def") {

            ctx.push_error(unexpected_token_error(token));
        }
    }
    else if (token.type == token_type::left_paren) {
        _mark_as_parsed(stoken);
        flush_until_type(token_type::right_paren);

        state = sexp_state::ss_none;
    }
    else if (token.type == token_type::right_paren) {
        stack.push_back(token.preparsed(stoken_type::st_begin_param));
        stoken.type = stoken_type::st_end_param;

        state = sexp_state::ss_param_list;
    }
    else if (token.type == token_type::literal) {
        stoken.type = stoken_type::st_literal;
    }
    else if (token.type == token_type::ident) {
        _mark_as_parsed(stoken);

        if (next_is_at) {
            ::stoken id(token);
            id.type = stoken_type::ident;
            result.push_back(id);

            ::stoken at(token);
            at.type = stoken_type::st_annotation;
            result.push_back(at);

            next_is_at = false;
        }
        else
            stack.push_back(token);
    }
    else if (token.type == token_type::comma) {
        flush_until_priority(token.priority);
        stoken.type = stoken_type::comma;
    }
    else if (token.type == token_type::right_bracket)
        stoken.type = stoken_type::st_end_block;
    else if (token.type == token_type::left_bracket ||
        token.type == token_type::right_bracket) {
        _mark_as_parsed(stoken);
    }

    if (stoken.type == stoken_type::none)
        ctx.push_error(unexpected_token_error(token));
    else if (stoken.type != stoken_type::nothing)
        result.push_back(stoken);
}
void sexper::sexp_methodbody(const token &token) {
    stoken stoken(token);

    if (token.type == token_type::dot ||
        token.type == token_type::semicolon ||
        token.type == token_type::colon ||
        token.type == token_type::comma) {

        _mark_as_parsed(stoken);
        flush_until_priority(token.priority);
        stoken = parse(token);
    }
    else if (token.type == token_type::annotation) {
        _mark_as_parsed(stoken);
        next_is_at = true;
    }
    else if (token.type == token_type::op) {
        _mark_as_parsed(stoken);
        //flush_until_priority(token.priority);

        if (stack.empty() == false) {
            auto t = stack.back();
            stack.pop_back();
            auto parsed = parse(t);

            if (parsed.type == stoken_type::st_literal ||
                parsed.type == stoken_type::ident)
                result.push_back(parsed);
            else if (parsed.type == stoken_type::st_memberaccess)
                result.push_back(parsed);
            else if (parsed.type == stoken_type::st_null)
                result.push_back(parsed);
            else if (parsed.type == stoken_type::st_true ||
                parsed.type == stoken_type::st_false)
                result.push_back(parsed);
            else {
                stack.push_back(t);
                goto end_parse;
            }

			if (stack.empty() == false) {
				auto type = stack.back().stype;
				if (type == stoken_type::st_begin_call ||
					type == stoken_type::st_memberaccess ||
					type == stoken_type::st_arraccess) {
					result.push_back(parse(stack.back()));
					stack.pop_back();
				}
			}

        end_parse:;

            flush_until_priority(token.priority);
        }

        stack.push_back(token);
    }
    else if (token.type == token_type::left_paren) {
        flush_until_type(token_type::right_paren);
        _mark_as_parsed(stoken);
    }
    else if (token.type == token_type::right_paren) {
        _mark_as_parsed(stoken);
        stack.push_back(token);
    }
    else if (token.type == token_type::left_bracket) {
        _mark_as_parsed(stoken);
        flush_until_type(token_type::right_bracket);
        result.push_back(parse(token));
        depth--;
    }
    else if (token.type == token_type::right_bracket) {
        _mark_as_parsed(stoken);

        flush_single_line();
        if (prev_token().raw == L")")
            stack.push_back(::token(token));
        else
            result.push_back(parse(token));
        depth++;
    }
    else if (token.type == token_type::left_sq_bracket) {
        _mark_as_parsed(stoken);

        auto right_sq_bracket =
            flush_until_type(token_type::right_sq_bracket);

        if (right_sq_bracket.hint_stype == stoken_type::st_end_arr)
            stoken.type = stoken_type::st_begin_arr;
    }
    else if (token.type == token_type::right_sq_bracket) {
        _mark_as_parsed(stoken);

        if (prev_token().type == token_type::ident ||
            prev_token().type == token_type::left_paren) {

            stack.push_back(::token(token)
                .for_stackdelim()
                .with_hint(stoken_type::st_arraccess));
        }
        else {
            stack.push_back(::token(token)
                .for_stackdelim()
                .with_priority(-3000)
                .with_hint(stoken_type::st_end_arr));
            stoken.type = stoken_type::st_end_arr;
        }
    }
    else if (token.type == token_type::ident ||
        token.type == token_type::literal) {

        _mark_as_parsed(stoken);
        bool pushed = false;

        if (prev_token().type == token_type::dot) {
            stack.push_back(prev_token().preparsed(stoken_type::st_memberaccess));
            pushed = true;
        }
        else if (next_token().type == token_type::right_sq_bracket) {
            stack.push_back(next_token().preparsed(stoken_type::st_arraccess));
            pushed = true;
        }

        if (next_token().type == token_type::right_paren) {
            stack.push_back(next_token().preparsed(stoken_type::st_begin_call));
            pushed = true;

            ::stoken endcall(next_token());
            endcall.type = stoken_type::st_end_call;
            result.push_back(endcall);
        }

        ::token ident_token = token;
        if (next_is_at) {
            ident_token.raw = L"@" + token.raw;
            stoken.raw = L"@" + token.raw;
            stoken.source = ident_token;
        }

        pushed = true;
        if (pushed)
            stack.push_back(ident_token);
        else {
            if (token.type == token_type::ident)
                stoken.type = stoken_type::ident;
            else if (token.type == token_type::literal)
                stoken.type = stoken_type::st_literal;
        }

        next_is_at = false;
    }
    else if (token.type == token_type::keyword) {
        _mark_as_parsed(stoken);

        if (token.raw == L"in") {
            flush_single_line();
            stack.back().stype = stoken_type::st_foreach;
            result.push_back(parse(token));
        }
        // Keywords such as
        //   KEYWORD (LINE);  // if (code)
        else if (token.raw == L"return" ||
            token.raw == L"if" ||
            token.raw == L"else" ||
            token.raw == L"for" ||
            token.raw == L"while" ||
            token.raw == L"throw") {

            flush_single_line();
            stack.push_back(token);
        }
        else 
            stack.push_back(token);
    }
    else
        stoken = parse(token);

    ffs:;
    if (stoken.type == stoken_type::none)
        ctx.push_error(unexpected_token_error(token));
    else if (stoken.type != stoken_type::nothing)
        result.push_back(stoken);
}

stoken sexper::parse(const token &token) {
    stoken stoken(token);
    stoken.raw = token.raw;

    // preparsed
    if (token.stype != stoken_type::none)
        stoken.type = token.stype;
    else if (token.type == token_type::left_bracket) {
        stoken.type = stoken_type::st_begin_block;
    }
    else if (token.type == token_type::right_bracket) {
        stoken.type = stoken_type::st_end_block;
    }
    else if (token.type == token_type::literal) {
        if (token.raw == L"true")
            stoken.type = stoken_type::st_true;
        else if (token.raw == L"false")
            stoken.type = stoken_type::st_false;
        else
            stoken.type = stoken_type::st_literal;
    }
    else if (token.type == token_type::ident) {
        stoken.type = stoken_type::ident;
    }
    else if (token.type == token_type::op) {
        stoken.type = stoken_type::op;
    }
    else if (token.type == token_type::semicolon)
        stoken.type = stoken_type::endl;
    else if (token.type == token_type::comma)
        stoken.type = stoken_type::comma;
    else if (token.type == token_type::colon)
        stoken.type = stoken_type::st_colon;
    else if (token.type == token_type::dot)
        stoken.type = stoken_type::nothing;
    else if (token.type == token_type::keyword) {
        parse_keyword(token, stoken);
    }

    return stoken;
}

void sexper::parse_keyword(const token &token, stoken &stoken) {
    if (token.raw == L"class")
        stoken.type = stoken_type::st_class;
    else if (token.raw == L"def")
        stoken.type = stoken_type::st_defmethod;
    else if (token.raw == L"if")
        stoken.type = stoken_type::st_if;
    else if (token.raw == L"else")
        stoken.type = stoken_type::st_else;
    else if (token.raw == L"for")
        stoken.type = stoken_type::st_for;
    else if (token.raw == L"while")
        stoken.type = stoken_type::st_while;
    else if (token.raw == L"return")
        stoken.type = stoken_type::st_return;
    else if (token.raw == L"this")
        stoken.type = stoken_type::st_this;
    else if (token.raw == L"try")
        stoken.type = stoken_type::st_try;
    else if (token.raw == L"catch")
        stoken.type = stoken_type::st_catch;
    else if (token.raw == L"throw")
        stoken.type = stoken_type::st_throw;
    else if (token.raw == L"null")
        stoken.type = stoken_type::st_null;
    else if (token.raw == L"in")
        stoken.type = stoken_type::st_in;
}

void sexper::flush_single_line() {
    flush_until_priority(-9999);
}
void sexper::flush_until_priority(int priority) {
    while (!stack.empty()) {
        auto token = stack.back();

        if (token.priority <= priority) {
            rklog("   stopped : %S, %d\n", token.raw.c_str(), token.priority);
            break;
        }

        stack.pop_back();

        rklog("   flushed : %S, %d\n", token.raw.c_str(), token.priority);
        auto parsed = parse(token);
        if (parsed.type != stoken_type::none &&
            parsed.type != stoken_type::nothing)
            result.push_back(parsed);
    }
}
token sexper::flush_until_type(token_type type) {
    while (!stack.empty()) {
        auto token = stack.back();
        stack.pop_back();

        rklog("   flushed : %S \n", token.raw.c_str());
        auto parsed = parse(token);
        if (parsed.type != stoken_type::none &&
            parsed.type != stoken_type::nothing)
            result.push_back(parsed);

        if (token.type == type) {
            rklog("   stop flushing at %S\n", token.raw.c_str());
            return token;
        }
    }

    rklog("   stop flushing due to end \n");
    return token();
}