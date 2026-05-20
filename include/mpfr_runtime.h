#pragma once
struct fp_entryF {
    float value;
    float error;
};
struct fp_entryD {
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

fp_entryF PropSinFError(float a, float da);
fp_entryD PropSinDError(double a, double da);
// fp_entryF PropsinfFError(float a, float da);
// fp_entryD PropsinfDError(double a, double da);
fp_entryF PropCosFError(float a, float da);
fp_entryD PropCosDError(double a, double da);
// fp_entryF PropcosfFError(float a, float da);
// fp_entryD PropcosfDError(double a, double da);
fp_entryF PropTanFError(float a, float da);
fp_entryD PropTanDError(double a, double da);
fp_entryF PropAsinFError(float a, float da);
fp_entryD PropAsinDError(double a, double da);
fp_entryF PropAcosFError(float a, float da);
fp_entryD PropAcosDError(double a, double da);
fp_entryF PropAtanFError(float a, float da);
fp_entryD PropAtanDError(double a, double da);
fp_entryF PropLogFError(float a, float da);
fp_entryD PropLogDError(double a, double da);
fp_entryF PropExpFError(float a, float da);
fp_entryD PropExpDError(double a, double da);
fp_entryF PropPowFError(float a, float da, float b, float db);
fp_entryD PropPowDError(double a, double da, double b, double db);
// fp_entryF PropexpfFError(float a, float da);
// fp_entryD PropexpfDError(double a, double da);
fp_entryF PropFabsFError(float a, float da);
fp_entryD PropFabsDError(double a, double da);
// fp_entryF PropfabsfFError(float a, float da);
// fp_entryD PropfabsfDError(double a, double da);

#ifdef __cplusplus
}
#endif