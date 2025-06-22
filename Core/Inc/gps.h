/*!
 * \file gps.h
 *
 * \brief This software module provides functions for the GPS module.
 */
#ifndef _GPS_H_
#define _GPS_H_

#include <stdbool.h>
#include "vars.h"


#define GPS_TIMEOUT   (120 * 1000) // 120sec.

#define gps_printf		tty_printf
#define gps_getch		sci6_getch

// GPS result codes.
typedef enum
{
  GPS_ERR_OK = 0,
  GPS_ERR_BUSY,
  GPS_ERR_FAILED
} gps_err_t;

// GPS messages.
typedef enum
{
  GPS_FRAME_NONE = 0,
  GPS_FRAME_GGA,
  GPS_FRAME_RMC,
  GPS_FRAME_TXT
} gps_frame_t;

// GPS data.
typedef struct
{
  char latitude[11+1];  // ddmm.mmmm[N/S]
  char longitude[12+1]; // dddmm.mmmm[E/W]
  char time[6+1];       // hhmmss
  char date[6+1];       // yymmdd
  char fix[1+1];        // 'A' or 'V'
  char numsat[2+1];
  char version[50];		// ROM CORE 1.00 (59842) Jun 27 2012 17:43:52*59
  bool dv;              // Data valid
} gps_data_t;

typedef struct
{
	  uint8_t hour;
	  uint8_t minute;
	  uint8_t second;
} gps_time_t;


typedef struct
{
	  uint8_t day;
	  uint8_t month;
	  uint8_t year;
} gps_date_t;

// GPS data numeric
typedef struct
{
  float latitude;
  float longitude;
  gps_time_t gps_time;
  gps_date_t gps_date;
  bool fix;
  uint8_t numsat;
  uint8_t progress; // percentage of message
  char version[15];		// ROM CORE 1.00

} gps_info_t;


// GPS process commands.
typedef enum
{
  GPS_CMD_IDLE = 0,
  GPS_CMD_POWERON,
  GPS_CMD_POWEROFF,
  GPS_CMD_SLEEP,
  GPS_CMD_WAKEUP
} gps_cmd_t;

// GPS function table object, used to map specific implementations.
typedef struct
{
  gps_err_t (*d_poweron)              (void);
  gps_err_t (*d_poweroff)             (void);
  gps_err_t (*d_sleep)                (void);
  gps_err_t (*d_wakeup)               (void);
  bool      (*d_getc)                 (char *c);
} gps_fxns_t;

// Public function prototypes.
void gps_init(void);
void gps_deinit(void);
uint8_t gps_process(location_t * location);
//void gps_start(void);
uint32_t gps_remain_valid(void);
void override_gps(void);

#endif
