/*
 * suncalc.h
 *
 *  Created on: 6 mei 2017
 *      Author: Mark
 */

#ifndef SUNCALC_H_
#define SUNCALC_H_

void suncalc(location_t home_pos, time_date_t time_date, coord_t *sunpos, coord_t *moonpos);


#define sun_azimuth				vars.sunpos.azimuth
#define sun_elevation			vars.sunpos.elevation
#define motor_pos_x				vars.goto_pos.x
#define motor_pos_y				vars.goto_pos.y


#define location_latitude		vars.hwinfo.home_location.latitude
#define location_longitude		vars.hwinfo.home_location.longitude
#define stepX					vars.hwinfo.steps.x
#define stepY					vars.hwinfo.steps.y
#define hw_offset_y				vars.hwinfo.hw_offset.y
#define hw_offset_x				vars.hwinfo.hw_offset.x
#define maxX					vars.hwinfo.maximum.x
#define maxY					vars.hwinfo.maximum.y

#define mirror_azimuth			vars.eevar.actual_motor.x
#define mirror_elevation		vars.eevar.actual_motor.y
#define target_pos_x			vars.tokeep.target.x
#define target_pos_y			vars.tokeep.target.y

bool follow_target(coord_t sunmoonpos);
bool follow_sun(void);

#endif /* SUNCALC_H_ */
