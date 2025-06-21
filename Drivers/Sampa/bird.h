

///////////////////////////////////////////////
//          HEADER FILE for BIRD.C           //
//                                           //
//             Richard E. Bird               //
// Clear Sky Broadband Solar Radiation Model //
//                                           //
//            September 19, 2012             //
//                                           //
//   Filename: BIRD.H                        //
//                                           //
//   Afshin Michael Andreas                  //
//   Afshin.Andreas@NREL.gov (303)384-6383   //
//                                           //
//   Solar Resource and Forecasting Group    //
//   Solar Radiation Research Laboratory     //
//   National Renewable Energy Laboratory    //
//   15013 Denver W Pkwy, Golden, CO 80401   //
//                                           //
//  This code is based on the SERI (NREL)    //
//  technical report "A Simplified Clear	 //
//  Sky model for Direct and Diffuse 		 //
//  Insolation on Horizontal Surfaces" by    //
//  R.E. Bird and R.L. Hulstrom              //
///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Usage:                                                               //
// 																		//
//   1) Obtain the Solar Position Algorithm from NREL (SPA.C and SPA.H) //
//           http://www.nrel.gov/midc/spa/ 								//
//                           --- OR ---									//
//      Compute your own zenith angle and earth radius vector       	//
//																		//
//   2) In calling program, include this header file,                   //
//      by adding this line to the top of file:                         //
//           #include "bird.h"                                          //
//                                                                      //
//   3) In calling program, declare the BIRD structure:                 //
//           bird_data bird;                                            //
//                                                                      //
//   4) Enter the required input values into BIRD structure             //
//      (input values listed in comments below)                         //
//                                                                      //
//   5) Call the BIRD calculate function and pass the BIRD structure    //
//      (prototype is declared at the end of this header file):         //
//           bird_calculate(&bird);                                     //
//                                                                      //
//   Output values (listed in comments below) will be                   //
//   computed and returned in the passed BIRD structure.			    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef __bird_clear_sky_model_header
#define __bird_clear_sky_model_header

typedef struct
{
	//----------------------------------INPUT VALUES--------------------------------------------
	double zenith;   //solar zenith angle [degrees] -- available from SPA output
	double r;	     //earth radius vector [Astronomical Units, AU] -- available from SPA output
	double pressure; //annual average local pressure [millibars] -- available from SPA input
	
	double sol_con;  //solar constant [W/m^2] -- earth typical is 1361.1	
	double ozone;	 //total column ozone thickness [cm] -- range from 0.05 - 0.4
	double water;    //total column water vapor [cm] -- range from 0.01 - 6.5 
	double taua;	 //broadband aerosol optical depth -- range from 0.02 - 0.5
	double ba;		 //forward scattering factor -- 0.85 recommended for rural aerosols
	double albedo;   //ground reflectance -- earth typical is 0.2, snow 0.9, vegitation 0.25 
	double dni_mod;  //direct normal irradiance modification factor -- optional value from 0.0 - 1.0,
	                 //   which is used to calculate the second set of "modified" irradiance values

	//--------------------------------- OUTPUT VALUES-------------------------------------------
	double amass;		  //relative optical airmass (not pressure corrected) 
	
	double direct_normal; //direct normal solar irradiance [W/m^2] -- Bird Clear Sky Estimated
	double global_horiz;  //global horizontal solar irradiance [W/m^2] -- Bird Clear Sky Estimated
	double diffuse_horiz; //diffuse horizontal solar irradiance [W/m^2] -- Bird Clear Sky Estimated
	
	double direct_normal_mod; //equavalent to direct_normal * dni_mod
	double global_horiz_mod;  //re-computed global horizontal based on direct_normal_mod
	double diffuse_horiz_mod; //re-computed diffuse horizontal based on direct_normal_mod
	
} bird_data;
	
void bird_calculate(bird_data *bird);


#endif