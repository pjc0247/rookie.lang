#pragma once

#include <string>
#include <map>
#include <vector>

class string_pool {
public:
    int get_ptr(const std::wstring &str) {
        if (instruction_indexes.find(str) != instruction_indexes.end())
            return instruction_indexes[str];
        return append(str);
    }
    const wchar_t *fin() const {
        if (pool.empty())
            return L"";
        return &(pool.front());
    }
    unsigned int size() const {
        return pool.size();
    }

    void dump() {
        printf("===begin_string_pool====\r\n");
        for (int i = 0; i < pool.size(); i++)
            putchar(pool[i]);
        printf("\r\n===end_string_pool====\r\n");
    }
private:
    int append(const std::wstring &str) {
        auto ptr = pool.size();
        pool.insert(pool.end(), str.begin(), str.end());
        pool.insert(pool.end(), 0);
        instruction_indexes[str] = ptr;
        return ptr;
    }

private:
    std::map<std::wstring, int> instruction_indexes;
    std::vector<wchar_t> pool;
};