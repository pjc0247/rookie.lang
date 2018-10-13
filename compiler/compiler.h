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
            .transformer<precalc>()
            .transformer<tco>(); // tail-call optimizer
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

        int round = 0;
        int round_changes = 0;
        int total_changes = 0;
        do {
            round_changes = 0;
            printf("[optimise] round %d\n", round++);
            for (auto &t : transformers) {
                t->prepare();
                printf("   [transform] %s\n", typeid(*t).name());
                round_changes += t->transform(root);
            }
            total_changes += round_changes;
            printf("%d node(s) optimised in this round.\n", round_changes);
            printf("%d changes so far!\n\n", total_changes);
        } while (round_changes > 0);

        return root;
    }

    bool compile(const std::string &src,
        program &program,
        std::vector<compile_error> &errors) {

		binding binding;
		compile_context ctx;

        auto root = ast_transformed(src, errors);
        auto cg = code_gen(ctx, binding);

        root->dump();
        program = cg.generate(root);

        delete root;

		if (ctx.errors.empty())
			return true;

		errors = ctx.errors;
        return false;
    }

    template <typename T>
    compiler &transformer() {
        transformers.push_back(std::make_shared<T>());
        return *this;
    }

private:
    std::vector<std::shared_ptr<syntax_traveler>> transformers;
};