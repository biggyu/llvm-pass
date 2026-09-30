#include <math.h>
#include <stdio.h>

double f(double x) {
    return sqrt(x + 1.0) - sqrt(x);
}
double f1(double x) {
    return ((1 - cos(x)) / (x * x));
}
double f2(double x) {
    return (exp(x + 1) - exp(x)) / exp(x);
}
double f3(double x) {
    return log(x + 1) - log(x);
}
double f4(double x) {
    double eps = 1e-10;
    return sin(x + eps) - sin(x);
}
double f5(double x) {
	return (1.0 / sqrt(x)) - (1.0 / sqrt((x + 1.0)));
}
int main() {
    volatile double x = 1e100;
    double y = f(x);
    double z = y * y;
    printf("%g\n", z);
    volatile double x1 = 1e-8;
    printf("%g\n", f1(x1));
    volatile double x2 = 705;
    printf("%g\n", f2(x2));
    volatile double x3 = 1e15;
    printf("%g\n", f3(x3));
    volatile double x4 = 1e15;
    printf("%g\n", f4(x4));
    volatile double x5 = 1e99;
    printf("%g\n", f5(x5));
    return 0;
}
