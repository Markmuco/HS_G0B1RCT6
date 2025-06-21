/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include <stdbool.h>
#include <stdint.h>
#include "uart_sci.h"
#include "shell.h"

static const uint8_t table[2][12] =
{
{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, // Non-leap year
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }  // Leap year
};

// Private function prototypes
static bool leap_year(uint16_t year);
//static bool IsDst(uint8_t day, uint8_t month, uint8_t dow);
static bool IsDst_EU(int8_t day, int8_t month, int8_t dow);
static bool IsDst_US(int8_t day, int8_t month, int8_t dow);
static uint16_t year_size(uint16_t year);
static uint32_t datetime_to_epoch(time_date_t *dt);

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/*!
 * \brief This function sets the RTC
 *
 * \param dt.Epoch is input
 * \param epoch The epoch time to convert.
 *
 * \return -.
 */
void set_rtc(time_date_t dt)
{
	static RTC_DateTypeDef datestructure;
	static RTC_TimeTypeDef timestructure;
	//uint32_t day_number;


	tty_printf("SET %02d-%02d-20%02d ", dt.Date, dt.Month, dt.Year);
	tty_printf("%02d:%02d:%02d\r\n", dt.Hours, dt.Minutes, dt.Seconds);

	//datestructure.WeekDay = dayofweek(dt.Date, dt.Month, dt.Year);
	datestructure.Month = dt.Month;
	datestructure.Date = dt.Date;
	datestructure.Year = dt.Year;

	timestructure.Hours = dt.Hours;
	timestructure.Minutes = dt.Minutes;
	timestructure.Seconds = dt.Seconds;
	timestructure.SecondFraction = 0;
	timestructure.SubSeconds = 0;
	timestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestructure.StoreOperation = RTC_STOREOPERATION_RESET;
	timestructure.TimeFormat = RTC_HOURFORMAT_24;

	HAL_RTC_SetDate(&hrtc, &datestructure, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc, &timestructure, RTC_FORMAT_BIN);

}

void rtc_get(time_date_t *dt)
{
	RTC_DateTypeDef datestructure;
	RTC_TimeTypeDef timestructure;

	HAL_RTC_GetTime(&hrtc, &timestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &datestructure, RTC_FORMAT_BIN);

	dt->Date = datestructure.Date;
	dt->Month = datestructure.Month;
	dt->Date = datestructure.Date;
	dt->WeekDay = datestructure.WeekDay;
	dt->Year = datestructure.Year;

	dt->Hours = timestructure.Hours;
	dt->Minutes = timestructure.Minutes;
	dt->Seconds = timestructure.Seconds;
	dt->mod = (60 * timestructure.Hours) + timestructure.Minutes;
	dt->DayLightSaving = IsDst_EU(datestructure.Date, datestructure.Month, datestructure.WeekDay);
}

/*!
 * \brief This function converts an epoch time to a datetime structure.
 *
 * \param dt A pointer to a datetime structure.
 * \param epoch The epoch time to convert.
 *
 * \return -.
 */
void epoch_to_datetime(time_date_t *dt)
{
	uint32_t day_clock;
	uint32_t day_number;
	uint16_t year = EPOCH_YEAR;

	day_clock = dt->Epoch % SECS_PER_DAY;
	day_number = dt->Epoch / SECS_PER_DAY;

	dt->Seconds = day_clock % SECS_PER_MINUTE;
	dt->Minutes = (day_clock % SECS_PER_HOUR) / SECS_PER_MINUTE;
	dt->Hours = day_clock / SECS_PER_HOUR;
	dt->Date = (day_number + 4) % 7;

	while (day_number >= year_size(year))
	{
		day_number -= year_size(year);
		year++;
	}

	dt->Year = year - 2000; // - YEAR_0;
	dt->Month = 0;

	while (day_number >= table[leap_year(year)][dt->Month])
	{
		day_number -= table[leap_year(year)][dt->Month];
		dt->Month++;
	}

	dt->Date = day_number;
	dt->Month++;
	dt->Date++;
}

/*!
 * \brief This function converts a datetime structure to an epoch time.
 *
 * \param p_epoch A pointer to the resulting epoch time.
 * \param dt A pointer to a datetime structure.
 *
 * \return -.
 */

