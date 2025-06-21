#ifndef _TIME
#define _TIME

#include "main.h"


#define FUNCTION_TIMERS
  
  
// Maximum number of timers
#define MAX_TIMERS     (32) // Must be a multiple of 8!

// Define invalid timer
#define NO_TIMER       (250)

// Public function prototypes
bool timer_open(void);
bool timer_close(void);
uint8_t timer_get(void);
bool timer_free(uint8_t *timerid);
bool timer_start(uint8_t timerid, uint32_t value, void *p_fxn);
bool timer_stop(uint8_t timerid);
bool timer_resume(uint8_t timerid);
bool timer_elapsed(uint8_t timerid);
uint32_t timer_read(uint8_t timerid);
uint16_t used_timers(void);


void timer_callback(void);

  
#endif




