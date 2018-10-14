#pragma once

#include <string>
#include <regex>
#include <vector>
#include <list>

#include "token.h"
#include "errors.h"

#define _str2cmp(m, idx, c0, c1) *(unsigned short*)(m+idx) == ((c1 << 8) | c0)

struct lexer_token {
    std::string raw;
    token_type type;
    int priority;

    lexer_token(const std::string &raw, token_type type, int priority = 0) :
        raw(raw), type(type), priority(priority) {
    }
};

class lexer {
public:
    lexer(compile_context &ctx) :
        ctx(ctx) {
        init_rules();
    }

    std::vector<token> lex(const std::string &_src) {
        std::string src = " " + _src + " ";
        std::vector<token> result;

        int head = 1, tail = 1;
        int line = 1, cols = 1;
        bool inside_quote = false;
        bool inside_comment = false;

        while (head < src.length() - 1) {
            bool found = false;

            if (src[head] == '"' && src[head - 1] != '\\')
                inside_quote ^= true;
            if (inside_quote) goto end_loop;

            if (_str2cmp(src.c_str(), head, '/', '/')) {
                while (head < src.length() - 1) {
                    head++;
                    if (src[head] == '\n') break;
                }
                continue;
            }

            for (auto &rule : rules) {
                auto &candidate = rule.raw;

                if (head + candidate.length() >= src.length())
                    continue;
                if (src.substr(head, candidate.length()) != candidate)
                    continue;

                if (head != tail) {
                    auto t = parse(src.substr(tail, head - tail));
                    t.line = line;
                    t.cols = cols;
                    result.push_back(t);

                    cols += head - tail;
                }

                auto t = token();
                t.raw = candidate;
                t.type = rule.type;
                t.priority = rule.priority;
                t.line = line; t.cols = cols;
                result.push_back(t);

                head += candidate.length();
                cols += candidate.length();
                tail = head;
                found = true;

                if (t.raw == "\r\n") {
                    line++; cols = 1;
                }
                break;
            }

            end_loop:
            if (!found)
                head++;
        }

        if (head != tail) {
            auto t = parse(src.substr(tail, head - tail));
            t.line = line;
            t.cols = cols;
            result.push_back(t);
        }

        result.erase(
            std::remove_if(
                result.begin(), result.end(),
                [](token &t) { return t.type == token_type::none; }),
            result.end());
        return result;
    }

private:
    void init_rules() {
        rules.push_back(lexer_token("\r\n", token_type::none));
        rules.push_back(lexer_token(" ", token_type::none));
        rules.push_back(lexer_token("\t", token_type::none));

        rules.push_back(lexer_token("//", token_type::none));

        rules.push_back(lexer_token("include", token_type::keyword));

        rules.push_back(lexer_token("@", token_type::keyword));
        rules.push_back(lexer_token("class", token_type::keyword));
        rules.push_back(lexer_token("def", token_type::keyword));
        rules.push_back(lexer_token("if", token_type::keyword));
        rules.push_back(lexer_token("for", token_type::keyword));
        //rules.push_back(lexer_token("new", token_type::keyword));

        rules.push_back(lexer_token("++", token_type::op));
        rules.push_back(lexer_token("--", token_type::op));

        rules.push_back(lexer_token("+", token_type::op, 2));
        rules.push_back(lexer_token("-", token_type::op, 2));
        rules.push_back(lexer_token("*", token_type::op, 4));
        rules.push_back(lexer_token("/", token_type::op, 4));

        rules.push_back(lexer_token("<=", token_type::op, 1));
        rules.push_back(lexer_token(">=", token_type::op, 1));
        rules.push_back(lexer_token("<", token_type::op, 1));
        rules.push_back(lexer_token(">", token_type::op, 1));

        rules.push_back(lexer_token("=", token_type::op, -5000));

        rules.push_back(lexer_token("(", token_type::left_paren, -2000));
        rules.push_back(lexer_token(")", token_type::right_paren, -2000));
        rules.push_back(lexer_token("{", token_type::left_bracket));
        rules.push_back(lexer_token("}", token_type::right_bracket, -99999));
        rules.push_back(lexer_token("[", token_type::left_sq_bracket, -900));
        rules.push_back(lexer_token("]", token_type::right_sq_bracket, -900));

        rules.push_back(lexer_token(".", token_type::dot, -3000));
        rules.push_back(lexer_token(",", token_type::comma, -1000));
        rules.push_back(lexer_token(";", token_type::semicolon, -9999));
    }

