#pragma once

#include <string>
#include <vector>
#include <map>

enum class call_type {
    ct_programcall_direct,
    ct_programcall_name,
    ct_syscall_direct,
    et_syscall_name
};
struct callinfo {
    uint32_t sighash;

    call_type type;
    uint32_t entry;

    uint32_t params;
};
class calltable_builder {
public:
    void add_programcall(const std::wstring &signature, int entry) {

    }
    int add_syscall(const std::wstring &signature, int params) {
        int entry = table.size();

        callinfo ci;
        ci.entry = entry;
        ci.type = call_type::ct_syscall_direct;
        ci.params = params;
        table.push_back(ci);

        lookup[signature] = entry;
        return entry;
    }

    bool try_get(const std::wstring &signature, callinfo &callinfo) {
        auto it = lookup.find(signature);
        if (it == lookup.end()) return false;
        callinfo = table[(*it).second];
        return true;
    }
    callinfo &get(int index) {
        return table[index];
    }

private:
    std::map<std::wstring, int> lookup;
    std::vector<callinfo> table;
};
struct calltable : public std::map<uint32_t, callinfo> {
};
class stack_provider;
struct syscalltable {
    std::vector<std::function<void(stack_provider&)>> table;
};