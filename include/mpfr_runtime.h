#pragma once

struct fp_entry {
    double value;
    double error;
};
#ifdef __cplusplus
extern "C" {
#endif

fp_entry PropSinError(double a, double da);
fp_entry PropCosError(double a, double da);
fp_entry PropTanError(double a, double da);
fp_entry PropAsinError(double a, double da);
fp_entry PropAcosError(double a, double da);
fp_entry PropAtanError(double a, double da);
fp_entry PropLogError(double a, double da);
fp_entry PropExpError(double a, double da);
fp_entry PropPowError(double a, double da, double b, double db);

#ifdef __cplusplus
}
#endif