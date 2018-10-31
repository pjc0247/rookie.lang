#include "stdafx.h"

#include "text_processor.h"
#include "tree_builder.h"
#include "code_gen.h"
#include "vnode_transformer.h"
#include "binding.h"
#include "optimiser.h"

#include "ast/annotation.h"
#include "ast/callmember.h"
#include "ast/callnewobj.h"
#include "ast/arraccess.h"
#include "ast/endlpop.h"
#include "ast/newdic.h"
#include "ast/string_interpolation.h"
#include "ast/classfield.h"
#include "ast/foreach.h"
#include "ast/ifelse_chain.h"

#include "validator/syntax_validator.h"

#include "compiler.h"

compiler compiler::default_compiler(::binding &binding) {
    auto c = compiler(binding);

    // ESSENTIALS TRANSFORMERS
    include_essential_passes(c)
        // OPTIMIZERS
        .transformer<precalc>()
        //.transformer<tco>(); // tail-call optimizer
        ;

    return c;
}
compiler compiler::build_compiler(::binding &binding, int options) {
    auto c = compiler(binding);
    include_essential_passes(c);

    if (!(options & rky_no_optimization))
        c.transformer<precalc>();

    return c;
}

compiler::compiler(::binding &binding) :
    binding(binding) {
}

root_node *compiler::ast_raw(
    compile_context &ctx,
    const std::wstring &src, std::vector<compile_error> &errors) {

    auto tokens = lexer(ctx).lex(src);
    auto stokens = sexper(ctx).sexp(tokens);

    auto root = tree_builder(ctx).build(stokens);

    auto validator = new syntax_validator(ctx);
    validator->transform(root);

    return root;
}
root_node *compiler::ast_transformed(
    compile_context &ctx,
    const std::wstring &src, std::vector<compile_error> &errors) {

    auto root = ast_raw(ctx, src, errors);

    if (ctx.errors.empty() == false)
        return root;

#if _DEBUG
    root->dump();
#endif

    vnode_transformer().transform(root);

    // 1. Firstpass transformers
    for (auto &t : firstpass)
        t->transform(root);

    // 2. Optimizers
    int round = 0;
    int round_changes = 0;
    int total_changes = 0;
    do {
        round_changes = 0;
        rklog("[optimise] round %d\n", round++);
        for (auto &t : optimizers) {
            t->prepare();
            rklog("   [transform] %s\n", typeid(*t).name());
            round_changes += t->transform(root);
        }
        total_changes += round_changes;
        rklog("%d node(s) optimised in this round.\n", round_changes);
        rklog("%d changes so far!\n\n", total_changes);
    } while (round_changes > 0);

    ((syntax_traveler*)(new endlpop_transformer()))->transform(root);

    return root;
}

compile_output compiler::compile(const std::wstring &src,
    const compile_option &opts) {

    compile_output out;
    compile_context ctx(opts, binding);
    calltable_builder syscalls;

    for (auto &b : binding.get_functions()) {
        syscalls.add_syscall(b.first);
    }
    for (auto &type : binding.get_types()) {
        for (auto &static_method : type.get_static_methods()) {
            syscalls.add_syscall(type.get_name() + L"::" + static_method.first);
        }
    }

    auto root = ast_transformed(ctx, src, out.errors);
    auto cg = code_gen(ctx, syscalls);

    if (ctx.errors.empty() == false)
        goto fin;

#if _DEBUG
    root->dump();
#endif

    out.program = cg.generate(root);
    if (opts.generate_pdb)
        out.pdb = cg.generate_pdb(binding);

fin:
    delete root;

    if (ctx.errors.empty() == false)
        out.errors = ctx.errors;
    return out;
}

compiler &compiler::bindings(::binding &bindings) {
    binding = bindings;
    return *this;
}

compiler &compiler::include_essential_passes(compiler &c) {
    c.transformer<annotation_transformer>()
     .transformer<callmember_transformer>()
     .transformer<callnewobj_transformer>()
     .transformer<newdic_transformer>()
     .transformer<string_interpolation_transformer>()
     .transformer<classfield_transformer>()
     .transformer<foreach_transformer>()
     .transformer<ifelse_chain_transformer>()
     .transformer<arr_setitem_transformer>()
     .transformer<arr_getitem_transformer>();
    return c;
}