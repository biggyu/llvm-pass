#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double m, double kp, double ki, double kd, double c) {
	double dt = 0.5;
	double invdt = 1.0 / dt;
	double e = 0.0;
	double p = 0.0;
	double i = 0.0;
	double d = 0.0;
	double r = 0.0;
	double m_1 = m;
	double eold = 0.0;
	double t = 0.0;
	int tmp = t < 100.0;
	while (tmp) {
		e = c - m_1;
		p = kp * e;
		i = i + ((ki * dt) * e);
		d = (kd * invdt) * (e - eold);
		r = (p + i) + d;
		m_1 = m_1 + (0.01 * r);
		eold = e;
		t = t + dt;
		tmp = t < 100.0;
	}
	return m_1;
}

