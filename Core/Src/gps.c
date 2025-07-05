/*!
 * \file gps.c
 *
 * \brief This software module provides functions for the GPS module.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "vars.h"
#include "time.h"
#include "gps.h"
#include "gps_decode.h"
#include "rtc.h"
#include "shell.h"
#include "uart_sci.h"
#include "usart.h"
#include "gpio.h"

// Private variables
static bool baudrate_ok = false;
static uint8_t gps_valid_tmr = NO_TIMER;
static bool override_gps_sync = false;

// Private functions
static uint8_t hex_c(uint8_t c);
static bool gps_handle(gps_info_t * gps_info);
static uint8_t get_progress(gps_data_t * gps_data);


/*!
 * \brief This function handles the GPS data.
 *
 * \param - gps_info_t
 *
 * \return - precentage of sync 0..100, after sysnc on coldstart always 100
 */
uint8_t gps_process(location_t * location)
{
	static gps_info_t gps_info;
	static bool cold_start = true;
	time_date_t time_date;

	if (gps_handle(&gps_info))
	{
		if (gps_valid_tmr == NO_TIMER)
			gps_valid_tmr = timer_get();

		timer_start(gps_valid_tmr, ONE_DAY, NULL);
		location->latitude = gps_info.latitude;
		location->longitude = gps_info.longitude;
		time_date.Hours = gps_info.gps_time.hour;
		time_date.Minutes = gps_info.gps_time.minute;
		time_date.Seconds = gps_info.gps_time.second;
		time_date.Month = gps_info.gps_date.month;
		time_date.Date = gps_info.gps_date.day;
		time_date.Year = gps_info.gps_date.year;
		// Clear received data
		set_rtc(time_date);
		gps_power(false);

		MX_USART6_UART_DeInit();

		cold_start = false;
	}

	// cheats sync for testing
	if (override_gps_sync)
	{
		if (gps_valid_tmr == NO_TIMER)
			gps_valid_tmr = timer_get();

		timer_start(gps_valid_tmr, ONE_DAY, NULL);

		override_gps_sync = false;
		gps_power(false);
		override_gps_sync = false;
		MX_USART6_UART_DeInit();

		cold_start = false;
	}



	// Valid gps time elapsed new sync
	if (timer_read(gps_valid_tmr) == 1)
	{
		if (!isGPS_ON)
		{
			timer_free(&gps_valid_tmr);
			time_stamp();
			tty_printf("GPS receiver on\r\n");
			gps_power(true);
			HAL_Delay(20);
			MX_USART6_UART_Init();
		}
	}

	// In cold start the system cannot operate until first sync
	if (cold_start)
		return gps_info.progress;

	return DECODING_RDY;
}

/*!
 * \brief Time the GPS time is valid, countdown from 24h
 *
 * \param -
 *
 * \return - minutes
 */
uint32_t gps_remain_valid(void)
{
	return timer_read(gps_valid_tmr) / 1000 / 60;
}

void override_gps(void)
{
	override_gps_sync = true;
}

/*!
 * \brief This function handles the GPS data.
 *
 * \param - gps_info_t
 *
 * \return - true on new value
 */
