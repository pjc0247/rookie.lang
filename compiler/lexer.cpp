#include "stdafx.h"

#include "string_pool.h"

#include "text_processor.h"

static std::vector<lexer_token> rules{
    lexer_token(L"\n", token_type::none),
    lexer_token(L" ", token_type::none),
    lexer_token(L"\t", token_type::none),

    lexer_token(L"//", token_type::none),

    lexer_token(L"include", token_type::keyword),

    lexer_token(L"@", token_type::annotation),
    lexer_token(L"class", token_type::keyword),
    lexer_token(L"def", token_type::keyword),
    lexer_token(L"static", token_type::keyword),
    lexer_token(L"if", token_type::keyword, -9000),
    lexer_token(L"else", token_type::keyword, -9000),
    lexer_token(L"for", token_type::keyword, -10000),
    lexer_token(L"while", token_type::keyword, -10000),
    lexer_token(L"return", token_type::keyword, -9000),
    lexer_token(L"null", token_type::keyword),
    lexer_token(L"in", token_type::keyword),

    lexer_token(L"try", token_type::keyword),
    lexer_token(L"catch", token_type::keyword),
    lexer_token(L"true", token_type::literal),
    lexer_token(L"false", token_type::literal),

    lexer_token(L"++", token_type::op),
    lexer_token(L"--", token_type::op),

    lexer_token(L"is", token_type::keyword),
    lexer_token(L"+", token_type::op, -5001),
    lexer_token(L"-", token_type::op, -5001),
    lexer_token(L"*", token_type::op, -5000),
    lexer_token(L"/", token_type::op, -5000),
    lexer_token(L"&&", token_type::op, -5003),
    lexer_token(L"||", token_type::op, -5003),

    lexer_token(L"==", token_type::op, -5002),
    lexer_token(L"!=", token_type::op, -5002),
    lexer_token(L"<=", token_type::op, -5002),
    lexer_token(L">=", token_type::op, -5002),
    lexer_token(L"<", token_type::op, -5002),
    lexer_token(L">", token_type::op, -5002),

    lexer_token(L"=", token_type::op, -7000),

    lexer_token(L"(", token_type::left_paren, -2000),
    lexer_token(L")", token_type::right_paren, -2000),
    lexer_token(L"{", token_type::left_bracket),
    lexer_token(L"}", token_type::right_bracket, -99999),
    lexer_token(L"[", token_type::left_sq_bracket, -900),
    lexer_token(L"]", token_type::right_sq_bracket, -900),

    lexer_token(L".", token_type::dot, -3000),
    lexer_token(L",", token_type::comma, -1000),
    lexer_token(L":", token_type::colon, -1000),
    lexer_token(L";", token_type::semicolon, -9999)
};

lexer_token::lexer_token(const std::wstring &raw, token_type type, int priority) :
    raw(raw), type(type), priority(priority) {
}

lexer::lexer(compile_context &ctx) :
    ctx(ctx) {
    spool = &ctx.code;
}

