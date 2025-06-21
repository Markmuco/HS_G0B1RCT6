/*
s * ayct102.c
 *
 *  Created on: 2 okt. 2016 / 8-1-2018
 *      Author: Mark
 */

#include "main.h"
#include "ayct102.h"
#include "tim.h"

/**
 * AYCT-102 Keys:
 * 1= 4 - 0
 * 2= 5 - 1
 * 3= 6 - 2
 * 4= 7 - 3
 *
 * G 12 - 8
 *
 * Slider 0-1-2-3
 *
 *  	 _   _
 * '0':	| |_| |____	(T,T,T,3T)
 *  	 _      _
 *  '1':| |____| |_	(T,3T,T,T)
 *   	 _   _
 * dim:	| |_| |_	(T,T,T,T)
 *
 *  T = korte periode = 275 µs lange periode 1300µs (5T)
 *  lange periode = 3 of 4*T (werkt ook allebei)Het frame bestaat normaal uit 32 bits:startpuls (T hoog, 9*laag)
 *  26	adres
 *  1	groep-bit
 *  1	on/off/[dim]
 *  4	unit (indien meerdere kanalen op één zender)
 *  [4]	[dimniveau]stoppuls (T hoog, verder laag)
 *
 */


static uint8_t cnt;
static uint8_t bitcounter = 0;
static uint8_t x = 0, buf[5];
static uint32_t start_time = 0;
static uint32_t received_code;


/**
 * @brief This function decodes the AYCT102
 *
 * Return TRUE on RX
 * pointer to ayct data structure
 */
bool get_ayct(ayct102_t * ayct102)
{
	if (bitcounter == 32)
	{
	//	ayct102->received_code = received_code;
		ayct102->home = (received_code & AYCT_ADR) >> 8;
		ayct102->slide = (received_code & AYCT_SLIDE) >> 2;
		ayct102->key = ((received_code & AYCT_KEY) >> 2) | ((received_code & AYCT_KEY) & 3);
		bitcounter = 0;
		return true;
	}
	else
		return false;
}

/**
 * @brief This function handles EXTI line 2 and 3 interrupts.
 *
 * Configure as Rising and Falling IRQ
 */
void AYCT_EXTI_IRQHandler(void)
{
	static uint32_t timer_now;

	// IRQ will start before the timer is installed
	if (htim16.Instance == NULL)
		return;

	timer_now = __HAL_TIM_GET_COUNTER(&htim16);

	if (is433_INT)
	{
		if ((timer_now - start_time > BITS_MINIMUM) && (timer_now - start_time < BITS_MAXIMUM)) // short low
			buf[x++] = SHORT_LOW;
		else if ((timer_now - start_time > BITL_MINIMUM) && (timer_now - start_time < BITL_MAXIMUM)) // long low
			buf[x++] = LONG_LOW;
		else
		{
			cnt = x = 0; // bit is fout
			__HAL_TIM_SET_COUNTER(&htim16, 0);
		}

		if (x > 4 || cnt > 32)
			cnt = x = 0; // bit is fout

		// received 0
		if (buf[0] == SHORT_HIGH && buf[1] == SHORT_LOW && buf[2] == SHORT_HIGH && buf[3] == LONG_LOW)
		{
			bit_clear(received_code, (31 - cnt++));
			buf[0] = buf[1] = buf[2] = buf[3] = x = NO_DATA;
			__HAL_TIM_SET_COUNTER(&htim16, 0);
		}

		// received 1
		if (buf[0] == SHORT_HIGH && buf[1] == LONG_LOW && buf[2] == SHORT_HIGH && buf[3] == SHORT_LOW)
		{
			bit_set(received_code, (31 - cnt++));
			buf[0] = buf[1] = buf[2] = buf[3] = x = NO_DATA;
			__HAL_TIM_SET_COUNTER(&htim16, 0);
		}

		start_time = __HAL_TIM_GET_COUNTER(&htim16);
	}
	else
	{
		if ((timer_now - start_time > BITS_MINIMUM) && (timer_now - start_time < BITS_MAXIMUM)) // 1= tussen MIN en MAX
			buf[x++] = SHORT_HIGH;
		else
		{
			cnt = x = 0; // bit is fout
			__HAL_TIM_SET_COUNTER(&htim16, 0);
		}
		start_time = __HAL_TIM_GET_COUNTER(&htim16);
	}

}

/**
 * @brief This function handles TIMER handler
 *
 * Set prescaler to run timer on 1Mhz and 5000 counts (5us)
 *  When the timer overflow there was no more data received
 */
void AYCT_TIM_IRQHandler(void)
{
	// if timer IRQ test if end of code
	bitcounter = cnt;
	buf[0] = buf[1] = buf[2] = buf[3] = x = cnt = NO_DATA;
}

