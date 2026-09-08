#include <math.h>
#include <stdio.h>

// Case 3: Naive polynomial evaluation vs Horner's method
// Polynomial: (x-1)^7 expanded
// = x^7 - 7x^6 + 21x^5 - 35x^4 + 35x^3 - 21x^2 + 7x - 1
// Near x=1, large intermediate terms nearly cancel
// Rump's polynomial is the classic example of this failure mode

double poly_naive(double x) {
    // Explicit expansion of (x-1)^7
    // Near x=1: terms like x^7 ≈ 1 and -7x^6 ≈ -7 etc.
    // Alternating large terms cancel catastrophically
    return (x*x*x*x*x*x*x)
         - 7.0*(x*x*x*x*x*x)
         + 21.0*(x*x*x*x*x)
         - 35.0*(x*x*x*x)
         + 35.0*(x*x*x)
         - 21.0*(x*x)
         + 7.0*x
         - 1.0;
}

double poly_horner(double x) {
    // Horner's method: p = (...((a7*x + a6)*x + a5)*x + ... + a0)
    // Avoids computing large powers explicitly
    // Same coefficients, numerically stable evaluation
    return ((((((x - 7.0)*x + 21.0)*x - 35.0)*x + 35.0)*x - 21.0)*x + 7.0)*x - 1.0;
}

// Direct computation using the factored form (ground truth)
double poly_exact(double x) {
    double t = x - 1.0;
    return t*t*t*t*t*t*t;  // (x-1)^7 directly
}

int main() {
    // x slightly above 1: large cancellation in naive version
    // The closer to 1, the more dramatic the cancellation
    double x = 1.0 + 1e-10;

    double r1 = poly_naive(x);
    double r2 = poly_horner(x);
    double r3 = poly_exact(x);

    volatile double out1 = r1;
    volatile double out2 = r2;
    volatile double out3 = r3;
    return 0;
}
