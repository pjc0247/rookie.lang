#pragma once

#include <string>
#include <vector>

#include "text_processor.h"
#include "compilation.h"
#include "tree_builder.h"
#include "vnode_transformer.h"

#include "optimiser.h"

#include "ast/callmember.h"
#include "ast/callnewobj.h"
#include "ast/arraccess.h"

class compiler {
public:
    static compiler default_compiler(binding &binding) {
		return compiler(binding)
			.transformer<callmember_transformer>()
			.transformer<callnewobj_transformer>()
			.transformer<arraccess_transformer>()

			.transformer<precalc>()
			//.transformer<tco>(); // tail-call optimizer
			;
    }

    compiler(binding &binding) :
        binding(binding) {

    }

    root_node *ast_raw(
        compile_context &ctx,
        const std::string &src, std::vector<compile_error> &errors) {

        auto tokens = lexer(ctx).lex(src);
        auto stokens = sexper(ctx).sexp(tokens);

        auto root = tree_builder(ctx).build(stokens);

        return root;
    }
    root_node *ast_transformed(
        compile_context &ctx,
        const std::string &src, std::vector<compile_error> &errors) {

        auto root = ast_raw(ctx, src, errors);
        
		root->dump();

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

        compile_context ctx;

        calltable_builder syscalls;

        for (auto &b : binding.get_methods()) {
            syscalls.add_syscall(b.first);
        }

        auto root = ast_transformed(ctx, src, errors);
        auto cg = code_gen(ctx, syscalls);

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
    compiler &bindings(binding &bindings) {
        binding = bindings;
        return *this;
    }

private:
    binding &binding;
    std::vector<std::shared_ptr<syntax_traveler>> transformers;
};