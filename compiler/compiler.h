#pragma once

#include <string>
#include <vector>

#include "text_processor.h"
#include "compilation.h"
#include "tree_builder.h"
#include "vnode_transformer.h"

#include "optimiser.h"

class compiler {
public:
    static compiler default_compiler() {
        return compiler()
            .transformer(tco()); // tail-call optimizer
    }

    root_node *ast_raw(const std::string &src, std::vector<compile_error> &errors) {
        compile_context ctx;

        auto tokens = lexer(ctx).lex(src);
        auto stokens = sexper(ctx).sexp(tokens);

        auto root = tree_builder(ctx).build(stokens);

        return root;
    }
    root_node *ast_transformed(const std::string &src, std::vector<compile_error> &errors) {
        auto root = ast_raw(src, errors);
        
        vnode_transformer().transform(root);
        for (auto &t : transformers)
            t.transform(root);

        return root;
    }

    bool compile(const std::string &src,
        program &program,
        std::vector<compile_error> &errors) {

        auto root = ast_transformed(src, errors);
        auto cg = code_gen();

        root->dump();
        program = cg.generate(root);

        delete root;

        return true;
    }

    compiler &transformer(const syntax_traveler &t) {
        transformers.push_back(t);
        return *this;
    }

private:
    std::vector<syntax_traveler> transformers;
};