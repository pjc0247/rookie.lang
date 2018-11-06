#pragma once

class vstack {
public:
    vstack() {
        buf = new value[1024];
    }

    void push_back(const value &v) {
        buf[ptr] = v;
        ptr++;
    }
    void pop_back() {
        ptr--;
    }
    value &back() {
        return buf[ptr - 1];
    }
    size_t size() {
        return ptr;
    }
    bool empty() {
        return size() == 0;
    }

    value &operator[](size_t ix) const {
        return buf[ix];
    }

private:
    size_t ptr = 0;
    value *buf;
};