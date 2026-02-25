#include <stdio.h>
// #include "../TwoSum/TwoSum_rt.hpp"

// void TwoSum(double a, double b, double& x, double& dx) {
//     x = a + b;
//     double bp = x - a;
//     double ap = x - bp;
//     double da = a - ap;
//     double db = b - bp;
//     dx = da + db;
//     printf("%f %f", x, dx);
// }

// float addf(float a, float b) {
//     float x = a + b;
//     float y = x + 1.25f;
//     return y;
// }

// double addd(double a, double b) {
//     double s = a + b;
//     double t = s + 1.0;
//     return t;
// }

int main(void) {
    float  f1 = 1.0f, f2 = 1e-7f;
    double d1 = 1e3, d2 = 1.0;

    float rf1 = f1 + f2;
    double rd1 = d1 + d2;
    float rf2 = rf1 - f2;
    double rd2 = rd1 - d2;

    printf("rf1 = %f\n", rf1);
    printf("rd1 = %.17g\n", rd1);
    printf("rf2 = %f\n", rf2);
    printf("rd2 = %.17g\n", rd2);
    return 0;
}