/*  stack_provider.h

    Provides wrapping methods for runtime stack.
 */
#pragma once

#include <deque>

#include "vstack.h"

class stack_provider {
public:
    stack_provider(vstack &stack) :
        stackref(stack) {
    }

    __forceinline void push(const value &v) {
        stackref.push_back(v);
    }
    __forceinline value pop() {
        auto item = stackref.back();
        stackref.pop_back();
        return item;
    }

    __forceinline value &get(int n) {
        return stackref[stackref.size() - 1 - n];
    }

    template <int N>
    __forceinline void drop() {
#if RK_USE_STDSTACK
        stackref.pop_back();
        drop<N - 1>();
#else
        stackref.drop(N);
#endif
    }

    template <int N>
    __forceinline void replace(const value &v) {
        stackref[stackref.size() - 1 - N] = v;
    }

private:
    vstack &stackref;
};

template <>
__forceinline void stack_provider::drop<1>() {
    stackref.pop_back();
}