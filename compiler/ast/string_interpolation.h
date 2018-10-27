#pragma once

#include <regex>

#include "syntax_travler.h"

class string_interpolation_transformer : public syntax_traveler {
protected:
    bool is_transformable(syntax_node *node) {
        return node->type == syntax_type::syn_literal &&
            ((literal_node*)node)->literal_type == literal_type::string_with_interpoloation;
    }
    virtual syntax_node *visit(syntax_node *node) {
        if (is_transformable(node)) {
            auto new_node = new op_node(node->s_token(), node->parent, L"+");
            auto l = (literal_node*)node;

            auto r = std::wregex(L"\\{\\{([a-zA-Z_0_9@]+)\\}\\}");
            std::wsmatch match;

            int offset = 0;
            std::wstring::const_iterator searchStart(l->str.cbegin());
            while (std::regex_search(searchStart, l->str.cend(), match, r))
            {
                auto left = l->str.substr(offset, match.position());
                auto right = l->str.substr(match.position(), match.length());

                // LEFT
                auto str_node = new literal_node(stoken::empty(), new_node, left);
                // RIGHT
                auto to_string_node = new callmember_node(stoken::empty(), new_node);
                auto id_node = new ident_node(stoken::empty(), new_node, match[1].str());
                // .strconcat
                auto add_node = new op_node(stoken::empty(), new_node, L"+");

                to_string_node->append(new ident_node(stoken::empty(), to_string_node, L"to_string"));
                to_string_node->append(id_node);

                add_node->append(str_node);
                add_node->append(to_string_node);
                new_node->append(add_node);

                searchStart += match.position() + match.length();
                offset += match.position() + match.length();
            }

            if (offset != l->str.size()) {
                auto left = l->str.substr(offset);
                auto str_node = new literal_node(stoken::empty(), new_node, left);
                new_node->append(str_node);
            }

            if (new_node->children.size() % 2 == 1)
                new_node->append(new literal_node(stoken::empty(), new_node, L""));

            return new_node;
        }
        return node;
    }
};