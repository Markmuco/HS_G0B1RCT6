/*
 * vars.c
 *
 *  Created on: 13 aug. 2019
 *      Author: Mark
 */

#include "main.h"
#include "vars.h"
#include "time.h"
#include "flash_ee.h"
#include "uart_sci.h"
#include "rtc.h"
#include "shell.h"
#include "e2p.h"
#include "tim.h"
#include "protection.h"
#include "adc.h"

/*!
 * \brief Set flash parameters to default
 *
 * \param poinyer to flashvars
 *
 * \return -.
 */
void factory(hw_info_t * hwinfo)
{
	uint32_t remember_stm_serial = hwinfo->stm_serial;

	// Clear STM32 Flash parameters
	memset(hwinfo, 0x00, sizeof(hw_info_t));

	// set back the serial
	hwinfo->stm_serial = remember_stm_serial;

	hwinfo->hysteresis.x = 10;
	hwinfo->hysteresis.y = 10;
	hwinfo->pid.repeat_ms = 5;
	hwinfo->pid.i = 40;		// ms
	hwinfo->pid.p = 600;	// 1000 = 1.000
	hwinfo->pid.d = 0;		// not used

	hwinfo->pid.softstart = 500; // ms

	hwinfo->debounce = 3000; // 3 seconds of ignore remote and end switch
	hwinfo->steps.x = 156;
	hwinfo->steps.y = 156;
	hwinfo->pwmfreq = 15000;
	hwinfo->turnback = 10;

	// parkposition should not be 0
	hwinfo->parkposition.x = 10 * hwinfo->steps.x;
	hwinfo->parkposition.y = 10 * hwinfo->steps.y;

	hwinfo->max_pwm = 50;
	hwinfo->min_pwm = 10;
	hwinfo->maximum.x = DEFAULT_MAX;
	hwinfo->maximum.y = DEFAULT_MAX;
	hwinfo->contrast = 30;
	hwinfo->track_interval = 10; // seconds
	hwinfo->max_windpulse = 0x00; // no maxwind
	hwinfo->sun_down_angle = 0;
	hwinfo->moonend_mod = FOLLOW_MOON_OFF;

	// copy to real location
	WriteStruct2Flash(hwinfo, sizeof(hw_info_t));
}

/*!
 * \brief Initialisation of variables
 *
 * \param pointer data
 *
 * \return -.
 */
void init_vars(void)
{
	vars.error_status = ERR_NONE;
	vars.out_of_range = false;
	vars.about_to_save = TG_NONE;

	vars.screen_tmr = NO_TIMER;
	vars.tracking_tmr = NO_TIMER;
	vars.calc_sun_tmr = NO_TIMER;
	vars.lastrx_ayct102_home = 0;
	vars.gps_debug = false;
	vars.gps_system = SYS_UNKNOWN;
	vars.wind_ppm = 0;
	vars.deviation.x = vars.deviation.y = 0;

	BRIDGE_DRIVE_DIS;
	LCD_BACK_ON;

	if (adc_calib() != HAL_OK)
		tty_printf(" Error ADC calibration\r\n");
#if 0
	if (!tle_reset_diagnosis())
		tty_printf(" Error TLE\r\n");

	tle_read_diagnosis(result, RESET_DIA);

	if (tle_read_diagnosis(result, READ_REV))
		tty_printf("TLE version 0x%02X 0x%02X\r\n", result[0], result[1]);
	else
		tty_printf(" Error TLE\r\n");
#endif
	// read vars.hwinfo
	if (ReadStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)) == ERR_CRC)
	{
		// Possibly the old version?
		old_hw_info_t old_hw_info;
		if (ReadStruct2Flash(&old_hw_info, sizeof(old_hw_info_t)) != ERR_CRC)
		{
			vars.hwinfo.stm_serial = old_hw_info.stm_serial;
			tty_printf("Found old serial\r\n");
		}

		tty_printf("  Error loading flash: restore defaults\r\n");
		factory(&vars.hwinfo);
	}


	/*
	 * testtest
	 */
	/** Load Clean, System Clean, PowerUp counters from I2C eerom */
	if (ReadStruct2eerom(&vars.eevar))
	{
		// CRC error: load defaults
		tty_printf("  Error reading eeprom\r\n");

		vars.eevar.actual_motor.x = 1000;
		vars.eevar.actual_motor.y = 1000;
		vars.eevar.bootcounter = 0;
		vars.eevar.tracking_minutes = 0;

		// will enter calibration mode because of this
		vars.hwinfo.maximum.x = DEFAULT_MAX;
		vars.hwinfo.maximum.y = DEFAULT_MAX;

		if (WriteStruct2eerom(vars.eevar))
			tty_printf("  Error write eeprom\r\n");

	}

	new_pwm_freq(vars.hwinfo.pwmfreq);

	// compatible with new parameter
	if (vars.hwinfo.turnback == 0)
		vars.hwinfo.turnback = 10;

	// number of boots
	vars.eevar.bootcounter++;
	vars.max_pwm = vars.hwinfo.max_pwm;

	// do not move
	vars.goto_motor = vars.eevar.actual_motor;
	set_contrast(vars.hwinfo.contrast);

	// store mode to wait for GPS mode
	vars.store_main_mode = vars.eevar.main_mode;
	vars.eevar.main_mode = ST_INIT;

}
