#pragma once

void powell(float p[], float **xi, int n, float ftol, int *iter, float *fret,
	float(*func)(float *), int maxIter);