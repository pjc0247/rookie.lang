#pragma once

#include <string>
#include <vector>
#include <map>

#include "libs/string.h"

class string_intern {
public:
    virtual ~string_intern() {
        for (auto &v : pool)
            delete v.second.objref;
    }

    void preload(const wchar_t *buf, uint32_t buf_len, runner &r) {
        if (buf == nullptr) return;

        uint32_t offset = 0;

        for (uint32_t i = 0; i < buf_len; i++) {
            if (buf[i] == 0) {
                auto str = new rkstring(buf + offset);
                pool[offset] = r._initobj_systype_nogc(sighash_string, str);
                offset = i + 1;
            }
        }
    }

    const value &get(uint32_t offset) {
        assert(pool.find(offset) != pool.end());

        return pool[offset];
    }

private:
    std::map<uint32_t, value> pool;
};