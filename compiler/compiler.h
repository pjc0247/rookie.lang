#pragma once

#include <string>
#include <vector>

#include "compilation.h"
#include "ast/syntax_travler.h"

#define rky_no_optimization (1 << 0)

class syntax_traveler;
class optimize_travler;

class compiler {
public:
    // Builds a new compiler with default options.
    static compiler default_compiler(binding &binding);
    // Builds a new compiler with given options.
    static compiler build_compiler(binding &binding, int options = 0);

    compiler(binding &binding);

    // Builds a AST-tree which may contain v-nodes.
    root_node *ast_raw(
        compile_context &ctx,
        const std::wstring &src, std::vector<compile_error> &errors);
    // Builds a AST-tree, fully transformed
    root_node *ast_transformed(
        compile_context &ctx,
        const std::wstring &src, std::vector<compile_error> &errors);

    // Compiles given codes into a program.
    compile_output compile(const std::wstring &src,
        const compile_option &opts);

    template <typename T>
    compiler &transformer() {
        for (auto &t : firstpass) {
            if (strcmp(typeid(*t).name(), (typeid(T).name())) == 0)
                throw std::invalid_argument("Duplicated transformer");
        }
        for (auto &t : optimizers) {
            if (strcmp(typeid(*t).name(), (typeid(T).name())) == 0)
                throw std::invalid_argument("Duplicated transformer");
        }

        _transformer<T>();
        return *this;
    }
    compiler &bindings(binding &bindings);

private:
    static compiler &include_essential_passes(compiler &c);

    template <typename T>
    typename std::enable_if<
        std::is_base_of<optimize_travler, T>::value>::type
    _transformer() {
        optimizers.push_back(std::make_shared<T>());
    }
    template <typename T>
    typename std::enable_if<
        !std::is_base_of<optimize_travler, T>::value>::type
    _transformer() {
        optimizers.push_back(std::make_shared<T>());
    }

private:
    binding &binding;

    std::vector<std::shared_ptr<syntax_traveler>> firstpass, optimizers;
};