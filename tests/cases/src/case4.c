#include <math.h>
#include <stdio.h>

// Naive quadratic formula — catastrophic cancellation
// when -b and sqrt(b²-4ac) nearly cancel
double quadratic_r1_naive(double a, double b, double c) {
    return (-b + sqrt(b*b - 4*a*c)) / (2*a);
}

// Numerically stable version
double quadratic_r1_stable(double a, double b, double c) {
    double disc = sqrt(b*b - 4*a*c);
    // Use sign of b to avoid cancellation
    double q = (b > 0) ? -(b + disc) / 2.0 : (-b + disc) / 2.0;
    return q / a;
}

int main(void) {
    // Input where cancellation is latent but not yet severe
    // EFT: small error   Condition number: large
    volatile double r1 = quadratic_r1_naive(1.0, 100.0, 1.0);

    // Nearby input where cancellation becomes catastrophic
    // EFT: large error   Condition number: large
    volatile double r2 = quadratic_r1_naive(1.0, 10000.0, 1.0);

    // Stable version: both inputs clean
    volatile double r3 = quadratic_r1_stable(1.0, 100.0, 1.0);
    volatile double r4 = quadratic_r1_stable(1.0, 10000.0, 1.0);

    // b=1e8: EFT fires and condition number fires — both confirm disaster
    volatile double r5 = quadratic_r1_naive(1.0, 1e8, 1.0);

    // b=1e4: EFT quiet, condition number large (latent danger)  
    volatile double r6 = quadratic_r1_naive(1.0, 1e4, 1.0);
    return 0;
}