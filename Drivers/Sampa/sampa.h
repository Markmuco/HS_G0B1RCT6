
///////////////////////////////////////////////
//         HEADER FILE for SAMPA.C           //
//                                           //
// Solar and Moon Position Algorithm (SAMPA) //
//                   for                     //
//        Solar Radiation Application        //
//                                           //
//              August 1, 2012               //
//                                           //
//   Filename: SAMPA.H                       //
//                                           //
//   Afshin Michael Andreas                  //
//   Afshin.Andreas@NREL.gov (303)384-6383   //
//                                           //
//   Solar Resource and Forecasting Group    //
//   Solar Radiation Research Laboratory     //
//   National Renewable Energy Laboratory    //
//   15013 Denver W Pkwy, Golden, CO 80401   //
//                                           //
//  This code is based on the NREL           //
//  technical report "Solar Eclipse          //
//  Monitoring for Solar Energy Applications //
//  using the Solar and Moon Position 		 //
//  Algorithms" by Ibrahim Reda              //
///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Usage:                                                               //
//                                                                      //
//   1) Obtain the Solar Position Algorithm from NREL (SPA.C and SPA.H) //
//           http://www.nrel.gov/midc/spa/                              //
//                                                                      //
//   2) In calling program, include this header file,                   //
//      by adding this line to the top of file:                         //
//           #include "sampa.h"                                         //
//                                                                      //
//   3) In calling program, declare the SAMPA structure:                //
//           sampa_data sampa;                                          //
//                                                                      //
//   4) Enter the required input values into SAMPA.SPA structure        //
//      (see below, most input values listed in SPA.H comments)         //
//                                                                      //
//   5) Call the SAMPA calculate function and pass the SAMPA structure  //
//      (prototype is declared at the end of this header file):         //
//           sampa_calculate(&sampa);                                   //
//                                                                      //
//   Output values (listed in comments below) will be computed          //
//   and returned in the passed SAMPA structure.  Output will           //
//   be based on function code selected from enumeration below.         // 
//                                                                      //
//   Note: A non-zero return code from sampa_calculate() indicates that //
//         one of the input values did not pass simple bounds tests.    //
//         The valid input ranges and return error codes are also       //
//         listed in SPA.H comments.                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef __sun_and_moon_position_algorithm_header
#define __sun_and_moon_position_algorithm_header

#include "../Sampa/spa.h"

//enumeration for function codes to select desired final outputs from SAMPA
enum {
    SAMPA_NO_IRR,  //calculate all values except estimated solar irradiances
    SAMPA_ALL      //calculate all values
};

typedef struct
{
	//-----------------Intermediate MPA OUTPUT VALUES--------------------   
	
	double l_prime;		//moon mean longitude [degrees]
	double d;			//moon mean elongation [degrees]
	double m;			//sun mean anomaly [degrees]
	double m_prime;		//moon mean anomaly [degrees]
	double f;           //moon argument of latitude [degrees]
	double l;			//term l
	double r;			//term r
	double b;			//term b
	double lamda_prime; //moon longitude [degrees]
	double beta;		//moon latitude [degrees]
	double cap_delta;   //distance from earth to moon [kilometers]
	double pi;          //moon equatorial horizontal parallax [degrees] 
	double lamda;       //apparent moon longitude [degrees]
	
    double alpha;       //geocentric moon right ascension [degrees]
    double delta;       //geocentric moon declination [degrees]

    double h;           //observer hour angle [degrees]
    double del_alpha;   //moon right ascension parallax [degrees]
    double delta_prime; //topocentric moon declination [degrees]
    double alpha_prime; //topocentric moon right ascension [degrees]
    double h_prime;     //topocentric local hour angle [degrees]

    double e0;          //topocentric elevation angle (uncorrected) [degrees]
    double del_e;       //atmospheric refraction correction [degrees]
    double e;           //topocentric elevation angle (corrected) [degrees]

    //---------------------Final MPA OUTPUT VALUES------------------------

    double zenith;        //topocentric zenith angle [degrees]
    double azimuth_astro; //topocentric azimuth angle (westward from south) [for astronomers]
    double azimuth;       //topocentric azimuth angle (eastward from north) [for navigators and solar radiation]

} mpa_data; //Moon Position Algorithm (MPA) structure

typedef struct
{
	spa_data spa; //Enter required INPUT VALUES into SPA structure (see SPA.H)
	              //spa.function will be forced to SPA_ZA, therefore slope & azm_rotation not required)
	
	mpa_data mpa; //Moon Position Algorithm structure (defined above)
	
	int function; //Switch to choose functions for desired output (from enumeration)
	
	//---------INPUT VALUES required for estimated solar irradiances--------

	double bird_sol_con;//solar constant [W/m^2] -- earth typical is 1361.1	
	double bird_ozone;  //total column ozone thickness [cm] -- range from 0.05 - 0.4
	double bird_pwv;    //total column water vapor [cm] -- range from 0.01 - 6.5 
	double bird_aod;    //broadband aerosol optical depth -- range from 0.02 - 0.5
	double bird_ba;	    //forward scattering factor -- 0.85 recommended for rural aerosols
	double bird_albedo; //ground reflectance -- earth typical is 0.2, snow 0.9, vegitation 0.25 
	
	//---------------------Final SAMPA OUTPUT VALUES------------------------
	
	double ems; //local observed, topocentric, angular distance between sun and moon centers [degrees]
	double rs;	//radius of sun disk [degrees]
	double rm;  //radius of moon disk [degrees]
	
	double a_sul;     //area of sun's unshaded lune (SUL) during eclipse [degrees squared]
	double a_sul_pct; //percent area of SUL during eclipse [percent]
	
	double dni;       //estimated direct normal solar irradiance using SERI/NREL Bird Clear Sky Model [W/m^2]
	double dni_sul;   //estimated direct normal solar irradiance from the sun's unshaded lune [W/m^2]

	double ghi;       //estimated global horizontal solar irradiance using SERI/NREL Bird Clear Sky Model [W/m^2]
	double ghi_sul;   //estimated global horizontal solar irradiance from the sun's unshaded lune [W/m^2]

	double dhi;       //estimated diffuse horizontal solar irradiance using SERI/NREL Bird Clear Sky Model [W/m^2]
	double dhi_sul;   //estimated diffuse horizontal solar irradiance from the sun's unshaded lune [W/m^2]
	
} sampa_data; //Solar and Moon Position Algorithm (SAMPA) structure

//Calculate SAMPA output values (in structure) based on input values passed in structure
int sampa_calculate(sampa_data *sampa);

#endif