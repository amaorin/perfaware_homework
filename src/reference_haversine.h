#include <math.h>

#define RADS_FROM_DEGS(D) ((D)*0.01745329251994329577)

#define EARTH_RADIUS 6372.8

f64
ReferenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 earth_radius)
{
	f64 dlat = RADS_FROM_DEGS(y1 - y0);
	f64 dlon = RADS_FROM_DEGS(x1 - x0);
	f64 lat0 = RADS_FROM_DEGS(y0);
	f64 lat1 = RADS_FROM_DEGS(y1);

	f64 sin_hdlat = sin(dlat/2);
	f64 sin_hdlon = sin(dlon/2);

	f64 a = sin_hdlat*sin_hdlat  + cos(lat0)*cos(lat1)*sin_hdlon*sin_hdlon;
	f64 c = 2*asin(sqrt(a));

	return c * earth_radius;
}
