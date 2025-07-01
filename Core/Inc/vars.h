/*
 * vars.h
 *
 *  Created on: 13 aug. 2019
 *      Author: Mark
 */

#ifndef VARS_H_
#define VARS_H_




#define APP_KEY   	(0xAA55AA55)
#define VERSION   	(0x0700FFFA)
#define CAN_SERIAL  (0x20250013)

#define WAIT_KEY  	(0x55AA55AA)
#define WAIT_KEY_1	(0x55AA5501) // Wait by usart1
#define WAIT_KEY_2	(0x55AA5502) // Wait by usart2
#define WAIT_KEY_3	(0x55AA5503) // Wait by usart3
#define CAN_KEY		(0x112255AA)

#define CRASH_KEY 	(0x55AA55CC)

#define RAM_KEY   	(0x20000000) // RAM address
#define RAM_SERIAL	(0x20000004)

#define KEY_STOP  	(0x55AA55AA)


#define ERR_NONE	0x00
#define ERR_HAL_XA	0x01
#define ERR_HAL_XB	0x02
#define ERR_HAL_YA	0x04
#define ERR_HAL_YB	0x08

#define ERR_CLX		0x10
#define ERR_CLY		0x20
#define ERR_OTX		0x40
#define ERR_OTY		0x80

#define ERR_END_X	0x100
#define ERR_END_Y	0x200


#define SCREEN_ON_TIME		120000	// ms

#define WIND_MEAS_MS		60000	// counting wind pulses
#define SAVE_AFTER_MOVE		2000	// when motor stop store position

#define STUCK_ON_END_TIME	5000	// when x seconds on end re-enable the remote

#define NR_OF_TARGETS		16		// used is 0..15 but user gets 1..16

#define DEFAULT_MAX			2000	// some value for maximum

#define NO_VALUE			0xBEEF //No value for deviation recorded

#define ONE_DAY				24 * 60 * 60 * 1000 // 24h in ms
//#define ONE_DAY				1 * 20 * 60 * 1000 // 24h in ms

#define DECODING_RDY		100
#define FULLPWM				100 // 0..100

#define APB_CLK 			48000000

#define HEADROOM			20 // decrees of moving space


#define DEBOUNCE_END		20 // ms endswitch debounce

#define HORIZONTAL			1
#define VERTICAL			2

#define						ENABLE_MODBUS
#define MOON_OFF			0xFF00

typedef enum
{
	ST_TRACK_SUN =0,
	ST_TRACK_MANUAL,	// Manual is mode
	ST_TRACK_TARGET_1,	// used for display target nr
	ST_TRACK_TARGET_2,	// used for display target nr
	ST_TRACK_TARGET_3,	// used for display target nr
	ST_TRACK_TARGET_4,	// used for display target nr
	ST_TRACK_TARGET_5,	// used for display target nr
	ST_TRACK_TARGET_6,	// used for display target nr
	ST_TRACK_TARGET_7,	// used for display target nr
	ST_TRACK_TARGET_8,	// used for display target nr
	ST_TRACK_TARGET_9,	// used for display target nr
	ST_TRACK_TARGET_10,	// used for display target nr
	ST_TRACK_TARGET_11,	// used for display target nr
	ST_TRACK_TARGET_12,	// used for display target nr
	ST_TRACK_TARGET_13,	// used for display target nr
	ST_TRACK_TARGET_14,	// used for display target nr
	ST_TRACK_TARGET_15,	// used for display target nr
	ST_TRACK_TARGET_16,	// used for display target nr

	ST_STOP,			// Up to stop are operating modes

	ST_INIT,
	ST_CALIBRATE,
	ST_ABOUT_TO_SAVE, 	//save has been pressed, ask: sure?
	ST_SAVED,
	ST_MOVE_REMOTE,			// move by remote control
	ST_WIND_STOP,
	ST_WAIT_GPS,
	ST_OFF,
	ST_HAL_TIMEOUT,
	ST_LOW_VCC,
	ST_OVERHEAT,
	ST_INVALID_PARAMETERS,
	ST_RECOVER,
	ST_END_TIMEOUT,


}main_mode_st;

typedef struct s_app_info
{
  uint32_t key;
  uint32_t version;
  uint32_t crc32;
  uint32_t size;
  uint8_t build_date[sizeof(__DATE__)];
  uint8_t build_time[sizeof(__TIME__)];
  uint8_t dummy[3];
} app_info_t;


