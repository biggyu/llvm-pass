#include <iostream>
#include <chrono>
using namespace std;

template <typename T, int N>
static double run_gausselm(int rep, bool isWarm = false) {
    static T A[N][N], x[N];

    // srand(0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) + 1);
        }
        x[i] = static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) + 1);
    }

    // for (int i = 0; i < N; i++) {
    //     for (int j = 0; j < N; j++) {
    //         cout << A[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    // cout << endl;
    // for (int i = 0; i < N; i++) {
    //     cout << x[i] << " ";
    // }
    // cout << endl;
    // cout << endl;

    auto start = std::chrono::steady_clock::now();
    // Echelon Form
    for(int i = 0; i < N; i++) {
        for(int j = i + 1; j < N; j++) {
            T ratio = A[j][i] / A[i][i];
            for(int k = 0; k < N; k++) {
                A[j][k] = A[j][k] - ratio * A[i][k];
            }
            x[j] = x[j] - ratio * x[i];
        }
    }

    // Reduced Echelon Form
    for(int i = N; i > 0 ; i--) {
        for(int j = i - 1; j > 0; j--) {
            T ratio = A[j - 1][i - 1] / A[i - 1][i - 1];
            for(int k = 0; k < N; k++) {
                A[j - 1][k] = A[j - 1][k] - ratio * A[i - 1][k];
            }
            x[j - 1] = x[j - 1] - ratio * x[i - 1];
        }
    }
    for(int i = 0; i < N; i++) {
        x[i] = x[i] / A[i][i];

        A[i][i] = 1.0;
    }
    auto end = std::chrono::steady_clock::now();

    T checksum_A = 0.0;
    T checksum_x = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum_A += A[i][j];
        }
        checksum_x += x[i];
    }

    if (isWarm) {
        // std::cout << "A checksum=" << checksum_A << std::endl;
        // std::cout << "x checksum=" << checksum_x << std::endl;
    }
    return std::chrono::duration<double, std::milli>(end - start).count();
}
int main() {
    constexpr int dim = 100;
    constexpr int warmup = 3;
    constexpr int rep = 500;
    double total_time = 0;
    srand((unsigned int)time(NULL));

    run_gausselm<double, dim>(warmup, true);

    total_time = run_gausselm<double, dim>(rep);
    std::cout << "Double type matmul execution time: " << total_time << "ms\n";
    // // std::cout << "Checksum: " << checksum << std::endl;

    // auto total_time = end - start;

    // run_gausselm<float>(dim, warmup);
    // std::cout << "Float type matmul execution time: " 
    //             << std::chrono::duration<double, std::milli>(total_time).count() << "ms" << std::endl;
    // // std::cout << "Checksum: " << checksum << std::endl;

    run_gausselm<float, dim>(warmup, true);
    
    total_time = run_gausselm<float, dim>(rep);
    std::cout << "Float type matmul execution time: " << total_time << "ms\n";
}