std::vector<token> lexer::lex(const std::wstring &_src) {
    std::wstring src = L" " + _src + L" ";
    std::vector<token> result;

    uint32_t head = 1, tail = 1;
    uint32_t line = 1, cols = 1;
    bool inside_quote = false;
    bool inside_comment = false;
    wchar_t last_meaningful_ch = 0;

    std::wstring line_buf;
    int spool_idx = 0;

    while (head < src.length() - 1) {
        bool found = false;

        //line_buf.push_back(src[head]);

        if ((src[head] == '"' || src[head] == '`') && src[head - 1] != '\\')
            inside_quote ^= true;
        if (inside_quote) goto end_loop;

        if (src[head] == '/' && src[head + 1] == '/') {
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

            if (rule.raw == L"-" &&
                (last_meaningful_ch == L'(' ||
                 last_meaningful_ch == L',' ||
                 last_meaningful_ch == L'='))
                continue;
            if (rule.type == token_type::keyword &&
                is_ident_acceptible(src[head - 1]))
                continue;
            if (rule.type == token_type::keyword &&
                is_ident_acceptible(src[head + rule.raw.length()]))
                continue;

            if (rule.type == token_type::dot &&
                is_number(src[head-1]) && is_number(src[head+1]))
                continue;

            if (head != tail) {
                auto t = parse(src.substr(tail, head - tail));
                t.line = line;
                t.cols = cols;
                t.dbg_codeidx = spool_idx;
                result.push_back(t);

                cols += head - tail;

                line_buf.insert(line_buf.end(), t.raw.begin(), t.raw.end());
            }

            auto t = token();
            t.raw = candidate;
            t.type = rule.type;
            t.priority = rule.priority;
            t.line = line; t.cols = cols;
            t.dbg_codeidx = spool_idx;
            result.push_back(t);

            line_buf.insert(line_buf.end(), t.raw.begin(), t.raw.end());

            if (is_ignorable(src[head]))
                last_meaningful_ch = src[head];

            head += candidate.length();
            cols += candidate.length();
            tail = head;
            found = true;

            if (t.raw == L"\n") {
                line++; cols = 1;

                if (ctx.opts.generate_pdb) {
                    spool->get_ptr(line_buf);
                    spool_idx = spool->size();
                    line_buf.clear();
                }
            }
            break;
        }

    end_loop:
        if (!found) {
            if (is_ignorable(src[head]))
                last_meaningful_ch = src[head];

            head++;
        }
    }

    if (head != tail) {
        auto t = parse(src.substr(tail, head - tail));
        t.line = line;
        t.cols = cols;
        t.dbg_codeidx = spool_idx;
        result.push_back(t);
    }

    result.erase(
        std::remove_if(
            result.begin(), result.end(),
            [](token &t) { return t.type == token_type::none; }),
        result.end());

    for (int i = 0; i < result.size(); ++i) {
        if (result[i].raw == L"is")
            result[i].type = token_type::op;
    }
    for (int i = 0; i < result.size(); ++i) {
        if (result[i].raw == L"true" || result[i].raw == L"false")
            result[i].literal_type = literal_type::boolean;
    }

    return result;
}

token lexer::parse(const std::wstring &raw) {
    if (raw.length() == 0)
        return token();

    auto t = token();
    t.raw = raw;

    if (raw[0] == '"' && raw[raw.length() - 1] == '"') {
        t.type = token_type::literal;
        t.literal_type = literal_type::string;
        t.raw = t.raw.substr(1, t.raw.size() - 2);
    }
    if (raw[0] == '`' && raw[raw.length() - 1] == '`') {
        t.type = token_type::literal;
        t.literal_type = literal_type::string_with_interpoloation;
        t.raw = t.raw.substr(1, t.raw.size() - 2);
    }
    else if (std::regex_match(raw, std::wregex(L"-?[0-9]+"))) {
        t.type = token_type::literal;
        t.literal_type = literal_type::integer;
    }
    else if (std::regex_match(raw, std::wregex(L"-?[0-9]+\\.[0-9]+"))) {
        t.type = token_type::literal;
        t.literal_type = literal_type::decimal;
    }
    else if (std::regex_match(raw, std::wregex(L"[a-zA-Z_]+[a-zA-Z0-9_]*"))) {
        t.type = token_type::ident;

        if (t.raw.length() >= rooke_max_signature - 1)
            ctx.push_error(name_too_long_error(t));
    }

    return t;
}

bool lexer::is_ignorable(wchar_t c) {
    if (c != ' ' && c != '\t' &&
        c != '\r' && c != '\n')
        return true;
    return false;
}
bool lexer::is_number(wchar_t c) {
    if (c >= '0' && c <= '9') return true;
    return false;
}
bool lexer::is_ident_acceptible(wchar_t c) {
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_')
        return true;
    return false;
}