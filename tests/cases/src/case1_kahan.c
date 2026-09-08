#include <math.h>
#include <stdio.h>

// Case 1: Naive summation vs Kahan summation
// Cancellation accumulates over many iterations
// Large values followed by small ones causes precision loss

double naive_sum(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

double kahan_sum(double *arr, int n) {
    double sum = 0.0;
    double c = 0.0;  // compensation
    for (int i = 0; i < n; i++) {
        double y = arr[i] - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

int main() {
    // Array designed to cause cancellation:
    // one large value, many small values that should sum to cancel it
    int n = 1000;
    double arr[1000];
    arr[0] = 1e15;
    for (int i = 1; i < n - 1; i++) {
        arr[i] = 1.1;
    }
    arr[n-1] = -1e15;  // catastrophic cancellation at the end

    double r1 = naive_sum(arr, n);
    double r2 = kahan_sum(arr, n);

    // Force use of results to prevent dead code elimination
    volatile double out1 = r1;
    volatile double out2 = r2;
    return 0;
}
