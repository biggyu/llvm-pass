#include <iostream>
#include <cmath>
typedef struct {
    double value;
    double error;
} fp_entry_d;
fp_entry_d PropDivDError(double a, double da, double b, double db) {
    fp_entry_d result;
    double val = a / b;
    result.value = val;
    result.error = (da - std::fma(val, b, -a) - val * db) / (b + db);
    return result;
}

fp_entry_d PropSqrtDError(double a, double da) {
    fp_entry_d result;
    double val = std::sqrt(a);
    result.value = val;
    if (val != 0.0) {
        result.error = (da + fma(-(val), val, a)) / (2.0 * val);
    }
    else {
        double ap = a + da;
        if (ap < 0.0) {
            result.error = std::numeric_limits<double>::quiet_NaN();
        }
        result.error = sqrt(ap) - val;
    }
    return result;
}

int main() {
    double a = 1e6, b = 1e4, da = 1e-5, db = 1e-6;
    fp_entry_d res;
    res = PropDivDError(a, da, b, da);
    std::cout << res.value << " " << res.error << std::endl;
    res = PropSqrtDError(a, da);
    std::cout << res.value << " " << res.error << std::endl;
}