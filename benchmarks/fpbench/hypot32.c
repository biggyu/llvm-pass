#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float x1, float x2) {
	return sqrtf(((x1 * x1) + (x2 * x2)));
}

