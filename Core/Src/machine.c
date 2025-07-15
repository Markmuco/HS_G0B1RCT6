/*
 * machine.c
 *
 *  Created on: 14 jan. 2019
 *      Author: Mark
 */

#include "main.h"
#include "vars.h"
#include "machine.h"
#include "rtc.h"
#include "suncalc.h"
#include "time.h"
#include "string.h"
#include "flash_ee.h"
#include "uart_sci.h"
#include "protection.h"
#include "adc.h"
#include "vector.h"

static void restore_target_pos( uint32_t mode);
static uint16_t wait_calc_newposiotion = false;

/*!
 * \brief Main function
 *
 * \param pointer to all variables
 *
 * \return -.
 */
void machine_process(void)
{
	static main_mode_st old_st_main_mode;
	static main_mode_st restore_main_mode;
	static uint8_t minute_counter_tmr = NO_TIMER;
	static uint8_t second_counter_tmr = NO_TIMER;
	static uint8_t welcome_tmr = NO_TIMER;
	static uint8_t do_parking_tmr = NO_TIMER;

	static uint8_t old_minute;
	static bool old_out_of_range = false;
	static bool sundown_parking = false;
	static bool stop_moon_by_mod = false;
	static bool parking_after_moontracking = false;

	time_date_t time_date;
	motorpos_t tg;

	if (vars.tracking_tmr == NO_TIMER)
	{
		vars.tracking_tmr = timer_get();
		timer_start(vars.tracking_tmr, 1, NULL);
	}

	// if checksum is ok make suntrack timer
	if (vars.calc_sun_tmr == NO_TIMER)
	{
		if (check_quick())
		{
			vars.calc_sun_tmr = timer_get();
			timer_start(vars.calc_sun_tmr, 1, NULL);
		}
		else
		{
			time_stamp();
			tty_printf("Suntrack error C\r\n");
			HAL_Delay(100);
		}
	}

	if (minute_counter_tmr == NO_TIMER)
	{
		minute_counter_tmr = timer_get();
		timer_start(minute_counter_tmr, MINUTE, NULL);
	}

	if (second_counter_tmr == NO_TIMER)
	{
		second_counter_tmr = timer_get();
		timer_start(second_counter_tmr, SECOND, NULL);
	}

	// windpulses overflow
	if ((vars.wind_ppm > vars.hwinfo.max_windpulse) && (mMAIN_MODE < ST_STOP) && vars.hwinfo.max_windpulse != 0)
	{
		tty_printf("Max wind parking\r\n");
		vars.goto_motor = vars.hwinfo.parkposition;
		mMAIN_MODE = ST_WIND_STOP;
	}

	// In tracking busy?
	if ((mMAIN_MODE < ST_STOP) && (!vars.out_of_range))
	{
		if (vars.sunpos.elevation > vars.hwinfo.sun_down_angle)
		{
			// Minute:
			if (timer_elapsed(minute_counter_tmr))
			{
				// add minute counter
				timer_start(minute_counter_tmr, MINUTE, NULL);
				vars.eevar.tracking_minutes++;
			}

			// Seconde, read hardware
			if (timer_elapsed(second_counter_tmr))
			{
				timer_start(second_counter_tmr, SECOND, NULL);
				if ((read_adapter() < LOW_VOLTAGE) && (mMAIN_MODE != ST_LOW_VCC))
				{
					restore_main_mode = mMAIN_MODE;
					mMAIN_MODE = ST_LOW_VCC;
					time_stamp();
					tty_printf("Voltage %d.%d nok\r\n", read_adapter() / 1000, (read_adapter() % 1000) / 100);
				}
			}
		}

		// In tracking mode and Moon count the minutes, check voltage
		if ((vars.moonpos.elevation > vars.hwinfo.sun_down_angle) && vars.hwinfo.moonend_mod != FOLLOW_MOON_OFF && !stop_moon_by_mod)
		{
			// Minute:
			if (timer_elapsed(minute_counter_tmr))
			{
				// add minute counter
				timer_start(minute_counter_tmr, MINUTE, NULL);
				vars.eevar.moon_minutes++;
			}
		}
	}

	// was Factory settings loaded?
	if ((vars.hwinfo.maximum.x == DEFAULT_MAX || vars.hwinfo.maximum.y == DEFAULT_MAX || vars.hwinfo.hw_offset.x == 0 || vars.hwinfo.hw_offset.x == 0) && (mMAIN_MODE <= ST_STOP))
	{
		mMAIN_MODE = ST_INVALID_PARAMETERS;
	}

	rtc_get(&time_date);

	// Calculation of sun position
	if (timer_elapsed(vars.calc_sun_tmr))
	{
		timer_start(vars.calc_sun_tmr, mTRACK_INTERVAL_S, NULL);
		suncalc(vars.hwinfo.home_location, time_date, &vars.sunpos, &vars.moonpos);

		if ((vars.gps_decode == DECODING_RDY) && (mMAIN_MODE != ST_STOP))
		{
			if ((!old_out_of_range && vars.out_of_range) || (vars.sunpos.elevation < mSUN_DOWN_ANGLE && !sundown_parking))
			{
				sundown_parking = true;

				time_stamp();
				tty_printf("%s Parking in %d seconds\r\n", vars.sunpos.elevation < mSUN_DOWN_ANGLE ? "Sunset" : "Out of range", OUTOF_RANGE_PARK_T / 1000);

				if (do_parking_tmr == NO_TIMER)
					do_parking_tmr = timer_get();

				timer_start(do_parking_tmr, OUTOF_RANGE_PARK_T, NULL);
			}

			if (vars.moonpos.elevation < mSUN_DOWN_ANGLE && vars.hwinfo.moonend_mod != FOLLOW_MOON_OFF && parking_after_moontracking)
			{
				parking_after_moontracking = false;

				time_stamp();
				tty_printf("Moonset parking in %d seconds\r\n", OUTOF_RANGE_PARK_T / 1000);

				if (do_parking_tmr == NO_TIMER)
					do_parking_tmr = timer_get();

				timer_start(do_parking_tmr, OUTOF_RANGE_PARK_T, NULL);
			}

			old_out_of_range = vars.out_of_range;
		}
	}

	// restoring memory status, out of range parking and sundown parking
	if ((!vars.out_of_range) && (vars.sunpos.elevation > mSUN_DOWN_ANGLE))
	{
		timer_free(&do_parking_tmr);
		sundown_parking = false;
		parking_after_moontracking = false;
		stop_moon_by_mod = false;
	}

	// The actual parking function
	if (timer_elapsed(do_parking_tmr))
	{
		timer_free(&do_parking_tmr);
		time_stamp();
		tty_printf("Do Parking\r\n");
		vars.goto_motor = vars.hwinfo.parkposition;
	}

	// at change of minute check if new target has to be set
	if (old_minute != time_date.Minutes && vars.eevar.main_mode >= ST_TRACK_MANUAL && vars.eevar.main_mode <= ST_TRACK_TARGET_16)
	{
		old_minute = time_date.Minutes;
		for (int var = 0; var < 16; ++var)
		{
			if (vars.hwinfo.target[var].mode != TIMED_OFF)
			{
				//			if (vars.hwinfo.target[var].mod == time_date.mod)
				if (get_dst_correction(vars.hwinfo.target[var].mod, vars.hwinfo.target[var].mode) == time_date.mod)
				{
					time_stamp();
					tty_printf("New Target %d\r\n", var + 1);
					vars.eevar.target = vars.hwinfo.target[var].pos;
					vars.store_main_mode = vars.eevar.main_mode = var + 2; // state machine
				}
			}
		}
	}

	if (mMAIN_MODE != old_st_main_mode)
	{
		old_st_main_mode = mMAIN_MODE;
		time_stamp();
		tty_printf("state [%s]\r\n", print_mode_name(vars.eevar.main_mode));
	}

	/*
	 * ************ Main Machine ************
	 */
	switch (mMAIN_MODE)
	{
	case ST_INIT:
		show_screen = LCD_WELCOME;

		if (welcome_tmr == NO_TIMER)
		{
			welcome_tmr = timer_get();
			timer_start(welcome_tmr, 5000, NULL);
		}
		if (timer_elapsed(welcome_tmr))
		{
			timer_free(&welcome_tmr);
			mMAIN_MODE = ST_WAIT_GPS;
		}

		break;

	case ST_STOP:
		show_screen = LCD_STOP;
		// Clear errors
		vars.error_status = ERR_NONE;
		// not auto continue on GS sync
		vars.store_main_mode = ST_STOP;
		// screen: STOP
		break;

	case ST_CALIBRATE:
		show_screen = LCD_CALIBRATE;
		// Clear errors
		vars.error_status = ERR_NONE;
		// not auto continue on GS sync
		vars.store_main_mode = ST_STOP;
		// screen: STOP
		break;

	case ST_OFF:
		show_screen = LCD_OFF;
// screen: OFF
		LCD_BACK_OFF;
		break;

	case ST_WAIT_GPS:
		show_screen = LCD_WAIT_GPS;
		// screen: Wait for GPS
		// sync of GSP will leave this mode or remote control
		break;

	case ST_WIND_STOP:

// screen: Stopped by wind
		break;

	case ST_ABOUT_TO_SAVE:
		show_screen = LCD_ABOUT_TO_SAVE;
		break;

	case ST_SAVED:
		show_screen = LCD_SAVED;
		break;

	case ST_TRACK_SUN:
		// only decoding is valid GPS
		if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
			show_screen = LCD_FOLLOW_SUN;
		else
			show_screen = LCD_SUNDOWN;

		if (vars.gps_decode == DECODING_RDY)
		{
			// screen: Track sun
			if (timer_elapsed(vars.tracking_tmr))
			{
				timer_start(vars.tracking_tmr, mTRACK_INTERVAL_S, NULL);
				if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
				{
					time_stamp();
					tty_printf("%s\r\n", print_mode_name(vars.eevar.main_mode));
					vars.out_of_range = follow_sun();
					vars.max_pwm = vars.hwinfo.max_pwm;
				}
				else
					tty_printf("Track sun, sundown\r\n");
			}
		}
		else
			mMAIN_MODE = ST_WAIT_GPS;

		break;

	case ST_TRACK_MANUAL:
	case ST_TRACK_TARGET_1:
	case ST_TRACK_TARGET_2:
	case ST_TRACK_TARGET_3:
	case ST_TRACK_TARGET_4:
	case ST_TRACK_TARGET_5:
	case ST_TRACK_TARGET_6:
	case ST_TRACK_TARGET_7:
	case ST_TRACK_TARGET_8:
	case ST_TRACK_TARGET_9:
	case ST_TRACK_TARGET_10:
	case ST_TRACK_TARGET_11:
	case ST_TRACK_TARGET_12:
	case ST_TRACK_TARGET_13:
	case ST_TRACK_TARGET_14:
	case ST_TRACK_TARGET_15:
	case ST_TRACK_TARGET_16:

		if ((vars.sunpos.elevation > mSUN_DOWN_ANGLE) || ((vars.hwinfo.moonend_mod != FOLLOW_MOON_OFF) && vars.moonpos.elevation > mSUN_DOWN_ANGLE))
			show_screen = (mMAIN_MODE == ST_TRACK_MANUAL) ? LCD_FOLLOW_MANUAL : LCD_FOLLOW_TARGET;
		else
			show_screen = LCD_SUNDOWN;

// only decoding is valid GPS
		if (vars.gps_decode == DECODING_RDY)
		{
			// screen: Track target
			if (timer_elapsed(vars.tracking_tmr))
			{
				timer_start(vars.tracking_tmr, mTRACK_INTERVAL_S, NULL);
				// Sun set?
				if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
				{
					time_stamp();
					tty_printf("Sun %s\r\n", print_mode_name(vars.eevar.main_mode));
					// set target position 1..5 for sun
					restore_target_pos(SUN);
					vars.out_of_range = follow_target(vars.sunpos);
					vars.max_pwm = vars.hwinfo.max_pwm;

				}
				else
				{
					if (vars.moonpos.elevation > mSUN_DOWN_ANGLE && vars.hwinfo.moonend_mod != FOLLOW_MOON_OFF && !stop_moon_by_mod)
					{
						time_stamp();
						tty_printf("Moon %s\r\n", print_mode_name(vars.eevar.main_mode));

						// set target position 6..10 for moon
						restore_target_pos(MOON);
						vars.out_of_range = follow_target(vars.moonpos);
						vars.max_pwm = vars.hwinfo.max_pwm;
						parking_after_moontracking = true;

						if (time_date.mod == vars.hwinfo.moonend_mod)
						{
							tty_printf("Moon stoptime %d:%02d\r\n", vars.hwinfo.moonend_mod / 60, vars.hwinfo.moonend_mod % 60);
							stop_moon_by_mod = true;
							vars.out_of_range = true;
						}
					}
					else
						tty_printf("Track target, down\r\n");
				}
			}
		}
		else
			mMAIN_MODE = ST_WAIT_GPS;

		break;

	case ST_RECOVER:
		show_screen = LCD_RECOVER;

		break;

	case ST_MOVE_REMOTE:
		show_screen = LCD_MOVE_REMOTE;

		// only decoding is valid GPS
		if (vars.gps_decode == DECODING_RDY)
		{
			if (isBRIGE_OFF && --wait_calc_newposiotion == 0)
			{
				// auto follow this target
				if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
					calc_target_pos(&vars.hwinfo, vars.sunpos, vars.eevar.actual_motor, &tg);
				else
					calc_target_pos(&vars.hwinfo, vars.moonpos, vars.eevar.actual_motor, &tg);

				vars.eevar.target = tg;
				mMAIN_MODE = ST_TRACK_MANUAL;
				time_stamp();
				tty_printf("Use new manual target\r\n");
			}
		}
		else
			mMAIN_MODE = ST_WAIT_GPS;
		break;

	case ST_HAL_TIMEOUT:
	case ST_END_TIMEOUT:

		LCD_BACK_ON;
		show_screen = LCD_ERROR;
		// stop
		vars.goto_motor = vars.eevar.actual_motor;

		break;

	case ST_LOW_VCC:
		show_screen = LCD_LOW_VCC;

		// stop
		vars.goto_motor = vars.eevar.actual_motor;

		if (timer_elapsed(second_counter_tmr))
		{
			timer_start(second_counter_tmr, SECOND, NULL);
			if (read_adapter() > RESTORE_VOLTAGE)
			{
				mMAIN_MODE = restore_main_mode;
				tty_printf("Voltage %d.%d ok\r\n", read_adapter() / 1000, (read_adapter() % 1000) / 100);
			}
		}
		break;

	case ST_OVERHEAT:
		LCD_BACK_ON;
		show_screen = LCD_ERROR;

		// stop
		vars.goto_motor = vars.eevar.actual_motor;

		if (timer_elapsed(second_counter_tmr))
		{
			timer_start(second_counter_tmr, SECOND, NULL);
#if 0
			if (tle_read_diagnosis(result, READ_DIA))
			{
				tty_printf("READ_DIA 0x%02X 0x%02X\r\n", result[0], result[1]);

				if (result[0] & NO_CURRENT_LIMIT)
					vars.error_status &= ~ERR_CLY;
				else
					vars.error_status |= ERR_CLY;

				if (result[1] & NO_CURRENT_LIMIT)
					vars.error_status &= ~ERR_CLX;
				else
					vars.error_status |= ERR_CLX;

				if (result[0] & NO_OVERTEMPERATURE)
					vars.error_status &= ~ERR_OTY;
				else
					vars.error_status |= ERR_OTY;

				if (result[1] & NO_OVERTEMPERATURE)
					vars.error_status &= ~ERR_OTX;
				else
					vars.error_status |= ERR_OTX;


				//if ((result[0] & NO_OVERTEMPERATURE) && (result[1] & NO_OVERTEMPERATURE))
				if (!(vars.error_status & (ERR_CLY | ERR_CLX | ERR_OTY | ERR_OTX)))
				{
					st_main_mode = restore_main_mode;
					time_stamp();
					tty_printf("Overheat restore\r\n");
				}
				if (tle_read_diagnosis(result, READ_CTRL))
					tty_printf("READ_CTRL 0x%02X 0x%02X\r\n", result[0], result[1]);
				else
					tty_printf(" Error TLE\r\n");
			}
			else
				tty_printf(" Error TLE\r\n");
#endif
		}
		break;

	case ST_INVALID_PARAMETERS:

		// The defaults seems to have loaded
		show_screen = LCD_INVALID_PARAMETER;

		break;

//	case ST_PRESS_REMOTE:
//		// screen: press remote
//		if (vars.lastrx_ayct102_home != 0)
//		{
//			vars.hwinfo.ayct102_home = vars.lastrx_ayct102_home;
//			st_main_mode = ST_INIT;
//			// TODO
//			// Save flash
//		}

		break;

	}

}

