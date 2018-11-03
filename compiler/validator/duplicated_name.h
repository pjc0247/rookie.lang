#pragma once

#include <map>

#include "compilation.h"

#include "../ast/syntax_travler.h"

class duplicated_name_validator : public syntax_traveler {
public:
    duplicated_name_validator(compile_context &ctx) :
        ctx(ctx) {
    }

    virtual syntax_node *visit(syntax_node *node) {
        switch (node->type) {
        case syntax_type::syn_class:
            add_type((class_node*)node);
            break;
        case syntax_type::syn_method:
            add_method((method_node*)node);
            break;
        }

        return node;
    }

private:
    void add_type(class_node *node) {
        auto it = types.find(node->ident_str());
        if (it == types.end())
            types[node->ident_str()] = std::set<std::wstring>();
    }
    void add_method(method_node *node) {
        auto &type = types[node->declaring_class()->ident_str()];
        if (type.find(node->ident_str()) != type.end())
            ctx.push_error(syntax_error(node, L"duplicated method name."));

        type.insert(node->ident_str());
    }

private:
    compile_context &ctx;
    
    std::map<std::wstring, std::set<std::wstring>> types;
};