    token parse(const std::string &raw) {
        if (raw.length() == 0)
            return token();

        auto t = token();
        t.raw = raw;

        if (raw[0] == '"' && raw[raw.length() - 1] == '"') {
            t.type = token_type::literal;
            t.literal_type = literal_type::string;
            t.raw = t.raw.substr(1, t.raw.size() - 2);
        }
        else if (std::regex_match(raw, std::regex("-?[0-9]+"))) {
            t.type = token_type::literal;
            t.literal_type = literal_type::integer;
        }
        else if (std::regex_match(raw, std::regex("[a-zA-Z_]+[a-zA-Z0-9_]*"))) {
            t.type = token_type::ident;

			if (t.raw.length() >= rooke_max_signature - 1)
				ctx.push_error(name_too_long_error(t));
        }

        return t;
    }

private:
    compile_context &ctx;

    std::vector<lexer_token> rules;
};

enum class semantic_position {
    sp_root,
    sp_class,
    sp_methodbody
};
class sexper {
public:
    sexper(compile_context &ctx) :
        ctx(ctx) {
    }

    std::vector<stoken> sexp(const std::vector<token> &_tokens) {
        result.clear();
        depth = 0;

        for (auto &token : _tokens)
            printf("%s ", token.raw.c_str());
        printf("\n\n\r\n");

        tokens = preprocess(_tokens);

        for (auto &token : tokens)
            printf("%s ", token.raw.c_str());
        printf("\n\n");

        for (cursor = 0; cursor < tokens.size(); cursor ++) {
            auto token = tokens[cursor];

            printf("%s / %d / %d / %d\n", token.raw.c_str(), token.type, token.line, depth);

            if (token.type == token_type::left_bracket)
                depth--;
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

        printf("\r\n\r\nSTOKEN\r\n");
        for (auto &stoken : result)
            printf("%s\r\n", stoken.to_string().c_str());

        return result;
    }

private:
    std::vector<token> preprocess(const std::vector<token> &_tokens) {
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
                            auto inserted = tokens.insert(std::next(it2), *std::prev(it));
                            tokens.erase(std::prev(it));

                            (*inserted).priority = -10000;

                            break;
                        }
                    }
                }
            }
        }

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

        return std::vector<token>(tokens.rbegin(), tokens.rend());
    }