typedef struct
{
	float latitude;
	float longitude;
}location_t;

typedef struct
{
	int32_t x;
	int32_t y;
}motorpos_t;

typedef struct
{
	int16_t repeat_ms;	// loop time
	int16_t i;			// when pwm < 100 increase with x every repeat_ms time
	int16_t p;			// proportional amplifier in milli, 1 = 1000
	int16_t d;			// not used
	int16_t softstart;	// total softstart time, timer = softstart /100
}pd_t;

typedef struct
{
	float azimuth;
	float elevation;
}coord_t;

typedef enum
{
	TIMED_OFF = 0,
	TIMED_ON,			// mode is on but not different for summer and winter
	TIMED_EU_SUMMER,	// European summer time
	TIMED_US_SUMMER		// us summer time

}pmode_t;

typedef struct
{
	motorpos_t pos;		// target positions in encoder values
	uint16_t mod;		// minute of day
	pmode_t mode;		// off,on,eu,us
}target_properties_t;

typedef struct Flash_variables
{
	// learning
	location_t home_location;	// position from GPS
	uint32_t stm_serial;		// crc32 STM32 base serial
	motorpos_t hw_offset;		// starting point of mirror in world, learned by calibrate sun
	uint32_t ayct102_home_1;	// home code of remote control
	uint32_t ayct102_home_2;	// home code of remote control

	uint32_t eeprom_serial;		// EEROM base serial not used now

	motorpos_t maximum;			// maximum encoder value, learned by max_x and max_y
	motorpos_t parkposition;	// park position in encoder values
	target_properties_t target[NR_OF_TARGETS];			// properties of the target

	// Manual parameters:
	motorpos_t steps;			// Steps per decreese
	motorpos_t hysteresis;		// hysteresis in position
	pd_t pid;					// PI motor controller
	uint16_t debounce;			// time (ms) between touch min and max sensor
	uint16_t sun_down_angle;	// operate above these angle
	uint32_t min_pwm;			// pwm of when the motor starts working
	uint32_t max_pwm;			// max allowed pwm
	uint32_t max_windpulse;		// Max windspeed
	uint32_t pwmfreq;			// Freq (Hz)
	uint8_t contrast;			// PWM of contrast for negative voltage
	uint16_t track_interval;	// when to calculate
	uint32_t turnback;			// decidec distance what will be rotated back on end

	uint32_t moonend_mod;			//
	uint32_t spare2;				//
	uint32_t spare3;				//
	uint32_t spare4;				//
	uint32_t spare5;				//
	uint32_t spare6;				//
	uint32_t spare7;				//
	uint32_t spare8;				//
	uint32_t spare9;				//
	uint32_t spareA;				//
	uint32_t spareB;				//

	uint32_t crc;
}hw_info_t;


// For compatibility:
typedef struct OLDFlash_variables
{
	// learning
	location_t home_location;	// position from GPS
	uint32_t stm_serial;		// crc32 STM32 base serial
	motorpos_t hw_offset;		// starting point of mirror in world, learned by calibrate sun
	uint32_t ayct102_home;		// home code of remote control
	uint32_t eeprom_serial;		// EEROM base serial not used now

	motorpos_t maximum;			// maximum encoder value, learned by max_x and max_y
	motorpos_t parkposition;	// park position in encoder values
	motorpos_t target[6];		// target positions in encoder values

	// Manual parameters:
	motorpos_t steps;			// Steps per decreese
	motorpos_t hysteresis;		// hysteresis in position
	pd_t pid;					// PI motor controller
	uint16_t debounce;			// time (ms) between touch min and max sensor
	uint16_t sun_down_angle;	// operate above these angle
	uint32_t min_pwm;			// pwm of when the motor starts working
	uint32_t max_pwm;			// max allowed pwm
	uint32_t max_windpulse;		// Max windspeed
	uint32_t spare;			//
	uint8_t contrast;			// PWM of contrast for negative voltage
	uint16_t track_interval;	// when to calculate

	uint32_t spare0;				//
	uint32_t spare1;				//
	uint32_t spare2;				//
	uint32_t spare3;				//
	uint32_t spare4;				//
	uint32_t spare5;				//
	uint32_t spare6;				//
	uint32_t spare7;				//
	uint32_t spare8;				//
	uint32_t spare9;				//
	uint32_t spareA;				//
	uint32_t spareB;				//

	uint32_t crc;
}old_hw_info_t;



