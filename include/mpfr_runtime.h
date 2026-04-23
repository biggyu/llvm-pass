#pragma once
struct fp_entry_f {
    float value;
    float error;
};
struct fp_entry_d {
    double value;
    double error;
};
#ifdef __cplusplus
extern "C" {
#endif

//TODO: merge into below
// void PropUnFloatError(float a, float da);
// void PropBinFloatError(float a, float da, float b, float db);
// void PropUnDoubleError(double a, double da);
// void PropBinDoubleError(double a, double da, double b, double db);

fp_entry_f PropSinFError(float a, float da);
fp_entry_d PropSinDError(double a, double da);
// fp_entry_f PropsinfFError(float a, float da);
// fp_entry_d PropsinfDError(double a, double da);
fp_entry_f PropCosFError(float a, float da);
fp_entry_d PropCosDError(double a, double da);
// fp_entry_f PropcosfFError(float a, float da);
// fp_entry_d PropcosfDError(double a, double da);
fp_entry_f PropTanFError(float a, float da);
fp_entry_d PropTanDError(double a, double da);
fp_entry_f PropAsinFError(float a, float da);
fp_entry_d PropAsinDError(double a, double da);
fp_entry_f PropAcosFError(float a, float da);
fp_entry_d PropAcosDError(double a, double da);
fp_entry_f PropAtanFError(float a, float da);
fp_entry_d PropAtanDError(double a, double da);
fp_entry_f PropLogFError(float a, float da);
fp_entry_d PropLogDError(double a, double da);
fp_entry_f PropExpFError(float a, float da);
fp_entry_d PropExpDError(double a, double da);
fp_entry_f PropPowFError(float a, float da, float b, float db);
fp_entry_d PropPowDError(double a, double da, double b, double db);
// fp_entry_f PropexpfFError(float a, float da);
// fp_entry_d PropexpfDError(double a, double da);
fp_entry_f PropFabsFError(float a, float da);
fp_entry_d PropFabsDError(double a, double da);
// fp_entry_f PropfabsfFError(float a, float da);
// fp_entry_d PropfabsfDError(double a, double da);

#ifdef __cplusplus
}
#endif