#define _mark_as_parsed(stoken) stoken.type = stoken_type::nothing
    void sexp_root(const token &token) {
        stoken stoken(token);

		if (token.type == token_type::keyword) {
			if (token.raw == "class")
				stoken.type = stoken_type::st_class;
			else if (token.raw == "include")
				stoken.type = stoken_type::st_include;
		}
		else if (token.type == token_type::ident)
			stoken.type = stoken_type::ident;
		else if (token.type == token_type::literal)
			stoken.type = stoken_type::st_literal;

		else if (token.type == token_type::left_bracket ||
			token.type == token_type::right_bracket ||
			token.type == token_type::semicolon)
			_mark_as_parsed(stoken);

		if (stoken.type == stoken_type::none)
			ctx.push_error(unexpected_token_error(token));
        else if (stoken.type != stoken_type::nothing)
            result.push_back(stoken);
    }
    void sexp_class(token &token) {
        stoken stoken(token);

        if (token.type == token_type::keyword) {
            result.push_back(pop_and_parse());

            if (token.raw == "def")
                stoken.type = stoken_type::st_defmethod;
            else if (token.raw == "@")
                stoken.type = stoken_type::st_annotation;
        }
        else if (token.type == token_type::left_paren) {
            flush_until_type(token_type::right_paren);
			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::right_paren) {
            stack.push_back(token.preparsed(stoken_type::st_begin_param));
            stoken.type = stoken_type::st_end_param;
        }
		else if (token.type == token_type::ident) {
			stack.push_back(token);
			_mark_as_parsed(stoken);
		}
        else if (token.type == token_type::comma) {
            flush_until_priority(token.priority);
            stoken.type = stoken_type::comma;
        }
		else if (token.type == token_type::left_bracket ||
			token.type == token_type::right_bracket)
			_mark_as_parsed(stoken);

        if (stoken.type == stoken_type::none)
            ctx.push_error(unexpected_token_error(token));
		else if (stoken.type != stoken_type::nothing)
            result.push_back(stoken);
    }
    void sexp_methodbody(const token &token) {
        stoken stoken(token);

        if (token.type == token_type::dot ||
            token.type == token_type::semicolon ||
            token.type == token_type::comma) {

			_mark_as_parsed(stoken);
            flush_until_priority(token.priority);
            stoken = parse(token);
        }
        else if (token.type == token_type::op) {
            flush_until_priority(token.priority);
            stack.push_back(token);
			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::left_paren) {
            flush_until_type(token_type::right_paren);
			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::right_paren) {
			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::left_bracket) {
            flush_until_type(token_type::right_bracket);
            result.push_back(parse(token));
            depth--;

			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::right_bracket) {
            result.push_back(parse(token));
            depth++;

			_mark_as_parsed(stoken);
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
                prev_token().type == token_type::right_paren) {

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
        else if (token.type == token_type::ident) {
            if (prev_token().type == token_type::dot) {
                stack.push_back(prev_token().preparsed(stoken_type::st_memberaccess));
                stack.push_back(token);
            }
            else if (next_token().type == token_type::right_sq_bracket) {
                stack.push_back(next_token().preparsed(stoken_type::st_arraccess));
                stack.push_back(token);
            }
            else if (next_token().type == token_type::right_paren) {
                stack.push_back(next_token().preparsed(stoken_type::st_begin_call));
                stack.push_back(token);
                
                ::stoken endcall(next_token());
                endcall.type = stoken_type::st_end_call;
                result.push_back(endcall);
            }
            else
                stack.push_back(token);

			_mark_as_parsed(stoken);
        }
        else if (token.type == token_type::keyword) {
            stack.push_back(token);
			_mark_as_parsed(stoken);
        }
        else
            stoken = parse(token);

        if (stoken.type == stoken_type::none)
            ctx.push_error(unexpected_token_error(token));
		else if (stoken.type != stoken_type::nothing)
            result.push_back(stoken);
    }

    stoken parse(const token &token) {
        stoken stoken(token);
        stoken.raw = token.raw;

        // preparsed
		if (token.stype != stoken_type::none)
			stoken.type = token.stype;
		else if (token.type == token_type::left_bracket) {
			stoken.type = stoken_type::begin_block;
		}
		else if (token.type == token_type::right_bracket) {
			stoken.type = stoken_type::end_block;
		}
		else if (token.type == token_type::literal) {
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
		else if (token.type == token_type::dot)
			stoken.type = stoken_type::nothing;
		else if (token.type == token_type::keyword) {
			parse_keyword(token, stoken);
		}
		

        return stoken;
    }
    void parse_keyword(const token &token, stoken &stoken) {
        if (token.raw == "class")
            stoken.type = stoken_type::st_class;
        else if (token.raw == "def")
            stoken.type = stoken_type::st_defmethod;
        else if (token.raw == "if")
            stoken.type = stoken_type::st_if;
        else if (token.raw == "for")
            stoken.type = stoken_type::st_for;
    }

    void flush_until_priority(int priority) {
        while (!stack.empty()) {
            auto token = stack.back();

            if (token.priority <= priority) {
                printf("   stopped : %s\n", token.raw.c_str());
                break;
            }

            stack.pop_back();

            printf("   flushed : %s, %d\n", token.raw.c_str(), token.priority);
            auto parsed = parse(token);
            if (parsed.type != stoken_type::none &&
                parsed.type != stoken_type::nothing)
                result.push_back(parsed);
        }
    }
    token flush_until_type(token_type type) {
        while (!stack.empty()) {
            auto token = stack.back();
            stack.pop_back();

            printf("   flushed : %s \n", token.raw.c_str());
            auto parsed = parse(token);
            if (parsed.type != stoken_type::none &&
                parsed.type != stoken_type::nothing)
                result.push_back(parsed);

            if (token.type == type) {
                printf("   stop flushing at %s\n", token.raw.c_str());
                return token;
            }
        }
        return token();
    }

    stoken pop_and_parse() {
        auto token = stack.back();
        stack.pop_back();
        auto p = parse(token);
        printf(" pop_and_parse %s %d\n", to_string(p.type), stack.size());
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

    int cursor;
    int depth;

    std::vector<token> tokens;
    std::vector<stoken> result;
    std::vector<token> stack;
};