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
#include "ast/ifelse_chain.h"
#include "ast/op_and_assign.h"

#include "validator/syntax_validator.h"
#include "validator/duplicated_name.h"

#include "compiler.h"

#define halt_if_error(x) \
    if (ctx.errors.empty() == false) \
        return x;

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
    halt_if_error(nullptr);
    auto stokens = sexper(ctx).sexp(tokens);
    halt_if_error(nullptr);

    auto root = tree_builder(ctx).build(stokens);

#if _DEBUG
    root->dump();
#endif

    auto validator = new syntax_validator();
    validator->transform(ctx, root);
    auto dup_name_validator = new duplicated_name_validator(ctx);
    dup_name_validator->transform(ctx, root);

    return root;
}
root_node *compiler::ast_transformed(
    compile_context &ctx,
    const std::wstring &src, std::vector<compile_error> &errors) {

    auto root = ast_raw(ctx, src, errors);

    halt_if_error(root);

#if _DEBUG
    root->dump();
#endif

    vnode_transformer().transform(root);

    // 1. Firstpass transformers
    for (auto &t : firstpass)
        t->transform(ctx, root);

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
            round_changes += t->transform(ctx, root);
        }
        total_changes += round_changes;
        rklog("%d node(s) optimised in this round.\n", round_changes);
        rklog("%d changes so far!\n\n", total_changes);
    } while (round_changes > 0);

    ((syntax_traveler*)(new endlpop_transformer()))->transform(ctx, root);

    auto validator = new syntax_validator();
    validator->transform(ctx, root);

    return root;
}

compile_output compiler::compile(const std::wstring &src,
    const compile_option &opts) {

    compile_output out;
    compile_context ctx(opts, binding);
    calltable_builder syscalls;

    for (auto &b : binding.get_functions()) {
        syscalls.add_syscall(b.first, b.second.params);
    }
    for (auto &type : binding.get_types()) {
        for (auto &static_method : type.get_static_methods()) {
            syscalls.add_syscall(type.get_name() + L"::" + static_method.first, static_method.second.params);
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
     .transformer<ifelse_chain_transformer>()
     .transformer<op_and_assign_transformer>()
     .transformer<arr_setitem_transformer>()
     .transformer<arr_getitem_transformer>();
    return c;
}