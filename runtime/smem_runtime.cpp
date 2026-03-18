#include "smem_runtime.h"
#include "shadow_table.h"
// #include <unordered_map>
// #include <cstdint>
// #include <iostream>

static ShadowTable s_tbl;
extern "C" void shadow_store(void* addr, double x, double dx) {
    // std::cout << "SHADOW STORE" << std::endl;
    // std::cout << x << " " << dx << std::endl;
    //std::cout << x << " " << dx << std::endl;
    s_tbl.insert(addr, x, dx);
    // smem[(std::uintptr_t)addr] = ShadowEntry{x, dx};
} 
extern "C" double shadow_load(void* addr) {
    // auto search = smem.find((std::uintptr_t)addr);
    // std::cout << "SHADOW LOAD\n";
    // for (const auto& pair : smem) {
    //     std::cout << pair.first << " " << pair.second.x << " " << pair.second.dx << std::endl;
    // }
    // std::cout << std::endl;
    return s_tbl.get(addr);

    // return search == smem.end() ? 0.0 : search->second.dx;
    // // if (search == smem.end()) {
    // //     return 0.0;
    // // }
    // // else {
    // //     return search->dx;
    // // }
}
