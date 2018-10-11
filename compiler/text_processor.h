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
		int line = 0, cols = 0;
		bool inside_quote = false;
		bool inside_comment = false;

        while (head < src.length() - 1) {
            bool found = false;

			if (inside_quote) goto end_loop;
			if (src[head] == '"' && src[head - 1] != '\\') {
				inside_quote ^= true;
				continue;
			}

			if (_str2cmp(src.c_str(), head, '/', '/')) {
				while (head < src.length() - 1) {
					head++;
					if (src[head] == '\n') break;
				}
				continue;
			}

			if (src[head] == '\n') {
				line++; cols = 0;
			}

            for (auto &rule : rules) {
                auto &candidate = rule.raw;

                if (head + candidate.length() >= src.length())
                    continue;
                if (src.substr(head, candidate.length()) != candidate)
                    continue;

                if (head != tail) {
                    auto t = parse(src.substr(tail, head - tail));
                    result.push_back(t);
                }

                auto t = token();
                t.raw = candidate;
                t.type = rule.type;
                t.priority = rule.priority;
				t.line = line; t.cols = cols;
                result.push_back(t);

                head += candidate.length();
                tail = head;
                found = true;
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

        rules.push_back(lexer_token("class", token_type::keyword));
        rules.push_back(lexer_token("def", token_type::keyword));

        rules.push_back(lexer_token("+", token_type::op, 2));
        rules.push_back(lexer_token("-", token_type::op, 2));
        rules.push_back(lexer_token("*", token_type::op, 4));
        rules.push_back(lexer_token("/", token_type::op, 4));

        rules.push_back(lexer_token("++", token_type::op));
        rules.push_back(lexer_token("--", token_type::op));

        rules.push_back(lexer_token("=", token_type::op, 4));

        rules.push_back(lexer_token("(", token_type::left_paren));
        rules.push_back(lexer_token(")", token_type::right_paren, -2000));
        rules.push_back(lexer_token("{", token_type::left_bracket));
        rules.push_back(lexer_token("}", token_type::right_bracket, -99999));

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
        }
        else if (std::regex_match(raw, std::regex("[a-zA-Z_]+[a-zA-Z0-9_]*"))) {
            t.type = token_type::ident;
        }
        else if (std::regex_match(raw, std::regex("-?[0-9]+"))) {
            t.type = token_type::literal;
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

            printf("%s / %d / %d\n", token.raw.c_str(), token.type, token.priority);

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

		/*
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
		*/

        return std::vector<token>(tokens.rbegin(), tokens.rend());
    }

	void sexp_root(const token &token) {
		stoken stoken(token);

		if (token.type == token_type::keyword) {
			if (token.raw == "class")
				stoken.type = stoken_type::st_class;
		}
		else if (token.type == token_type::ident)
			stoken.type = stoken_type::ident;
		
		if (stoken.type == stoken_type::none)
			ctx.push_error(unexpected_token_error(token));
		else
			result.push_back(stoken);
	}
	void sexp_class(const token &token) {
		stoken stoken(token);

		if (token.type == token_type::keyword) {
			if (token.raw == "def")
				stoken.type = stoken_type::st_defmethod;
		}
		else if (token.type == token_type::left_paren)
			stoken.type = stoken_type::st_begin_param;
		else if (token.type == token_type::right_paren)
			stoken.type = stoken_type::st_end_param;
		else if (token.type == token_type::ident)
			stoken.type = stoken_type::ident;
		else if (token.type == token_type::comma) {
			flush_until_priority(token.priority);
			stoken.type = stoken_type::endl;
		}
		
		if (stoken.type == stoken_type::none)
			ctx.push_error(unexpected_token_error(token));
		else
			result.push_back(stoken);
	}
	void sexp_methodbody(const token &token) {
		stoken stoken(token);

		if (token.type == token_type::semicolon ||
			token.type == token_type::comma) {

			flush_until_priority(token.priority);
		}
		else if (token.type == token_type::op) {
			flush_until_priority(token.priority);
			stack.push_back(token);
		}
		else if (token.type == token_type::left_paren) {
			flush_until_type(token_type::right_paren);

			//stoken.type = stoken_type::st_begin_call;
		}
		else if (token.type == token_type::right_paren) {
			stack.push_back(token);

			//stoken.type = stoken_type::st_end_call;
		}
		else if (token.type == token_type::left_bracket) {
			depth--;
			flush_until_type(token_type::right_bracket);

			result.push_back(parse(token));
		}
		else if (token.type == token_type::right_bracket) {
			depth++;
			//stack.push_back(token);

			result.push_back(parse(token));
		}
		else if (token.type == token_type::ident) {
			if (prev_token().type == token_type::left_paren) {
				::stoken begin_call(prev_token());
				begin_call.type = stoken_type::st_begin_call;

				result.push_back(parse(token));
				result.push_back(begin_call);
			}
			else
				stoken.type = stoken_type::ident;
		}
		else
			stoken = parse(token);

		if (stoken.type == stoken_type::none)
			ctx.push_error(unexpected_token_error(token));
		else
			result.push_back(stoken);
	}

    stoken parse(const token &token) {
        stoken stoken(token);
        stoken.raw = token.raw;

        if (token.type == token_type::left_bracket) {
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
    }

    void flush_until_priority(int priority) {
        while (!stack.empty()) {
            auto token = stack.back();
            stack.pop_back();

			printf("flushed : %s \n", token.raw.c_str());
			auto parsed = parse(token);
			if (parsed.type != stoken_type::none)
				result.push_back(parsed);

            if (token.priority >= priority)
                break;
        }
    }
    void flush_until_type(token_type type) {
        while (!stack.empty()) {
            auto token = stack.back();
            stack.pop_back();

			if (token.type == type)
				break;

			printf("flushed : %s \n", token.raw.c_str());
			auto parsed = parse(token);
			if (parsed.type != stoken_type::none)
				result.push_back(parsed);
        }
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