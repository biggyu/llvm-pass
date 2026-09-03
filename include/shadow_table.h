#pragma once
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>

struct ShadowEntry{
    uintptr_t key;
    double xhat; // value
    double rhat; // residual
    double fp_val;
    double relerr;
    bool used;
};
class ShadowTable {
private:
    static const int TABLE_SIZE = 1 << 24;
    ShadowEntry *table;

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
    ShadowTable() : table(new ShadowEntry[TABLE_SIZE]()) {}
    ~ShadowTable() {
        delete[] table;
    }
    void insert(void* key, double xhat, double rhat, double fp_val, double relerr) {
        size_t idx = hashPtr(key);
        ShadowEntry &entry = table[idx];
        entry.used = true;
        entry.key = (uintptr_t)key;
        entry.xhat = xhat;
        entry.rhat = rhat;
        entry.fp_val = fp_val;
        entry.relerr = relerr;
    }

    ShadowEntry* get(void* key, double progVal) {
        size_t idx = hashPtr(key);
        ShadowEntry &entry = table[idx];
        if(entry.used && entry.key == (uintptr_t)key && entry.fp_val == progVal) {
            return &entry;
        }
        return nullptr;
    }
};

class ShadowStack {
private:
    static const int STACK_SIZE = 1 << 20;
    ShadowEntry *stack;
    int top = 0;
public:
    ShadowStack() : stack(new ShadowEntry[STACK_SIZE]()) {}
    ~ShadowStack() {
        delete[] stack;
    }
    void push(double xhat, double rhat, double fp_val, double relerr) {
        static int maxTop = 0;
        if (top > maxTop) {
            maxTop = top;
            fprintf(stderr, "stack depth: %d\n", maxTop);
        }
        if (top >= STACK_SIZE) {
            fprintf(stderr, "[shadow stack overflow]\n");
            return;
        }
        ShadowEntry &e = stack[top++];
        e.used = true;
        e.xhat = xhat;
        e.rhat = rhat;
        e.fp_val = fp_val;
        e.relerr = relerr;
    }
    ShadowEntry* pop() {
        if (top <= 0) {
            static ShadowEntry dummy{};
            return &dummy;
        }
        return &stack[--top];
    }
};