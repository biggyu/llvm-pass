#include <iostream>
// #include <cstdio>
// #include <cstdlib>
#include <chrono>

template <typename T, int N>
static double run_matmul(int rep, bool isWarm = false) {
    static T mat1[N][N], mat2[N][N], res[N][N];

    srand(0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mat1[i][j] = static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) + 1);
            mat2[i][j] = static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) + 1);
        }
    }
    // asm volatile("" : : "r"(&mat1[0][0]), "r"(&mat2[0][0]) : "memory");

    auto start = std::chrono::steady_clock::now();
    for (int k = 0; k < rep; k++) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                // res[i][j] = 0;
                // for (int k = 0; k < N; k++) {
                //     res[i][j] += mat1[i][k] * mat2[k][j];
                // }
                T s = 0;
                for (int k = 0; k < N; k++) {
                    s += mat1[i][k] * mat2[k][j];
                }
                res[i][j] = s;
            }
        }
        // asm volatile("" : : "r"(&res) : "memory");
        // mat1[k % N][k % N] += res[k % N][k % N] * static_cast<T>(1e-30);
    }
    auto end = std::chrono::steady_clock::now();

    T checksum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += res[i][j];
        }
    }
    // asm volatile("" :: "g"(checksum));
    if (isWarm) {
        std::cout << "checksum=" << checksum << std::endl;
    }
    return std::chrono::duration<double, std::milli>(end - start).count();
}
int main() {
    constexpr int dim = 100;
    constexpr int warmup = 3;
    constexpr int rep = 500;
    double total_time = 0;

    run_matmul<double, dim>(warmup, true);

    total_time = run_matmul<double, dim>(rep);
    std::cout << "Double type matmul execution time: " << total_time << "ms\n";
    // // std::cout << "Checksum: " << checksum << std::endl;

    // auto total_time = end - start;

    // run_matmul<float>(dim, warmup);
    // std::cout << "Float type matmul execution time: " 
    //             << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;
    // // std::cout << "Checksum: " << checksum << std::endl;

    run_matmul<float, dim>(warmup, true);
    
    total_time = run_matmul<float, dim>(rep);
    std::cout << "Float type matmul execution time: " << total_time << "ms\n";
    
}