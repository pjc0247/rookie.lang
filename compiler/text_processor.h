#pragma once

#include <string>
#include <regex>
#include <vector>
#include <list>

#include "token.h"

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
    lexer() {
        init_rules();
    }

    std::vector<token> lex(const std::string &src, compile_context &ctx) {
        std::vector<token> result;

        int head = 0, tail = 0;
        while (head != src.length()) {
            bool found = false;

            for (auto &rule : rules) {
                auto &candidate = rule.raw;

                if (head + candidate.length() > src.length())
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
                result.push_back(t);

                head += candidate.length();
                tail = head;
                found = true;
                break;
            }

            if (!found)
                head++;
        }

        if (head != tail) {
            auto t = parse(src.substr(tail, head - tail));
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
    std::vector<lexer_token> rules;
};

class sexper {
public:

    std::vector<stoken> sexp(const std::vector<token> &tokens, compile_context &ctx) {
        result.clear();
        depth = 0;

        auto rtokens = preprocess(tokens);

        for (auto &token : rtokens) {
            printf("%s / %d / %d\n", token.raw.c_str(), token.type, token.priority);

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
            }
            else if (token.type == token_type::right_paren) {
                stack.push_back(token);
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
            else
                result.push_back(parse(token));
        }

        //flush_until_priority(-9999);

        std::reverse(result.begin(), result.end());

        printf("\r\n\r\nSTOKEN\r\n");
        for (auto &stoken : result)
            printf("%s / %d\r\n", stoken.raw.c_str(), stoken.type);

        return result;
    }

private:
    std::vector<token> preprocess(const std::vector<token> &_tokens) {
        std::list<token> tokens(_tokens.begin(), _tokens.end());

        /*
        for (auto &token : tokens) {
        if (token.type == token_type::left_paren) {
        token.type = token_type::right_paren;
        token.raw = ")";
        }
        }
        */

        for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it) {
            if ((*it).type == token_type::left_paren &&
                ((*std::prev(it)).type == token_type::ident ||
                (*std::prev(it)).type == token_type::keyword)) {

                int depth = 0;
                for (auto it2 = it; it2 != tokens.begin(); --it2) {
                    if ((*it2).type == token_type::left_paren)
                        depth += 1;
                    else if ((*it2).type == token_type::right_paren) {
                        depth -= 1;

                        if (depth == 0) {
                            std::iter_swap(std::prev(it), it2);

                            break;
                        }
                    }
                }
            }
        }

        return std::vector<token>(tokens.rbegin(), tokens.rend());
    }

    stoken parse(const token &token) {
        stoken stoken;
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

            result.push_back(parse(token));

            if (token.priority <= priority)
                break;
        }
    }
    void flush_until_type(token_type type) {
        while (!stack.empty()) {
            auto token = stack.back();
            stack.pop_back();

            printf("FLUSH %s\n", token.raw.c_str());

            result.push_back(parse(token));

            if (token.type == type)
                break;
        }
    }

private:
    int depth;

    std::vector<stoken> result;
    std::vector<token> stack;
};