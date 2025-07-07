/*
 * build_display.c
 *
 *  Created on: 10 sep. 2019
 *      Author: Mark
 */

#include "main.h"
#include "vars.h"
#include "time.h"
#include "build_display.h"
#include "hd44780.h"
#include "shell.h"
#include "rtc.h"
#include "protection.h"
#include "adc.h"
#include "machine.h"

static void header(void);


/*!
 * \brief This function
 *
 * \param
 *
 * \return -.
 */
void process_display(void)
{
	static uint8_t refresh_tmr = NO_TIMER;
	static lcd_display_t old_lcd = LCD_ERROR;
	static bool print_once = false;
	static bool init_on_backlight = false;
	char buf[LCD_LENGTH]; //32
	char value_x[16]; // 16
	char value_y[16]; // 16
	time_date_t dt;

	// Init display
	if (refresh_tmr == NO_TIMER)
	{
		refresh_tmr = timer_get();

	}

	if (old_lcd != st_lcd_screen)
	{
		print_once = false;
		HD44780_Clear();
	}

	if (!isDISPLAY_ON)
		init_on_backlight = false;


	// refresh display if backlight is on
	if ((old_lcd != st_lcd_screen || timer_elapsed(refresh_tmr)) && isDISPLAY_ON)
	{
		timer_start(refresh_tmr, 1000, NULL);

		if (!init_on_backlight)
		{
			init_on_backlight = true;
			dsp_init(); // because the LCD had some crap sometimes
		}

		switch (st_lcd_screen)
		{
		case LCD_WELCOME:
#ifdef ENABLE_MODBUS
			snprintf(buf, sizeof(buf), "Suntrack %X.%02X RTU", ((VERSION >> 24) & 0xFF), ((VERSION >> 16) & 0xFF));
#else
			snprintf(buf, sizeof(buf), "Suntrack %X.%02X", ((VERSION >> 24) & 0xFF), ((VERSION >> 16) & 0xFF));
#endif
			HD44780_Puts(0, 0, buf);

			snprintf(buf, sizeof(buf), "Serial %lu", stm_serial());
			HD44780_Puts(0, 1, buf);

			snprintf(buf, sizeof(buf), "Hours: %ld.%02ld", vars.eevar.tracking_minutes / 60, vars.eevar.tracking_minutes % 60);
			HD44780_Puts(0, 2, buf);

			if (vars.hwinfo.moonend_mod != FOLLOW_MOON_OFF)
			{
				snprintf(buf, sizeof(buf), "Lunar Hours: %ld.%02ld", vars.eevar.moon_minutes / 60, vars.eevar.moon_minutes % 60);
				HD44780_Puts(0, 3, buf);
			}

			break;

		case LCD_WAIT_GPS:

			//HD44780_PutCustom(18,0,0);

			snprintf(buf, sizeof(buf), "Wait for %s: %d%%", vars.gps_system == SYS_GPS ? "GPS" : "GNS", vars.gps_decode);
			HD44780_Puts(0, 0, buf);

			break;

		case LCD_INVALID_PARAMETER:
			header();

			if (out_learned_range(HORIZONTAL))
				snprintf(buf, sizeof(buf), "ErrCalHor %ld", vars.hwinfo.maximum.x);
			else if (out_learned_range(VERTICAL))
				snprintf(buf, sizeof(buf), "ErrCalVer %ld", vars.hwinfo.maximum.y);
			else
				snprintf(buf, sizeof(buf), "Please calibrate");
			HD44780_Puts(0, 3, buf);

			break;

		case LCD_STOP:
			header();

			if (out_learned_range(HORIZONTAL))
				snprintf(buf, sizeof(buf), "ErrCalHor %ld", vars.hwinfo.maximum.x);
			else if (out_learned_range(VERTICAL))
				snprintf(buf, sizeof(buf), "ErrCalVer %ld", vars.hwinfo.maximum.y);
			else
				snprintf(buf, sizeof(buf), "Mode: STOP");

			HD44780_Puts(0, 3, buf);
			break;

		case LCD_CALIBRATE:
			header();

			if (vars.deviation.x == NO_VALUE)
				snprintf(value_x, sizeof(value_x), "?");
			else
				snprintf(value_x, sizeof(value_x), "%ld", vars.deviation.x);

			if (vars.deviation.y == NO_VALUE)
				snprintf(value_y, sizeof(value_y), "?");
			else
				snprintf(value_y, sizeof(value_y), "%-16ld", vars.deviation.y);

			snprintf(buf, sizeof(buf), "Devitn %s %s", value_x, value_y);
			HD44780_Puts(0, 3, buf);
			break;

		case LCD_FOLLOW_TARGET:
			header();

			if (vars.out_of_range)
				snprintf(buf, sizeof(buf), "FT out of range    ");
			else if (out_learned_range(HORIZONTAL))
				snprintf(buf, sizeof(buf), "ErrCalHor %ld", vars.hwinfo.maximum.x);
			else if (out_learned_range(VERTICAL))
				snprintf(buf, sizeof(buf), "ErrCalVer %ld", vars.hwinfo.maximum.y);

			else if (st_main_prn == ST_WIND_STOP)
				snprintf(buf, sizeof(buf), "Mode: Windstop");
			else
				snprintf(buf, sizeof(buf), "Mode: Follow TG-%d", st_main_prn - 1);

			HD44780_Puts(0, 3, buf);
			break;

		case LCD_FOLLOW_SUN:
			header();

			if (vars.out_of_range)
				snprintf(buf, sizeof(buf), "FS out of range ");
			else if (out_learned_range(HORIZONTAL))
				snprintf(buf, sizeof(buf), "ErrCalHor %ld", vars.hwinfo.maximum.x);
			else if (out_learned_range(VERTICAL))
				snprintf(buf, sizeof(buf), "ErrCalVer %ld", vars.hwinfo.maximum.y);
			else
				snprintf(buf, sizeof(buf), "Mode: Follow SUN");
			HD44780_Puts(0, 3, buf);
			break;

		case LCD_FOLLOW_MANUAL:
			header();

			if (vars.out_of_range)
				snprintf(buf, sizeof(buf), "FM out of range    ");
			else if (out_learned_range(HORIZONTAL))
				snprintf(buf, sizeof(buf), "ErrCalHor %ld", vars.hwinfo.maximum.x);
			else if (out_learned_range(VERTICAL))
				snprintf(buf, sizeof(buf), "ErrCalVer %ld", vars.hwinfo.maximum.y);
			else
				snprintf(buf, sizeof(buf), "Mode: Follow MANUAL");
			HD44780_Puts(0, 3, buf);
			break;

		case LCD_MOVE_REMOTE:
			header();

			snprintf(buf, sizeof(buf), "Mode: RemoteCTRL");
			HD44780_Puts(0, 3, buf);
			break;

		case LCD_OFF:
			snprintf(buf, sizeof(buf), "System OFF");
			HD44780_Puts(0, 0, buf);

			break;

		case LCD_SUNDOWN:
			header();

			snprintf(buf, sizeof(buf), "Tracking: Sun Down");
			HD44780_Puts(0, 3, buf);

			break;

		case LCD_ERROR:
			// No more screen updates

			if (!print_once)
			{
				print_once = true;
				snprintf(buf, sizeof(buf), "Hardware error at:");
				HD44780_Puts(0, 0, buf);

				rtc_get(&dt);
				snprintf(buf, sizeof(buf), "%02d:%02d:%02d %02d-%02d-%02d", dt.Hours, dt.Minutes, dt.Seconds, dt.Date, dt.Month, dt.Year);
				HD44780_Puts(0, 1, buf);

				// END switch error or HAL timeout?
				if ((get_err & ERR_END_X) || (get_err & ERR_END_Y))
				{
					snprintf(buf, sizeof(buf), "ERROR");
					HD44780_Puts(0, 2, buf);
					snprintf(buf, sizeof(buf), "Stuck endswitch %s%s", get_err & ERR_END_X ? "X " : " ", get_err & ERR_END_Y ? "Y " : " ");
					HD44780_Puts(0, 3, buf);

				}
				else
				{
					snprintf(buf, sizeof(buf), "HAL %s%s%s%s", get_err & ERR_HAL_XA ? "XA " : "-", get_err & ERR_HAL_XB ? "XB " : "-", get_err & ERR_HAL_YA ? "YA " : "-", get_err & ERR_HAL_YB ? "YB " : "-");
					HD44780_Puts(0, 2, buf);

					snprintf(buf, sizeof(buf), "DRV %s%s%s%s", get_err & ERR_CLX ? "CLX " : "-", get_err & ERR_CLY ? "CLY " : "-", get_err & ERR_OTX ? "OTX " : "-", get_err & ERR_OTY ? "OTY" : "-");
					HD44780_Puts(0, 3, buf);
				}
			}
			break;

		case LCD_LOW_VCC:

			snprintf(buf, sizeof(buf), "Stopped Low Vcc");
			HD44780_Puts(0, 0, buf);

			snprintf(buf, sizeof(buf), "Vcc =% d.%dV\r\n", read_adapter() / 1000, (read_adapter() % 1000) / 10);
			HD44780_Puts(0, 2, buf);

			break;

		case LCD_ABOUT_TO_SAVE:
			snprintf(buf, sizeof(buf), "Press G-1 to save");
			HD44780_Puts(0, 0, buf);

			snprintf(buf, sizeof(buf), "Press G-0 to abort");
			HD44780_Puts(0, 1, buf);

			snprintf(buf, sizeof(buf), "target: %s", target_name[vars.about_to_save]);
			HD44780_Puts(0, 2, buf);

			break;

		case LCD_RECOVER:
			snprintf(buf, sizeof(buf), "* Recovery mode *");
			HD44780_Puts(0, 0, buf);

			snprintf(buf, sizeof(buf), "Use slider-3");
			HD44780_Puts(0, 1, buf);

			snprintf(buf, sizeof(buf), "Max-X/Y to move");
			HD44780_Puts(0, 2, buf);

			snprintf(buf, sizeof(buf), "Press Stop to exit");
			HD44780_Puts(0, 3, buf);
			break;


		case LCD_SAVED:

			snprintf(buf, sizeof(buf), "Saved");
			HD44780_Puts(0, 0, buf);

			snprintf(buf, sizeof(buf), "target: %s", target_name[vars.about_to_save]);
			HD44780_Puts(0, 1, buf);

			//vars.about_to_save = TG_NONE;
			break;

		}
	}
	old_lcd = st_lcd_screen;

}