/*
 *	restore_target_pos
 *		&vars.eevar.target
 *		st_main_mode
 *		SUN / MOON
 */

static void restore_target_pos(uint32_t mode)
{
	uint16_t mem_pos = mMAIN_MODE - 2;


	switch (mMAIN_MODE)
	{
	case ST_TRACK_TARGET_1: // 2
	case ST_TRACK_TARGET_2:
	case ST_TRACK_TARGET_3:
	case ST_TRACK_TARGET_4:
	case ST_TRACK_TARGET_5:
		// We are in SUN mode switch to MOON?
		if (mode == MOON)
		{
			// values available in MOON mode?
			if (vars.hwinfo.target[mem_pos + 5].pos.x != 0)
			{
				mMAIN_MODE +=5;
				mem_pos += 5;
				vars.eevar.target = vars.hwinfo.target[mem_pos].pos;
				tty_printf("switch to Moon mode target %d\r\n", mem_pos);
			}
		}

		break;

	case ST_TRACK_TARGET_6:
	case ST_TRACK_TARGET_7:
	case ST_TRACK_TARGET_8:
	case ST_TRACK_TARGET_9:
	case ST_TRACK_TARGET_10:
		// We are in MON mode switch to SUN?
		if (mode == SUN)
		{
			// values available in MOON mode?
			if (vars.hwinfo.target[mem_pos - 5].pos.x != 0)
			{
				mMAIN_MODE -=5;
				mem_pos -= 5;
				vars.eevar.target = vars.hwinfo.target[mem_pos].pos;
				tty_printf("switch to Sun mode target %d\r\n", mem_pos);
			}
		}

		break;

	default:
		break;
	}
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_set_end_cw(void)
{
	time_stamp();
	tty_printf("End CW\r\n");
	vars.goto_motor.x = vars.hwinfo.steps.x * 360;
	if (vars.eevar.main_mode != ST_RECOVER)
		vars.eevar.main_mode = ST_CALIBRATE;
	vars.deviation.x = NO_VALUE;
	vars.max_pwm = FULLPWM;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_set_end_ccw(void)
{
	time_stamp();
	tty_printf("End CCW\r\n");
	vars.goto_motor.x = -vars.hwinfo.steps.x * 360;
	if (vars.eevar.main_mode != ST_RECOVER)
		vars.eevar.main_mode = ST_CALIBRATE;
	vars.deviation.x = NO_VALUE;
	vars.max_pwm = FULLPWM;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_set_end_up(void)
{
	time_stamp();
	tty_printf("End Up\r\n");
	vars.goto_motor.y = vars.hwinfo.steps.y * 360;
	if (vars.eevar.main_mode != ST_RECOVER)
		vars.eevar.main_mode = ST_CALIBRATE;
	vars.deviation.y = NO_VALUE;
	vars.max_pwm = FULLPWM;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_set_end_down(void)
{
	time_stamp();
	tty_printf("End Down\r\n");
	vars.goto_motor.y = -vars.hwinfo.steps.y * 360;
	if (vars.eevar.main_mode != ST_RECOVER)
		vars.eevar.main_mode = ST_CALIBRATE;
	vars.deviation.y = NO_VALUE;
	vars.max_pwm = FULLPWM;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_move_cw(uint32_t steps)
{
	timer_free(&vars.calc_sun_tmr);
	// delay so new position is not calculated immediate
	wait_calc_newposiotion = 1000;

	time_stamp();
	tty_printf("Clockwise %d\r\n", steps);
	if (vars.eevar.actual_motor.x < vars.hwinfo.maximum.x - vars.hwinfo.steps.x)
		vars.goto_motor.x += steps;
	vars.eevar.main_mode = ST_MOVE_REMOTE;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_move_ccw(uint32_t steps)
{
	timer_free(&vars.calc_sun_tmr);
	// delay so new position is not calculated immediate
	wait_calc_newposiotion = 1000;

//	timer_free(&vars.tracking_tmr);
//	timer_free(&vars.calc_sun_tmr);

	time_stamp();
	tty_printf("Counter Clockwise %d\r\n", steps);
	if (vars.eevar.actual_motor.x > vars.hwinfo.steps.x)
		vars.goto_motor.x -= steps;
	vars.eevar.main_mode = ST_MOVE_REMOTE;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_move_up(uint32_t steps)
{
	timer_free(&vars.calc_sun_tmr);
	// delay so new position is not calculated immediate
	wait_calc_newposiotion = 1000;

	time_stamp();
	tty_printf("Up %d\r\n", steps);
	if (vars.eevar.actual_motor.y < vars.hwinfo.maximum.y - vars.hwinfo.steps.y)
		vars.goto_motor.y += steps;
	vars.eevar.main_mode = ST_MOVE_REMOTE;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_move_down(uint32_t steps)
{
	timer_free(&vars.calc_sun_tmr);
	// delay so new position is not calculated immediate
	wait_calc_newposiotion = 1000;

	time_stamp();
	tty_printf("Down %d\r\n", steps);
	if (vars.eevar.actual_motor.y > vars.hwinfo.steps.y)
		vars.goto_motor.y -= steps;
	vars.eevar.main_mode = ST_MOVE_REMOTE;
}

/*!
 * \brief This function Calibrate ccw
 *
 * \param object A pointer to globals
 *
 * \return -.
 */
void cmd_savesun(void)
{
	if ((location_latitude < 0) && (sun_azimuth < 180))
		hw_offset_x = ((360 + sun_azimuth) * stepX) - mirror_azimuth; //units: step
	else
		hw_offset_x = (sun_azimuth * stepX) - mirror_azimuth; //units: step

	hw_offset_y = (sun_elevation * stepY) - mirror_elevation; //units: step

	time_stamp();
	tty_printf("Save Sun %d %d\r\n", hw_offset_x, hw_offset_y);

	vars.about_to_save = TG_SAVED;
	vars.eevar.main_mode = ST_SAVED;
	if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
		tty_printf("Error saving flash");
}

/*!
 * \brief This function Saves a target
 *
 * \param target 1..5
 *
 * \return -.
 */
void cmd_savetarget(uint8_t target)
{
	motorpos_t tg;

	if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
	{
		calc_target_pos(&vars.hwinfo, vars.sunpos, vars.eevar.actual_motor, &tg);
		vars.hwinfo.target[target - 1].pos = tg;
		tty_printf("Sun save target %d, %d %d\r\n", target, tg.x, tg.y);
	}
	else if (vars.moonpos.elevation > mSUN_DOWN_ANGLE)
	{
		calc_target_pos(&vars.hwinfo, vars.moonpos, vars.eevar.actual_motor, &tg);
		vars.hwinfo.target[target - 1 + 5].pos = tg;
		tty_printf("Moon save target %d, %d %d\r\n", target, tg.x, tg.y);
	}

	vars.about_to_save = TG_SAVED;
	vars.eevar.main_mode = ST_SAVED;
	if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
		tty_printf("Error saving flash");
}

/*!
 * \brief This function Saves parkposition
 *
 * \param -
 *
 * \return -.
 */
void cmd_savepark(void)
{
	time_stamp();
	tty_printf("Save Parking %d %d\r\n", vars.eevar.actual_motor.x, vars.eevar.actual_motor.y);
	vars.hwinfo.parkposition = vars.eevar.actual_motor;
	vars.about_to_save = TG_SAVED;
	if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
		tty_printf("Error saving flash");
}

char* print_mode_name(main_mode_st mode)
{
	switch (mode)
	{
	case ST_TRACK_SUN:
		return "TRACK_SUN";
	case ST_TRACK_MANUAL:
		return "TRACK_MANUAL";
	case ST_TRACK_TARGET_1:
		return "TRACK_T1";
	case ST_TRACK_TARGET_2:
		return "TRACK_T2";
	case ST_TRACK_TARGET_3:
		return "TRACK_T3";
	case ST_TRACK_TARGET_4:
		return "TRACK_T4";
	case ST_TRACK_TARGET_5:
		return "TRACK_T5";
	case ST_TRACK_TARGET_6:
		return "TRACK_T6";
	case ST_TRACK_TARGET_7:
		return "TRACK_T7";
	case ST_TRACK_TARGET_8:
		return "TRACK_T8";
	case ST_TRACK_TARGET_9:
		return "TRACK_T9";
	case ST_TRACK_TARGET_10:
		return "TRACK_T10";
	case ST_TRACK_TARGET_11:
		return "TRACK_T11";
	case ST_TRACK_TARGET_12:
		return "TRACK_T12";
	case ST_TRACK_TARGET_13:
		return "TRACK_13";
	case ST_TRACK_TARGET_14:
		return "TRACK_14";
	case ST_TRACK_TARGET_15:
		return "TRACK_15";
	case ST_TRACK_TARGET_16:
		return "TRACK_16";

	case ST_STOP:
		return "STOP";
	case ST_INIT:
		return "INIT";
	case ST_CALIBRATE:
		return "CALIBRATE";
	case ST_ABOUT_TO_SAVE:
		return "ABOUT_TO_SAVE";
	case ST_SAVED:
		return "SAVED";
	case ST_MOVE_REMOTE:
		return "MOVE_REMOTE";
	case ST_WIND_STOP:
		return "WIND_STOP";
	case ST_WAIT_GPS:
		return "WAIT_GPS";
	case ST_OFF:
		return "OFF";
	case ST_HAL_TIMEOUT:
		return "HAL_TIMEOUT";
	case ST_LOW_VCC:
		return "LOW_VCC";
	case ST_OVERHEAT:
		return "OVERHEAT";
	case ST_INVALID_PARAMETERS:
		return "INVALID_PARAMETERS";
	case ST_RECOVER:
		return "RECOVER";
	case ST_END_TIMEOUT:
		return "END_TIMEOUT";
	default:
		return "Unknown State";
	}
}

/*
 *	 Horizontal or vertical value is higher than learned value.
 *
 */
bool out_learned_range(uint8_t axle)
{
	switch (axle)
	{
	case HORIZONTAL:
		if ((vars.goto_motor.x > (int32_t) vars.hwinfo.maximum.x) || ((int32_t) vars.goto_motor.x < (((int32_t) vars.hwinfo.steps.x * (int32_t) vars.hwinfo.turnback) / 11)))
			return true;
		break;

	case VERTICAL:
		if ((vars.goto_motor.y > (int32_t) vars.hwinfo.maximum.y) || ((int32_t) vars.goto_motor.y < (((int32_t) vars.hwinfo.steps.y * (int32_t) vars.hwinfo.turnback) / 11)))
			return true;
		break;

	default:
		break;
	}
	return false;
}

