/*
 * solcalc.c
 *
 *  Created on: 12 jan. 2019
 *      Author: Mark
 */

#include "main.h"
#include "rtc.h"
#include "suncalc.h"
#include "vector.h"
#include "uart_sci.h"
#include "sampa.h"


void suncalc(location_t home_pos, time_date_t time_date, coord_t *sunpos)
{
    static sampa_data sampa;  //declare the SAMPA structure
    int result;

    sampa.spa.year          = 2025;
    sampa.spa.month         = 6;
    sampa.spa.day           = 6;
    sampa.spa.hour          = 17;
    sampa.spa.minute        = 0;
    sampa.spa.second        = 0;
    sampa.spa.timezone      = 0;
    sampa.spa.delta_ut1     = 0;
    sampa.spa.delta_t       = 66.4;
    sampa.spa.longitude     = 5.089;
    sampa.spa.latitude      = 52.647;
    sampa.spa.elevation     = 0;
    sampa.spa.pressure      = 1000;
    sampa.spa.temperature   = 11;
    sampa.spa.atmos_refract = 0.5667;
    sampa.function          = SAMPA_NO_IRR;

	sampa.bird_sol_con = 1367.0;
	sampa.bird_ozone   = 0.3;
	sampa.bird_pwv     = 1.5;
	sampa.bird_aod     = 0.07637;
	sampa.bird_ba      = 0.85;
	sampa.bird_albedo  = 0.2;

    //call the SAMPA calculate function and pass the SAMPA structure

   	result = sampa_calculate(&sampa);

    if (result == 0)  //check for SPA errors
    {
        //display the results inside the SAMPA structure

//        printf("Julian Day:    %.6f\r\n",sampa.spa.jd);
//        printf("L:             %.6e degrees\r\n",sampa.spa.l);
//        printf("B:             %.6e degrees\r\n",sampa.spa.b);
//        printf("R:             %.6f AU\r\n",sampa.spa.r);
//        printf("H:             %.6f degrees\r\n",sampa.spa.h);
//        printf("Delta Psi:     %.6e degrees\r\n",sampa.spa.del_psi);
//        printf("Delta Epsilon: %.6e degrees\r\n",sampa.spa.del_epsilon);
//        printf("Epsilon:       %.6f degrees\r\n",sampa.spa.epsilon);
        printf("Moon elevation:        %.6f degrees\r\n", 90 - sampa.mpa.zenith);
        printf("Moon Azimuth:       %.6f degrees\r\n",sampa.mpa.azimuth);

        printf("Sun elevation:        %.6f degrees\r\n",90 - sampa.spa.zenith);
        printf("Sun Azimuth:       %.6f degrees\r\n",sampa.spa.azimuth);

//        printf("Angular dist:  %.6f degrees\r\n",sampa.ems);
//        printf("Sun Radius:    %.6f degrees\r\n",sampa.rs);
//        printf("Moon Radius:   %.6f degrees\r\n",sampa.rm);
//        printf("Area unshaded: %.6f percent\r\n",sampa.a_sul_pct);
//        printf("DNI:           %.6f W/m^2\r\n",sampa.dni_sul);

    }



	sunpos->azimuth = sampa.spa.azimuth;
	sunpos->elevation = 90 - sampa.spa.zenith;




}

/*******************************************************************************
 *  Calculate the new x and y pos for sun tracking.
 *
 *  Return True: out_of_range
 */
bool follow_sun(void)
{
	motorpos_t mp;

	// the coordinates of the sun into motor values
	if ((location_latitude < 0) && (sun_azimuth < 180))           // quadrant 1: 360-450�
		mp.x = ((360 + sun_azimuth) * stepX) - hw_offset_x;
	else
		mp.x = (sun_azimuth * stepX) - hw_offset_x;

	mp.y = (sun_elevation * stepX) - hw_offset_y;

	// keep 1º from end points
	if ((mp.x > stepX) && (mp.x < (maxX - stepX)) && (mp.y > stepY) && (mp.y < (maxY - stepY))) // && daylight)
	{
//		vars.out_of_range = false;
		vars.goto_motor = mp;
		return false;
	}
	else
	{
		tty_printf("out of range\n\r");
		//vars.out_of_range = true;
	}
	return true;
}

/*******************************************************************************
 *  Calculate the new x and y pos for follow target
 *
 *	Return True: out_of_range
 */
bool follow_target(void)
{
	motorpos_t mp;

	calc_mirror_pos(&vars.hwinfo, vars.sunpos, &mp, vars.eevar.target);

	// keep 1º from end points
	if ((mp.x > stepX) && (mp.x < (maxX - stepX)) && (mp.y > stepY) && (mp.y < (maxY - stepY)))
	{
		vars.goto_motor = mp;
//		vars.out_of_range = false;
		return false;
	}
	else
		tty_printf("out of range\n\r");

	return true;
}
