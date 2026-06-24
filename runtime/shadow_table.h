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
        uintptr_t k = (uintptr_t)key;
        size_t idx = hashPtr(key);
        for (size_t probe = 0; probe < TABLE_SIZE; probe++) {
            ShadowEntry &entry = table[idx];
            if (!entry.used) {
                entry.used = true;
                entry.key = k;
                entry.xhat = x;
                entry.rhat = rhat;
                entry.error = dx;
                entry.sign = sign;
                entry.isExact = isExact;
                entry.ehat = ehat;
                return;
            }
            //! Update value or just error
            if (entry.key == k) {
                entry.xhat = x;
                entry.rhat = rhat;
                entry.error = dx;
                entry.sign = sign;
                entry.isExact = isExact;
                entry.ehat = ehat;
                return;
            }
            idx = (idx + 1) & (TABLE_SIZE - 1); 
        }
    }
    // void insert(void* key, float x, double dx) {
    //     insert(key, (double)x, dx);
    // }
    ShadowEntry* get(void* key) {
        uintptr_t k = (uintptr_t)key;
        size_t idx = hashPtr(key);
        
        for (size_t probe = 0; probe < TABLE_SIZE; probe++) {
            ShadowEntry &entry = table[idx];

            if (!entry.used) {
                return nullptr;
            }
            if (entry.key == k) {
                return &entry;
            }
            idx = (idx + 1) & (TABLE_SIZE - 1); 
        }
        return nullptr;
    }
    // ShadowEntry* getFloat(void* key) {
    //     return getDouble(key);
    // }
};