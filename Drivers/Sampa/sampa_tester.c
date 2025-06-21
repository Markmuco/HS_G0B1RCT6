

/////////////////////////////////////////////
//        SAMPA TESTER for SAMPA.C         //
//                                         //
//      Solar Position Algorithm (SPA)     //
//                   for                   //
//        Solar Radiation Application      //
//                                         //
//              July 13, 2017              //
//                                         //
//   Filename: SAMPA_TESTER.C              //
//                                         //
//   Afshin Michael Andreas                //
//   afshin.andreas@nrel.gov (303)384-6383 //
//                                         //
//   Metrology Laboratory                  //
//   Solar Radiation Research Laboratory   //
//   National Renewable Energy Laboratory  //
//   1617 Cole Blvd, Golden, CO 80401      //
/////////////////////////////////////////////

/////////////////////////////////////////////
// This sample program shows how to use    //
//    the SAMPA.C code.                      //
/////////////////////////////////////////////

#include <stdio.h>

#include "../Sampa/sampa.h"  //include the SAMPA header file

#if 0
int main (int argc, char *argv[])
{
    sampa_data sampa;  //declare the SAMPA structure
    int result;

    //enter required input values into SAMPA structure

    sampa.spa.year          = 2009;
    sampa.spa.month         = 7;
    sampa.spa.day           = 22;
    sampa.spa.hour          = 1;
    sampa.spa.minute        = 33;
    sampa.spa.second        = 0;
    sampa.spa.timezone      = 0;
    sampa.spa.delta_ut1     = 0;
    sampa.spa.delta_t       = 66.4;
    sampa.spa.longitude     = 143.36167;
    sampa.spa.latitude      = 24.61167;
    sampa.spa.elevation     = 0;
    sampa.spa.pressure      = 1000;
    sampa.spa.temperature   = 11;
    sampa.spa.atmos_refract = 0.5667;
    sampa.function          = SAMPA_ALL;

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

        printf("Julian Day:    %.6f\n",sampa.spa.jd);
        printf("L:             %.6e degrees\n",sampa.spa.l);
        printf("B:             %.6e degrees\n",sampa.spa.b);
        printf("R:             %.6f AU\n",sampa.spa.r);
        printf("H:             %.6f degrees\n",sampa.spa.h);
        printf("Delta Psi:     %.6e degrees\n",sampa.spa.del_psi);
        printf("Delta Epsilon: %.6e degrees\n",sampa.spa.del_epsilon);
        printf("Epsilon:       %.6f degrees\n",sampa.spa.epsilon);
        printf("Zenith:        %.6f degrees\n",sampa.spa.zenith);
        printf("Azimuth:       %.6f degrees\n",sampa.spa.azimuth);
        printf("Angular dist:  %.6f degrees\n",sampa.ems);
        printf("Sun Radius:    %.6f degrees\n",sampa.rs);
        printf("Moon Radius:   %.6f degrees\n",sampa.rm);
        printf("Area unshaded: %.6f percent\n",sampa.a_sul_pct);
        printf("DNI:           %.6f W/m^2\n",sampa.dni_sul);

    } else printf("SAMPA Error Code: %d\n", result);

    return 0;
}
#endif
/////////////////////////////////////////////
// The output of this program should be:
//
// Julian Day:    2455034.564583
// L:             2.994024e+02 degrees
// B:             -1.308059e-05 degrees
// R:             1.016024 AU
// H:             344.999100 degrees
// Delta Psi:     4.441121e-03 degrees
// Delta Epsilon: 1.203311e-03 degrees
// Epsilon:       23.439252 degrees
// Zenith:        14.512686 degrees
// Azimuth:       104.387917 degrees
// Angular dist:  0.374760 degrees
// Sun Radius:    0.262360 degrees
// Moon Radius:   0.283341 degrees
// Area unshaded: 78.363514 percent
// DNI:           719.099358 W/m^2
//
/////////////////////////////////////////////
