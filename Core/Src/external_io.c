/*
 * external_io.c
 *
 *  Created on: 17 jan. 2019
 *      Author: VM
 */

#include "main.h"
#include "usart.h"
#include "time.h"
#include "vars.h"
#include "shell.h"
#include "external_io.h"

/*
 * Local variables
 */

static uint8_t wind_interval_tmr = NO_TIMER;



/*!
 * \brief This function
 *
 * \param -
 *
 * \return -.
 */
uint32_t get_wind_counter(void)
{
	return timer_read(wind_interval_tmr);
}

/*!
 * \brief This function
 *
 * \param -
 *
 * \return -.
 */
void external_io_functions(void)
{

	static bool old_wind;
	static uint32_t wind_pulse_cnt;

	if (wind_interval_tmr == NO_TIMER)
	{
		wind_interval_tmr = timer_get();
		timer_start(wind_interval_tmr, WIND_MEAS_MS, NULL);
	}

	// count wind on rising edge
	if (!old_wind && isEXT_IN_1)
		wind_pulse_cnt++;
	old_wind = isEXT_IN_1;

	// test wind counter
	if (timer_elapsed(wind_interval_tmr))
	{
		timer_start(wind_interval_tmr, WIND_MEAS_MS, NULL);

		vars.wind_ppm = wind_pulse_cnt;
		wind_pulse_cnt = 0;
	}
}

