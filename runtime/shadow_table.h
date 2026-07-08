#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

struct ShadowEntry{
    uintptr_t key;
    double xhat; // value
    double rhat; // residual
    bool sign;
    bool isExact;
    double ehat; // log value

    double error;
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
    void insert(void* key, double x, double rhat, double dx, bool sign, bool isExact, double ehat) {
        size_t idx = hashPtr(key);
        ShadowEntry &entry = table[idx];
        entry.used = true;
        entry.key = (uintptr_t)key;
        entry.xhat = x;
        entry.rhat = rhat;
        entry.error = dx;
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

//TODO: modify to DSLValue
class ShadowStack {
private:
    double shadow_stack[256];
    int top = 0;
public:
    void push(double err) {
        shadow_stack[top++] = err;
    }
    double pop() {
        return shadow_stack[--top];
    }
};