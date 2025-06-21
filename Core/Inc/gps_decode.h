/*
 * gps_decode.h
 *
 *  Created on: 25 sep. 2018
 *      Author: VM
 */

#ifndef GPS_DECODE_H_
#define GPS_DECODE_H_

typedef struct
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
}td_t;

float geo_coord_to_deg(char *str);
float geo_distance(char *p_lat1, char *p_lon1, char *p_lat2, char *p_lon2);
bool str_to_td(char *str, td_t * abc);

#endif /* GPS_DECODE_H_ */
