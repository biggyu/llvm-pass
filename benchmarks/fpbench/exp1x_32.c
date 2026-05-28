#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float x) {
	return (expf(x) - 1.0f) / x;
}

