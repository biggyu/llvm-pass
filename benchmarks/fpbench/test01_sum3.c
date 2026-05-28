#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float x0, float x1, float x2) {
	float p0 = (x0 + x1) - x2;
	float p1 = (x1 + x2) - x0;
	float p2 = (x2 + x0) - x1;
	return (p0 + p1) + p2;
}