static uint32_t datetime_to_epoch(time_date_t *dt)
{
	uint32_t epoch = 0;
	uint16_t month;
	uint16_t Date;
	uint16_t year;

//    printf("F %02d:%02d:%02d ", ts->Hours, ts->Minutes, ts->Seconds);
//    printf("%02d-%02d-%02d\r\n", ds->Date, ds->Month, ds->Year);

	Date = dt->Date;
	Date--;
	month = dt->Month;
	month--;
	year = dt->Year + 2000; //YEAR_0;

	epoch += (Date * SECS_PER_DAY) + (dt->Hours * SECS_PER_HOUR) + (dt->Minutes * SECS_PER_MINUTE) + dt->Seconds;

	while (month > 0)
	{
		month--;
		epoch += table[leap_year(year)][month] * SECS_PER_DAY;
	}

	while (year > EPOCH_YEAR)
	{
		year--;
		epoch += year_size(year) * SECS_PER_DAY;
	}

	return epoch;
}

/*!
 * \brief This function determines if the year is a leap year.
 *
 * \param year The year.
 *
 * \return True on a leap year, false otherwise.
 */
static bool leap_year(uint16_t year)
{
	return (!(year % 4) && ((year % 100) || !(year % 400)));
}

/*!
 * \brief This function calculates the number of days in a year.
 *
 * \param year The year.
 *
 * \return The number of days in the year.
 */
static uint16_t year_size(uint16_t year)
{
	return (leap_year(year) ? 366 : 365);
}

/*!
 * \brief This function calculates if daylight saving.
 *
 * \param day of month and month
 *
 * \return true is summertime (DST)
 */
static bool IsDst_EU(int8_t day, int8_t month, int8_t dow)
{
  if (month < 3 || month > 10)
    return false;
  if (month > 3 && month < 10)
    return true;

  int previousSunday = day - dow;

  if (month == 3)
    return previousSunday >= 25;
  if (month == 10)
    return previousSunday < 25;

  return false; // this line never gonna happend
}

// Code for USA
static bool IsDst_US(int8_t day, int8_t month, int8_t dow)
{
  //January, february, and december are out.
  if (month < 3 || month > 11)
    return false;

  //April to October are in
  if (month > 3 && month < 11)
    return true;

  int8_t previousSunday = day - dow;
  //In march, we are DST if our previous sunday was on or after the 8th.

  if (month == 3)
    return previousSunday >= 8;

  //In november we must be before the first sunday to be dst.
  //That means the previous sunday must be before the 1st.
  return previousSunday <= 0 ? true : false;
}

void time_stamp(void)
{
	time_date_t time_date;

	rtc_get(&time_date);
	tty_printf("%02d:%02d:%02d ", time_date.Hours, time_date.Minutes, time_date.Seconds);
}

/*
 * return the corrected time to set
 */
uint16_t set_dst_correction(uint16_t modin, pmode_t mode)
{
    int16_t result = 0;
    time_date_t td;

    rtc_get(&td);

    switch (mode)
    {
		case TIMED_OFF:
    	case TIMED_ON:
    		return modin;
    		break;
    	case TIMED_EU_SUMMER:
    	 result = modin + (IsDst_EU(td.Date, td.Month, td.WeekDay) ? 60 : 0);

    		break;
    	case TIMED_US_SUMMER:
       	 result = modin + (IsDst_US(td.Date, td.Month, td.WeekDay) ? 60 : 0);
    		break;
    }
    if (result > 24 * 60)
       result -= 24 * 60;

  return result;
}

/*
 * return the corrected time to set
 */
uint16_t get_dst_correction(uint16_t modin, pmode_t mode)
{
    int16_t result = 0;
    time_date_t td;

    rtc_get(&td);

    switch (mode)
    {
		case TIMED_OFF:
    	case TIMED_ON:
    		return modin;
    		break;
    	case TIMED_EU_SUMMER:
    	 result = modin - (IsDst_EU(td.Date, td.Month, td.WeekDay) ? 60 : 0);
    		break;
    	case TIMED_US_SUMMER:
       	 result = modin - (IsDst_US(td.Date, td.Month, td.WeekDay) ? 60 : 0);
    		break;
    }
    if (result < 0)
       result += 24 * 60;

  return result;
}

/* USER CODE END 1 */
