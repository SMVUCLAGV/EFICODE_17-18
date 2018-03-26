// numericalIntegration.cpp

#include "numericalIntegration.h"

double trapezoidalNumericalIntegration(
	const double* f, 
	const double* x, 
	unsigned long int size
	)
{
	double result = 0;
	
	for (unsigned long int i = 0; i < size - 1; i++)
	{
		result += (x[i + 1] - x[i]) * (f[i + 1] + f[i]); 
	}
	
	result *= 0.5;
	
	return result;
}