/*!
 * @struct i2c_ee_t
 * @brief Struct data to be saved on EEROM
 */
typedef struct
{
	motorpos_t actual_motor; // actual position of the motor
	motorpos_t target;	// position of the target
	main_mode_st main_mode;

	uint32_t serial;				// Serial key
	// logging
	uint32_t tracking_minutes;		// minutes of tracking
	uint32_t bootcounter;			// number of boots
	uint16_t set_zero_x;			// times the zero limit has reached
	uint16_t set_zero_y;
	uint16_t error_hal_xa;			// times the error occurred
	uint16_t error_hal_xb;
	uint16_t error_hal_ya;
	uint16_t error_hal_yb;

	uint32_t crc;
}i2c_ee_t;

static const char * target_name[] =
{
		"TG_NONE",
		"PARKING",
		"SUN",
		"TG_1",
		"TG_2",
		"TG_3",
		"TG_4",
		"TG_5",
		"TG_SAVED",

};

typedef enum
{
	TG_NONE,
	TG_PARK,
	TG_SUN,
	TG_1,
	TG_2,
	TG_3,
	TG_4,
	TG_5,
	TG_SAVED,
}target_t;

typedef enum
{
	LCD_WELCOME,
	LCD_WAIT_GPS,
	LCD_STOP,
	LCD_CALIBRATE,
	LCD_FOLLOW_TARGET,
	LCD_FOLLOW_MANUAL,
	LCD_FOLLOW_SUN,
	LCD_MOVE_REMOTE,
	LCD_OFF,
	LCD_SUNDOWN,
	LCD_ABOUT_TO_SAVE,
	LCD_SAVED,
	LCD_LOW_VCC,

	LCD_ERROR,
	LCD_RECOVER,
	LCD_INVALID_PARAMETER,

}lcd_display_t;

typedef struct
{
	uint8_t stage;

	uint32_t a_rise;
	uint32_t a_fall;
	uint32_t b_rise;
	uint32_t b_fall;
	uint32_t b_rise_end;
	uint32_t frequency;
	uint32_t duty_a;
	uint32_t duty_b;
	uint32_t shift;


}phase_t;

typedef enum
{
	SYS_UNKNOWN,
	SYS_GLONASS,
	SYS_GPS,
}gps_e;

typedef struct
{
	lcd_display_t screen_lcd;
	motorpos_t goto_motor; // desired position
	motorpos_t deviation;		// difference real and calculated zero point
	target_t about_to_save;
	hw_info_t hwinfo;
	main_mode_st store_main_mode;
	coord_t sunpos;
	coord_t moonpos;
	i2c_ee_t eevar;
	uint32_t spare;	// overrun eerom function?

	uint32_t wind_ppm; 				// actual wind pulses per minute
	uint32_t lastrx_ayct102_home;
	uint32_t error_status;
	int8_t max_pwm;					// PWM 0..100

	uint8_t out_of_range;
	uint8_t parkmode;
	uint8_t screen_tmr;
	uint8_t tracking_tmr;
	uint8_t calc_sun_tmr;
	uint8_t gps_decode;		// 0..100% decoding state
	gps_e gps_system;

	bool gps_debug;
	phase_t phase_x;
	phase_t phase_y;

}vars_t;

typedef enum ram_position_lcd
{
	CHAR_NONE =0,
	CHAR_GPS,
	CHAR_BT,
}lcd_ram_t;

/*
 * Specialchar 0.6 b.Mueller
 */
static const uint8_t noneChar[] =
{	0x00,    // xxx 11111
	0x00,    // xxx 10001
	0x00,    // xxx 10001
	0x00,    // xxx 10001
	0x00,    // xxx 10001
	0x00,    // xxx 10001
	0x00,    // xxx 10001
	0x00,  	 // xxx 11111
};

static const uint8_t gpsChar[] =
{	0x0E,    //
	0x11,    //
	0x15,    //
	0x11,    //
	0x0E,    //
	0x04,    //
	0x04,    //
	0x00,  	 //
};

static const uint8_t btChar[] =
{	0x04,    //
	0x16,    //
	0x0D,    //
	0x06,    //
	0x0D,    //
	0x16,    //
	0x04,    //
	0x00,  	 //
};



void init_vars(void);
void factory(hw_info_t * hwinfo);

#endif /* VARS_H_ */