static bool gps_handle(gps_info_t * gps_info)
{
	static gps_data_t gps_data;
	static uint8_t param = 0;
	static uint8_t offset = 0;
	static uint8_t parity = 0;
	static uint8_t checksum_param = 0;
	static gps_frame_t frame = GPS_FRAME_NONE;
	static char string[55 + 1];
	char c;

	if (gps_getch(&c))
	{
		//sci1_putc(c);
		if (c == '$')
		{
			param = 0;
			offset = 0;
			parity = 0;
			frame = GPS_FRAME_NONE;
		}
		else if ((c == ',') || (c == '*'))
		{
			string[offset] = '\0';

			if (param == 0)
			{
				if (!strcmp(string, "GPGGA"))
					frame = GPS_FRAME_GGA;
				else if (!strcmp(string, "GPRMC"))	// GP - GPRS
				{
					vars.gps_system = SYS_GPS;
					frame = GPS_FRAME_RMC;
				}
				else if (!strcmp(string, "GNRMC")) // GN = GLONASS
				{
					vars.gps_system = SYS_GLONASS;
					frame = GPS_FRAME_RMC;
				}
				else if (!strcmp(string, "GPTXT"))
					frame = GPS_FRAME_TXT;
				else
					frame = GPS_FRAME_NONE;
			}
			else if (frame == GPS_FRAME_TXT)
			{
				if (param == 4)
				{
					if (strncmp(string, "ROM", 3) == 0)
						tty_printf("GPS [%s]\r\n", string);
					if (strncmp(string, "IC=", 3) == 0)
						tty_printf("GNS [%s]\r\n", string);
					//strncpy(gps_data.version, string, sizeof(gps_data.version));
				}
			}
			else if (frame == GPS_FRAME_GGA)
			{
				if (param == 7)
					strcpy(gps_data.numsat, string);
			}
			else if (frame == GPS_FRAME_RMC)
			{
				baudrate_ok = true;
				if (param == 1)
				{
					string[6] = '\0'; // cut the millisecond
					strcpy(gps_data.time, string);
				}
				else if (param == 2)
				{
					strcpy(gps_data.fix, string);
				}

				else if (param == 3)
				{
					strcpy(gps_data.latitude, string);
				}
				else if (param == 4)
				{
					strcat(gps_data.latitude, string);

				}
				else if (param == 5)
				{
					strcpy(gps_data.longitude, string);
				}
				else if (param == 6)
				{
					strcat(gps_data.longitude, string);
				}

				else if (param == 9)
				{
					strcpy(gps_data.date, string);
				}
			}

			param++;
			offset = 0;

			if (c == '*')
				checksum_param = 1;
			else
				parity ^= c;
		}
		else if ((c == '\r') || (c == '\n'))
		{
			if (checksum_param)
			{
				// parity ok?
				if (((hex_c(string[0]) << 4) | hex_c(string[1])) == parity)
				{
					gps_info->progress = (get_progress(&gps_data));
					//tty_printf("Progress %d\r\n", gps_info->progress);

					if (gps_data.fix[0] == 'A') // A for valid V for no conclusion
					{
						if ((gps_data.latitude[0] != '\0') && (gps_data.longitude[0] != '\0'))
						{
							gps_info->latitude = geo_coord_to_deg(gps_data.latitude);
							gps_info->longitude = geo_coord_to_deg(gps_data.longitude);
							str_to_td(gps_data.date, (td_t*) &gps_info->gps_date);
							str_to_td(gps_data.time, (td_t*) &gps_info->gps_time);
							gps_info->fix = true;
							gps_info->numsat = atoi(gps_data.numsat);
							checksum_param = 0;
							// Clear received data
							memset(&gps_data, 0x00, sizeof(gps_data_t));
							return true;
						}
					}
				}
				checksum_param = 0;
			}
		}
		else
		{
			if (offset < sizeof(string))
				string[offset++] = c;
			if (!checksum_param)
				parity ^= c;
		}
	}
	return false;
}

/*!
 * \brief This function converts a hexadecimal character to its hexadecimal value.
 *
 * \param c The character to process (0..9, A..F, a..f).
 *
 * \return The hexadecimal value of the input character.
 */
static uint8_t hex_c(uint8_t c)
{
	c -= '0';
	if (c > 9)
		c -= 7;
	c &= 0x0F;

	return (c);
}

static uint8_t get_progress(gps_data_t * gps_data)
{
	uint8_t p = 0;
//	static bool startup_msg = false;

//	if (gps_data->version[0] != '\0' && !startup_msg)
//	{
//		startup_msg = true;
//		tty_printf("GPS: [%s]\r\n", gps_data->version[0]);
//	}
	if (baudrate_ok)
		p += 5;

	if (gps_data->time[0] != '\0')
		p += 20;

	if (gps_data->date[0] != '\0')
		p += 20;

	if (gps_data->latitude[0] != '\0')
		p += 15;

	if (gps_data->longitude[0] != '\0')
		p += 15;

	if (gps_data->fix[0] == 'A')
		p +=  25;

	return p;

}

