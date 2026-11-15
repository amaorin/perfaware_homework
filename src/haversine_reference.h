#include "common.h"

#include <math.h>

#define TAU64 6.283185307179586

#define EARTH_RADIUS_METERS 6378.137

f64
DegreesToRadians(f64 degs)
{
	return degs * (TAU64/360);
}

f64
Square(f64 x)
{
	return x * x;
}

f64
ReferenceHaversineOfDegrees(f64 x0_degs, f64 y0_degs, f64 x1_degs, f64 y1_degs, f64 earth_radius)
{
	f64 dy_degs = y1_degs - y0_degs;
	f64 dx_degs = x1_degs - x0_degs;

	f64 dy = DegreesToRadians(dy_degs);
	f64 dx = DegreesToRadians(dx_degs);
	f64 y0 = DegreesToRadians(y0_degs);
	f64 y1 = DegreesToRadians(y1_degs);

	f64 root_term = Square(sin(dy / 2)) + cos(y0)*cos(y1)*Square(sin(dx / 2));
	f64 result    = 2*earth_radius*asin(sqrt(root_term));

	return result;
}
