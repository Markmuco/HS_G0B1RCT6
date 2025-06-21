/*
 * test_functions.h
 *
 *  Created on: 5 dec. 2022
 *      Author: Mark
 */

#ifndef TEST_FUNCTIONS_H_
#define TEST_FUNCTIONS_H_


#define CMD_ID				0x0000	//	1	Read	Slave fixed ID = 0x100

#define CMD_READ_CNT		0x0001	//	1	Increase on any read

#define CMD_POS_X_HIGH		0x0002
#define CMD_POS_X_LOW		0x0003

#define CMD_POS_Y_HIGH		0x0004
#define CMD_POS_Y_LOW		0x0005

#define CMD_MOVE_X			0x0006
#define CMD_MOVE_Y			0x0007

#define CMD_OPMODE			0x0008	// 0: stop
									// 1: follow sun
									// 2: follow TG1
									// 3: follow TG2
									// 4: follow TG3
									// 5: follow TG4
									// 6: follow TG5

#define CMD_SAVE			0x0009	// 0: stop
									// 1: save sun
									// 2: save TG1
									// 3: save TG2
									// 4: save TG3
									// 5: save TG4
									// 6: save TG5

#define CMD_STEPS_X			0x000A	// steps per decree
#define CMD_STEPS_Y			0x000B	// steps per decree

#define CMD_WIND			0x000C	// pulses per minute
#define CMD_WIND_SET		0x000D	// pulses per minute


#define CMD_MINUTES_HIGH	0x000E	//
#define CMD_MINUTES_LOW		0x000F	//
#define CMD_GPS_VALID		0x0010	//
#define CMD_MAX_HOR_HIGH	0x0011	//
#define CMD_MAX_HOR_LOW		0x0012	//
#define CMD_MAX_VER_HIGH	0x0013	//
#define CMD_MAX_VER_LOW		0x0014	//
#define CMD_CALIBRATE_HOR	0x0015	//
#define CMD_CALIBRATE_VER	0x0016	//
#define CMD_DEVIATION_HOR	0x0017	//
#define CMD_DEVIATION_VER	0x0018	//
#define CMD_SUNDOWN_ANGLE	0x0019	//
#define CMD_SUN_ELEVATION	0x001A	//
#define CMD_SUN_AZIMUTH		0x001B	//


void process_modbus_slave(void);


#endif /* TEST_FUNCTIONS_H_ */