/*!
 * \brief This function Sets first 3 lines
 *
 * \param -
 *
 * \return -.
 */
static void header(void)
{
	time_date_t dt;
	static uint8_t sec = 0;
	static bool blink;
	char buf[20];
	char value_x[20];
	char value_y[20];
//	float x, y;

	if (isGPS_ON)
	{
		if (blink ^= 1)
			HD44780_PutCustom(19,0,CHAR_GPS);
		else
			HD44780_PutCustom(19,0,CHAR_NONE);
	}
	else
		HD44780_PutCustom(19,0,CHAR_GPS);


	rtc_get(&dt);
	if (vars.gps_decode == DECODING_RDY)
		snprintf(buf, sizeof(buf), "%2d:%02d:%02d %2d-%02d-%02d", dt.Hours, dt.Minutes, dt.Seconds, dt.Date, dt.Month, dt.Year);
	else
		snprintf(buf, sizeof(buf), "Wait for GPS");

	HD44780_Puts(0, 0, buf);

	if (vars.gps_decode == DECODING_RDY)
	{
		if (vars.sunpos.elevation > mSUN_DOWN_ANGLE)
			snprintf(buf, sizeof(buf), "SUN %7.2f %-7.2f", vars.sunpos.azimuth, vars.sunpos.elevation);
		else
			snprintf(buf, sizeof(buf), "MOON %7.2f %-7.2f", vars.moonpos.azimuth, vars.moonpos.elevation);
	}
	else
		snprintf(buf, sizeof(buf), "SUN ?? ??");

	HD44780_Puts(0, 1, buf);

	if (isEND_X)
		snprintf(value_x, sizeof(value_x), "ENDX  ");
	else
		snprintf(value_x, sizeof(value_x), "%ld", vars.eevar.actual_motor.x);

	if (isEND_Y)
		snprintf(value_y, sizeof(value_y), "ENDY  ");
	else
		snprintf(value_y, sizeof(value_y), "%-16ld", vars.eevar.actual_motor.y);

	snprintf(buf, sizeof(buf), "Mirror %s %s", value_x, value_y);

	if (++sec == 5)
		sec = 0;

	HD44780_Puts(0, 2, buf);
}


