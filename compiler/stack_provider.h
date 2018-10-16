#pragma once

#include <deque>

#include "value_object.h"

class stack_provider {
public:
    stack_provider(std::deque<value> &stack) :
        stackref(stack) {
    }

    void push(const value &v) {
        stackref.push_back(v);
    }
    value pop() {
        auto item = stackref.back();
        stackref.pop_back();
        return item;
    }

    __forceinline value &get(int n) {
        return stackref[stackref.size() - 1 - n];
    }

    template <int N>
    __forceinline void drop() {
        stackref.pop_back();
        drop<N - 1>();
    }

    template <int N>
    __forceinline void replace(const value &v) {
        stackref[stackref.size() - 1 - N] = v;
    }

private:
    std::deque<value> &stackref;
};

template <>
__forceinline void stack_provider::drop<1>() {
    stackref.pop_back();
}