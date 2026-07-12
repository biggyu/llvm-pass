#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

struct ShadowEntry{
    uintptr_t key;
    double xhat; // value
    double rhat; // residual
    bool sign;
    double ehat; // log value
    bool isExact;
    double relerr;
    bool used;
};
class ShadowTable {
private:
    static const int TABLE_SIZE = 1 << 22;
    ShadowEntry table[TABLE_SIZE];

    static inline uint64_t mix64(uint64_t x) {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return x;
    }
    static inline std::size_t hashPtr(void* addr) {
        uint64_t x = (uint64_t)(uintptr_t)addr;
        return (std::size_t)(mix64(x) & (TABLE_SIZE - 1));
    }
public:
    ShadowTable() {
        std::memset(table, 0, sizeof(table));
    }
    void insert(void* key, double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr) {
        size_t idx = hashPtr(key);
        ShadowEntry &entry = table[idx];
        entry.used = true;
        entry.key = (uintptr_t)key;
        entry.xhat = xhat;
        entry.rhat = rhat;
        entry.relerr = relerr;
        entry.sign = sign;
        entry.isExact = isExact;
        entry.ehat = ehat;
    }

    ShadowEntry* get(void* key, double progVal) {
        size_t idx = hashPtr(key);
        ShadowEntry &entry = table[idx];
        if(entry.used && entry.key == (uintptr_t)key && entry.xhat == progVal) {
            return &entry;
        }
        return nullptr;
    }
};

class ShadowStack {
private:
    ShadowEntry shadow_stack[256];
    int top = 0;
public:
    void push(double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr) {
        ShadowEntry &e = shadow_stack[top++];
        e.used = true;
        e.xhat = xhat;
        e.rhat = rhat;
        e.relerr = relerr;
        e.sign = sign;
        e.isExact = isExact;
        e.ehat = ehat;
    }
    ShadowEntry* pop() {
        return &shadow_stack[--top];
    }
};