#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

class ShadowTable {
public:
    struct ShadowEntry{
        uintptr_t key;
        double value;
        double error;
        bool used;
    };
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
    void insert(void* key, double x, double dx) {
        uintptr_t k = (uintptr_t)key;
        size_t idx = hashPtr(key);
        for (size_t probe = 0; probe < TABLE_SIZE; probe++) {
            ShadowEntry &entry = table[idx];

            if (!entry.used) {
                entry.used = true;
                entry.key = k;
                entry.value = x;
                entry.error = dx;
                return;
            }
            //! Update value or just error
            if (entry.key == k) {
                entry.error = dx;
                return;
            }
            idx = (idx + 1) & (TABLE_SIZE - 1); 
        }
    }
    void insert(void* key, float x, double dx) {
        insert(key, (double)x, dx);
    }
    double getDouble(void* key) {
        uintptr_t k = (uintptr_t)key;
        size_t idx = hashPtr(key);
        
        for (size_t probe = 0; probe < TABLE_SIZE; probe++) {
            ShadowEntry &entry = table[idx];

            if (!entry.used) {
                return 0.0;
            }
            if (entry.key == k) {
                return entry.error;
            }
            idx = (idx + 1) & (TABLE_SIZE - 1); 
        }
        return 0.0;
    }
    float getFloat(void* key) {
        return (float)getDouble(key);
    }
};