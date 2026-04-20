#include <iostream>
// #include <cstdio>
// #include <cstdlib>
#include <vector>
#include <chrono>

template <typename T>
static double run_matmul(int dim, int rep) {
    std::vector<std::vector<T>> mat1(dim, std::vector<T>(dim));
    std::vector<std::vector<T>> mat2(dim, std::vector<T>(dim));
    std::vector<std::vector<T>> res(dim, std::vector<T>(dim, 0.0));

    srand(0);
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            mat1[i][j] = ((T)rand()) / ((T)RAND_MAX + 1);
            mat2[i][j] = ((T)rand()) / ((T)RAND_MAX + 1);
        }
    }

    auto start = std::chrono::steady_clock::now();
    for (int k = 0; k < rep; k++) {
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                res[i][j] = 0;
                for (int k = 0; k < dim; k++) {
                    res[i][j] += mat1[i][k] * mat2[k][j];
                }
            }
        }
    }
    auto end = std::chrono::steady_clock::now();

    double checksum = 0.0;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            checksum += res[i][j];
        }
    }
    auto tot = end - start;
    return std::chrono::duration<double, std::milli>(tot).count();
}
int main() {
    static int dim = 100;
    static int warmup = 3;
    static int rep = 500;
    auto total_time = 0;

    run_matmul<double>(dim, warmup);
    // std::cout << "Double type matmul execution time: " 
    //             << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;
    // std::cout << "Checksum: " << checksum << std::endl;

    total_time = run_matmul<double>(dim, rep);
    // printf("Double type matmul exeuction time: %fms\n", std::chrono::duration<double, std::milli>(total_time).count());
    std::cout << "Double type matmul execution time: " 
                << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;
    // // std::cout << "Checksum: " << checksum << std::endl;

    // auto total_time = end - start;

    // run_matmul<float>(dim, warmup);
    // std::cout << "Float type matmul execution time: " 
    //             << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;
    // // std::cout << "Checksum: " << checksum << std::endl;
    
    total_time = run_matmul<float>(dim, rep);
    // printf("Float type matmul exeuction time: %fms\n", std::chrono::duration<double, std::milli>(total_time).count());
    std::cout << "Float type matmul execution time: " 
                << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;

    // report_fp_profile();
    // report_smem_profile();
}