/*!
 * \file gps_geo.c
 *
 * \brief This software module provides functions for geographic calculations.
 *
 * Based on: http://www.movable-type.co.uk/scripts/latlong.html
 *           http://andrew.hedges.name/experiments/convert_lat_long/
 *           http://andrew.hedges.name/experiments/haversine/
 *
 * Loc. 1: (5223.0340N, 446.0560E) -> (52.3839, 4.7676)
 * Loc. 2: (5223.0940N, 450.6700E) -> (52.3849, 4.8445)
 * Distance between loc. 1 and loc. 2: 5222m
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "ctype.h"
#include "gps_decode.h"

// Private constants.
static const double DEG_TO_RAD = 0.017453292519943295769236907684886; // PI / 180

static const double EARTH_RADIUS_IN_METERS = 6372797.560856;

/*!
 * \brief This function converts a NMEA-0183 string representing date or time into numeric degrees.
 *
 * \param str A pointer to a NMEA-0183 time (hhmmss) or date (ddmmyy) string.
 *
 * \return pointer The converted string in numeric degrees.
 */
bool str_to_td(char *str, td_t * abc)
{
	char buf[2];

	if (strlen(str) != 6)
		return false;

	buf[0] = *str++;
	buf[1] = *str++;
	abc->a = atoi(buf);

	buf[0] = *str++;
	buf[1] = *str++;
	abc->b = atoi(buf);

	buf[0] = *str++;
	buf[1] = *str++;
	abc->c = atoi(buf);

	return true;
}

/*!
 * \brief This function converts a NMEA-0183 string representing degrees/minutes into numeric degrees.
 *
 * \param str A pointer to a NMEA-0183 latitude (ddmm.mmmm[N/S]) or longitude (dddmm.mmmm[E/W]) string.
 *
 * \return The converted string in numeric degrees.
 */
float geo_coord_to_deg(char *str)
{
	char *p, *q;
	uint8_t i, deg = 0, min = 0;
	uint16_t frac_min = 0;

	// Scan for decimal point or end of field.
	for (p = str; isdigit(*p); p++);
	q = str;

	// Convert degrees.
	while ((p - q) > 2)
	{
		if (deg)
		deg *= 10;
		deg += *q++ - '0';
	}

	// Convert minutes.
	while (p > q)
	{
		if (min)
		min *= 10;
		min += *q++ - '0';
	}

	// Convert fractional minutes (expect up to 4 digits).
	if (*p == '.')
	{
		q = p + 1;
		for (i = 0; i < 4; i++)
		{
			frac_min *= 10;
			if (isdigit(*q))
			frac_min += *q++ - '0';
		}
	}

    p++;
    // Scan for decimal point or end of field.
	for (; isdigit(*p); p++);

    if ((*p == 'S') || (*p == 'W'))
        return (deg + ((min + (frac_min / 10000.0)) / 60.0)) * -1;
    else
        return (deg + ((min + (frac_min / 10000.0)) / 60.0));

}

/*!
 * \brief This function calculates the distance between two sets of latitude longitude coordinates.
 *
 * \param p_lat1 A pointer to a NMEA-0183 latitude (ddmm.mmmm[N/S]) string.
 * \param p_lon1 A pointer to a NMEA-0183 longitude (dddmm.mmmm[E/W]) string.
 * \param p_lat2 A pointer to a NMEA-0183 latitude (ddmm.mmmm[N/S]) string.
 * \param p_lon2 A pointer to a NMEA-0183 longitude (dddmm.mmmm[E/W]) string.
 *
 * \return The distance between two sets of lat. long. coordinates in meters.
 */
float geo_distance(char *p_lat1, char *p_lon1, char *p_lat2, char *p_lon2)
{
	double lat1, lon1, lat2, lon2, dlat, dlon, a, c;

	// Convert latitude longitude pair 1 from a NMEA-0183 string to radians.
	lat1 = geo_coord_to_deg(p_lat1) * DEG_TO_RAD;
	if (strchr(p_lat1, 'S'))
		lat1 = -lat1;

	lon1 = geo_coord_to_deg(p_lon1) * DEG_TO_RAD;
	if (strchr(p_lon1, 'W'))
		lon1 = -lon1;

	// Convert latitude longitude pair 2 from a NMEA-0183 string to radians.
	lat2 = geo_coord_to_deg(p_lat2) * DEG_TO_RAD;
	if (strchr(p_lat2, 'S'))
		lat2 = -lat2;

	lon2 = geo_coord_to_deg(p_lon2) * DEG_TO_RAD;
	if (strchr(p_lon2, 'W'))
		lon2 = -lon2;

	// Calculate the distance by the haversine formula.
	dlat = lat2 - lat1;
	dlon = lon2 - lon1;

	a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);

	c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return (EARTH_RADIUS_IN_METERS * c);
}
