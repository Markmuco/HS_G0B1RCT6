/*
 * motors.c
 *
 *  Created on: 11 jan. 2019
 *      Author: VM
 */
#include <stdlib.h>
#include "main.h"
#include "vars.h"
#include "tim.h"
#include "time.h"
#include "vars.h"
#include "motors.h"
#include "e2p.h"
#include "uart_sci.h"
#include "shell.h"
#include "flash_ee.h"
#include "rtc.h"
#include "ayct102.h"

static uint8_t hal_xa_tmr = NO_TIMER;
static uint8_t hal_xb_tmr = NO_TIMER;
static uint8_t hal_ya_tmr = NO_TIMER;
static uint8_t hal_yb_tmr = NO_TIMER;

static bool position_saved = true;
static uint16_t hal_ms;
static bool timer_overflow = false;

static dir_t direction_x = NONE;
static dir_t direction_y = NONE;

static void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
static void soft_start_stop(int32_t target_x, int32_t target_y, int16_t soft);
static void measure_phase(phase_t *p_phase, uint16_t GPIO_Pin, uint16_t irq_a_pin, uint16_t irq_b_pin, bool enc_a, bool enc_b);
static bool is_endY_debounce(void);
static bool is_endX_debounce(void);

/*******************************************************************************
 *  Speed Controller
 *  culculate the x and y speed and enable h-bridge
 *
 *  return True = Normal, false = busy nulling
 */
bool motor_process(void)
{
	static uint8_t ignore_switch_x_tmr = NO_TIMER;
	static uint8_t ignore_switch_y_tmr = NO_TIMER;
	static uint8_t disable_remote_tmr = NO_TIMER;

	static uint8_t int_x_tmr = NO_TIMER;
	static uint8_t int_y_tmr = NO_TIMER;

	static uint8_t end_x_tmr = NO_TIMER;
	static uint8_t end_y_tmr = NO_TIMER;

	static dir_t old_x_direction;
	static dir_t old_y_direction;

	static uint8_t control_tmr = NO_TIMER;
	static uint8_t position_save_tmr = NO_TIMER;

	static int32_t integral_x = 0; // intergrator
	static int32_t integral_y = 0; // intergrator

	static int32_t target_pwm_x = 0; // calculated value: 0..100
	static int32_t target_pwm_y = 0;
	static bool save_flash = false;
	int32_t error = 0; // error offset

	// it can take up to 50x i to have some power
	hal_ms = 250 * vars.hwinfo.pid.i;

	// Intergrator timer
	if (int_x_tmr == NO_TIMER)
	{
		int_x_tmr = timer_get();
		timer_start(int_x_tmr, ki, NULL);
	}

	if (int_y_tmr == NO_TIMER)
	{
		int_y_tmr = timer_get();
		timer_start(int_y_tmr, ki, NULL);
	}

	// Process interval
	if (control_tmr == NO_TIMER)
	{
		control_tmr = timer_get();
		timer_start(control_tmr, proces_ms, NULL);
	}

	// The actual PWM setting
	soft_start_stop(target_pwm_x, target_pwm_y, softstart);

	// Check for too long on endswitch
	if (isEND_X)
	{
		if (end_x_tmr == NO_TIMER)
		{
			end_x_tmr = timer_get();
			timer_start(end_x_tmr, debounce + 1000, NULL);
		}
	}
	else
		timer_free(&end_x_tmr);

	if (isEND_Y)
	{
		if (end_y_tmr == NO_TIMER)
		{
			end_y_tmr = timer_get();
			timer_start(end_y_tmr, debounce + 1000, NULL);
		}
	}
	else
		timer_free(&end_y_tmr);

	/*
	 * Main motor controller
	 */
	if (timer_elapsed(control_tmr))
	{
		timer_start(control_tmr, proces_ms, NULL);

		/*
		 * xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		 */

		error = setpoint_x - feedback_x;

		// need to move?
		if (abs(error) > ((hal_xa_tmr == NO_TIMER) ? (hysteresis_x * 2) : hysteresis_x))
		{
			BRIDGE_DRIVE_EN;

			// start HAL guard timer
			if (hal_xa_tmr == NO_TIMER)
			{
				hal_xa_tmr = timer_get();
				timer_start(hal_xa_tmr, hal_ms, NULL);
			}

			if (hal_xb_tmr == NO_TIMER)
			{
				hal_xb_tmr = timer_get();
				timer_start(hal_xb_tmr, hal_ms, NULL);
			}

			if (error > 0)
				direction_x = FORWARD;
			else
				direction_x = REVERSE;

			// Intergrator if on
			if (ki)
			{
				if (timer_elapsed(int_x_tmr))
				{
					timer_start(int_x_tmr, ki, NULL);
					// p below offset integrate
					if (abs(error) < FULLPWM)
						direction_x == FORWARD ? integral_x++ : integral_x--;
					else
						integral_x = 0;
				}
			}

			// Change of direction clears the intergrator
			if (direction_x != old_x_direction)
				integral_x = 0;
			old_x_direction = direction_x;

			// calculate output E = P + I
			target_pwm_x = ((error * kp) / 1000) + (direction_x == FORWARD ? min_pwm : -min_pwm);
			// max limiter
			if (target_pwm_x > max_pwm && target_pwm_x > 0)
				target_pwm_x = max_pwm;

			// max limiter
			if (target_pwm_x < -max_pwm && target_pwm_x < 0)
				target_pwm_x = -max_pwm;

			target_pwm_x += integral_x;

			// max limiter
			if (target_pwm_x > FULLPWM && target_pwm_x > 0)
				target_pwm_x = FULLPWM;

			// max limiter
			if (target_pwm_x < -FULLPWM && target_pwm_x < 0)
				target_pwm_x = -FULLPWM;

			//tty_printf("i=%d DIR:%d pwm=%d | %d %d\r\n", integral_x, direction_x, target_pwm_x, setpoint_x, feedback_x); //p, pwm_x, target_pwm_x, motor_pos_x, pos_x);
		}

		// enable bridge calculation
		else
		{
			target_pwm_x = 0;
			direction_x = NONE;
			integral_x = 0;
			timer_free(&hal_xa_tmr);
			timer_free(&hal_xb_tmr);
		}

		/*
		 * yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
		 */
		error = setpoint_y - feedback_y;

		// need to move?
		if (abs(error) > ((hal_ya_tmr == NO_TIMER) ? (hysteresis_y * 2) : hysteresis_y))
		{
			BRIDGE_DRIVE_EN;

			// start HAL guard timer
			if (hal_ya_tmr == NO_TIMER)
			{
				hal_ya_tmr = timer_get();
				timer_start(hal_ya_tmr, hal_ms, NULL);
			}

			if (hal_yb_tmr == NO_TIMER)
			{
				hal_yb_tmr = timer_get();
				timer_start(hal_yb_tmr, hal_ms, NULL);
			}

			if (error > 0)
				direction_y = FORWARD;
			else
				direction_y = REVERSE;

			// Intergrator if on
			if (ki)
			{
				if (timer_elapsed(int_y_tmr))
				{
					timer_start(int_y_tmr, ki, NULL);
					// p below offset integrate
					if (abs(error) < FULLPWM)
						direction_y == FORWARD ? integral_y++ : integral_y--;
					else
						integral_y = 0;
				}
			}

			// Change of direction clears the intergrator
			if (direction_y != old_y_direction)
				integral_y = 0;
			old_y_direction = direction_y;

			// calculate output E = P + I
			target_pwm_y = ((error * kp) / 1000); // + (direction_y == FORWARD ? min_pwm : -min_pwm);

			// max limiter
			if (target_pwm_y > max_pwm && target_pwm_y > 0)
				target_pwm_y = max_pwm;

			// max limiter
			if (target_pwm_y < -max_pwm && target_pwm_y < 0)
				target_pwm_y = -max_pwm;

			target_pwm_y += integral_y;

			// max limiter
			if (target_pwm_y > FULLPWM && target_pwm_y > 0)
				target_pwm_y = FULLPWM;

			// max limiter
			if (target_pwm_y < -FULLPWM && target_pwm_y < 0)
				target_pwm_y = -FULLPWM;

			//	tty_printf("i=%d DIR:%dp=%d pwm=%d goal=%d | %d %d\r\n", integral_x, direction_x, p, pwm_x, target_pwm_x, motor_pos_x, pos_x);
		}

		// enable bridge calculation
		else
		{
			target_pwm_y = 0;
			direction_y = NONE;
			integral_y = 0;
			timer_free(&hal_ya_tmr);
			timer_free(&hal_yb_tmr);
		}
		//tty_printf("%d %d %d %d\r\n", target_pwm_x, integral_x, target_pwm_y, integral_y);
	}

	if (direction_x == NONE && direction_y == NONE)
	{
		// H bridges off
		BRIDGE_DRIVE_DIS;

		if (!position_saved)
		{
			if (position_save_tmr == NO_TIMER)
			{
				position_save_tmr = timer_get();
				timer_start(position_save_tmr, SAVE_AFTER_MOVE, NULL);
			}
			if (timer_elapsed(position_save_tmr))
			{
				timer_free(&position_save_tmr);
				time_stamp();
				tty_printf("Saving position %d %d\r\n", vars.eevar.actual_motor.x, vars.eevar.actual_motor.y);
				if (WriteStruct2eerom(vars.eevar))
					tty_printf("  Error write eeprom\r\n");
				position_saved = true;
			}
		}

		if (save_flash)
		{
			save_flash = false;
			time_stamp();
			tty_printf("Saving Flash\r\n");

			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
	}
	else
		timer_free(&position_save_tmr);

// guard of HAL sensors
	if (timer_elapsed(hal_xa_tmr))
	{
		tty_printf("xi=%d DIR:%d pwm=%d | %d %d\r\n", integral_x, direction_x, target_pwm_x, setpoint_x, feedback_x);

		tty_printf(" ERROR HAL XA\r\n");
		timer_free(&hal_xa_tmr);
		vars.eevar.main_mode = ST_HAL_TIMEOUT;
		vars.error_status |= ERR_HAL_XA;
	}

	if (timer_elapsed(hal_xb_tmr))
	{
		tty_printf("xi=%d DIR:%d pwm=%d | %d %d\r\n", integral_x, direction_x, target_pwm_x, setpoint_x, feedback_x);

		tty_printf(" ERROR HAL XB\r\n");
		timer_free(&hal_xb_tmr);
		vars.eevar.main_mode = ST_HAL_TIMEOUT;
		vars.error_status |= ERR_HAL_XB;
	}

	if (timer_elapsed(hal_ya_tmr))
	{
		tty_printf("yi=%d DIR:%d pwm=%d | %d %d\r\n", integral_y, direction_y, target_pwm_y, setpoint_y, feedback_y);

		tty_printf(" ERROR HAL YA\r\n");
		timer_free(&hal_ya_tmr);
		vars.eevar.main_mode = ST_HAL_TIMEOUT;
		vars.error_status |= ERR_HAL_YA;
	}

	if (timer_elapsed(hal_yb_tmr))
	{
		tty_printf("yi=%d DIR:%d pwm=%d | %d %d\r\n", integral_y, direction_y, target_pwm_y, setpoint_y, feedback_y);

		tty_printf(" ERROR HAL YB\r\n");
		timer_free(&hal_yb_tmr);
		vars.eevar.main_mode = ST_HAL_TIMEOUT;
		vars.error_status |= ERR_HAL_YB;
	}


	if (timer_elapsed(end_x_tmr))
	{
		tty_printf(" ERR too long on x-endswitch\r\n");

		timer_free(&end_x_tmr);
		vars.eevar.main_mode = ST_END_TIMEOUT;
		vars.error_status |= ERR_END_X;
	}

	if (timer_elapsed(end_y_tmr))
	{
		tty_printf(" ERR too long on y-endswitch\r\n");

		timer_free(&end_y_tmr);
		vars.eevar.main_mode = ST_END_TIMEOUT;
		vars.error_status |= ERR_END_Y;
	}


	// Test F switches NOT in recover mode
	if (vars.eevar.main_mode != ST_RECOVER)
	{
		if (is_endX_debounce())
		{
			if (disable_remote_tmr == NO_TIMER)
			{
				disable_remote_tmr = timer_get();
				timer_start(disable_remote_tmr, STUCK_ON_END_TIME, NULL);
				HAL_TIM_Base_Stop_IT(&htim16); // Stop Timer 16 IRQ
			}

			// Too long on endposition enable the remote
			if ((timer_elapsed(disable_remote_tmr)) && (htim16.State == HAL_TIM_STATE_READY))
				HAL_TIM_Base_Start_IT(&htim16); // Start Timer 16 IRQ

			// Hit zero
			if (direction_x == REVERSE && ignore_switch_x_tmr == NO_TIMER)
			{
				if (ignore_switch_x_tmr == NO_TIMER)
					ignore_switch_x_tmr = timer_get();

				timer_start(ignore_switch_x_tmr, debounce, NULL);

				// New position after ramp down
				setpoint_x = (steps_x * turnback) / 10;

				// Measured vs real position
				deviation_x = feedback_x;

				tty_printf("XZero at %d\r\n", feedback_x);
				feedback_x = 0;
			}
			// Hit max
			if (direction_x == FORWARD && ignore_switch_x_tmr == NO_TIMER)
			{
				if (ignore_switch_x_tmr == NO_TIMER)
					ignore_switch_x_tmr = timer_get();

				timer_start(ignore_switch_x_tmr, debounce, NULL);

				max_x = feedback_x;

				save_flash = true;
				// New position after ramp down
				setpoint_x = max_x - (steps_x * turnback) / 10;

				tty_printf("XMAX at %d\r\n", feedback_x);
			}
		}

		if (is_endY_debounce())
		{
			if (disable_remote_tmr == NO_TIMER)
			{
				disable_remote_tmr = timer_get();
				timer_start(disable_remote_tmr, STUCK_ON_END_TIME, NULL);
				HAL_TIM_Base_Stop_IT(&htim16); // Stop Timer 16 IRQ
			}

			// Too long on endposition enable the remote
			if ((timer_elapsed(disable_remote_tmr)) && (htim16.State == HAL_TIM_STATE_READY))
				HAL_TIM_Base_Start_IT(&htim16); // Start Timer 16 IRQ

			// Hit zero
			if (direction_y == REVERSE && ignore_switch_y_tmr == NO_TIMER)
			{
				if (ignore_switch_y_tmr == NO_TIMER)
					ignore_switch_y_tmr = timer_get();

				timer_start(ignore_switch_y_tmr, debounce, NULL);

				// New position after ramp down
				setpoint_y = (steps_y * turnback) / 10;

				// Measured vs real position
				deviation_y = feedback_y;

				tty_printf("YZero at %d\r\n", feedback_y);
				feedback_y = 0;
			}
			// Hit max
			if (direction_y == FORWARD && ignore_switch_y_tmr == NO_TIMER)
			{
				if (ignore_switch_y_tmr == NO_TIMER)
					ignore_switch_y_tmr = timer_get();

				timer_start(ignore_switch_y_tmr, debounce, NULL);

				max_y = feedback_y;
				save_flash = true;

				// New position after ramp down
				setpoint_y = max_y - (steps_y * turnback) / 10;

				tty_printf("YMAX at %d\r\n", max_y);

			}
		}

		if (!is_endX_debounce() && !is_endY_debounce())
		{
			if (htim16.State == HAL_TIM_STATE_READY)
				HAL_TIM_Base_Start_IT(&htim16); // Start Timer 16 IRQ
			timer_free(&disable_remote_tmr);
		}
	}

// end the timers
	if (timer_elapsed(ignore_switch_x_tmr) && !isEND_X)
	{
		HAL_TIM_Base_Start_IT(&htim16); // Start Timer 16 IRQ
		timer_free(&ignore_switch_x_tmr);
	}

	if (timer_elapsed(ignore_switch_y_tmr) && !isEND_Y)
	{
		HAL_TIM_Base_Start_IT(&htim16); // Start Timer 16 IRQ
		timer_free(&ignore_switch_y_tmr);
	}

	return (ignore_switch_x_tmr == NO_TIMER && ignore_switch_y_tmr == NO_TIMER);
}


/*
 *
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	HAL_GPIO_EXTI_Callback(GPIO_Pin);
}

/*
 *
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	HAL_GPIO_EXTI_Callback(GPIO_Pin);
}

/**
 * @brief  EXTI line detection callback.
 * 			Encoder pulses are about 5ms
 * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
static void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	extern vars_t vars;

	static uint32_t hal_debounce_xa = 0;
	static uint32_t hal_debounce_xb = 0;
	static uint32_t hal_debounce_ya = 0;
	static uint32_t hal_debounce_yb = 0;

	uint32_t now = HAL_GetTick();

	switch (GPIO_Pin)
	{
	case MX_A_Pin:
		position_saved = false;
		timer_start(hal_xa_tmr, hal_ms, NULL);
		vars.error_status &= ~ERR_HAL_XA;

		if (now != hal_debounce_xa)
		{
			if (isMX_A == isMX_B)
				vars.eevar.actual_motor.x++;
			else
				vars.eevar.actual_motor.x--;
		}
		else
			tty_printf("debounce XA\r\n");

		hal_debounce_xa = now;
		break;

	case MX_B_Pin:
		position_saved = false;
		timer_start(hal_xb_tmr, hal_ms, NULL);
		vars.error_status &= ~ERR_HAL_XB;

		if (now != hal_debounce_xb)
		{
			if (isMX_A == isMX_B)
				vars.eevar.actual_motor.x--;
			else
				vars.eevar.actual_motor.x++;
		}
		else
			tty_printf("debounce XB\r\n");

		hal_debounce_xb = now;
		break;

	case MY_A_Pin:
		position_saved = false;
		timer_start(hal_ya_tmr, hal_ms, NULL);
		vars.error_status &= ~ERR_HAL_YA;

		if (now != hal_debounce_ya)
		{
			if (isMY_A == isMY_B)
				vars.eevar.actual_motor.y++;
			else
				vars.eevar.actual_motor.y--;
		}
		else
			tty_printf("debounce YA\r\n");

		hal_debounce_ya = now;
		break;

	case MY_B_Pin:
		position_saved = false;
		timer_start(hal_yb_tmr, hal_ms, NULL);
		vars.error_status &= ~ERR_HAL_YB;

		if (now != hal_debounce_yb)
		{
			if (isMY_A == isMY_B)
				vars.eevar.actual_motor.y--;
			else
				vars.eevar.actual_motor.y++;
		}
		else
			tty_printf("debounce YB\r\n");

		hal_debounce_yb = now;
		break;

	case RX433_Pin:
		AYCT_EXTI_IRQHandler();
		break;

	default:
		break;
	}
}

/*
 * Measure phase timeout
 */
void timer6_irq(void)
{
	timer_overflow = true;
}

/**
 * @brief  Soft start/Stop
 * @param  x,y , time
 * @retval None
 */
static void soft_start_stop(int32_t target_x, int32_t target_y, int16_t soft)
{
	static uint8_t softstart_tmr = NO_TIMER;
	static int32_t actual_x = 0; // actual motor value: 0..100
	static int32_t actual_y = 0;

	if (softstart_tmr == NO_TIMER)
	{
		softstart_tmr = timer_get();
		timer_start(softstart_tmr, soft / 100, NULL);
	}

// Soft start/stop
	if (timer_elapsed(softstart_tmr))
	{
		timer_start(softstart_tmr, soft / 100, NULL);

		// softstart with limiter -100..0..+100
		if (actual_x < target_x)
			actual_x++;

		if (actual_x > target_x)
			actual_x--;
#if 1
		if (actual_x > 0)
			DIR_X_1;
		else
			DIR_X_0;

		set_x_pwm(abs(actual_x));
#else
		set_x_pwm(actual_x);
#endif
		// softstart with limiter -100..0..+100
		if (actual_y < target_y)
			actual_y++;

		if (actual_y > target_y)
			actual_y--;

#if 1
		if (actual_y > 0)
			DIR_Y_1;
		else
			DIR_Y_0;

		//tty_printf("%d ", actual_x);

		set_y_pwm(abs(actual_y));
#else


		set_y_pwm(actual_y);
#endif
	}
}

/*
 * Set the time structure with A and B encoder rising and falling edge
 */
static void measure_phase(phase_t *p_phase, uint16_t GPIO_Pin, uint16_t irq_a_pin, uint16_t irq_b_pin, bool enc_a, bool enc_b)
{
	if (timer_overflow)
	{
		tty_printf("Overflow\r\n");
		timer_overflow = false;
		HAL_TIM_Base_Stop_IT(&htim6);
		p_phase->stage = 0;
		p_phase->duty_a = 1;
		p_phase->duty_b = 2;
		p_phase->shift = 3;
	}

	// start of measure
	switch (p_phase->stage)
	{
	case 0:
		// Wait for start
		break;

	case 1:
		HAL_TIM_Base_Start_IT(&htim6); // Start Timer 6
		__HAL_TIM_SET_COUNTER(&htim6, 0);

		// Init wait for A rising
		if ((GPIO_Pin == irq_a_pin) && enc_a)
			p_phase->stage++;
		break;

	case 2:
		// B rising
		if ((GPIO_Pin == irq_b_pin) && enc_b)
		{
			p_phase->b_rise = __HAL_TIM_GET_COUNTER(&htim6);
			p_phase->stage++;
		}
		break;

	case 3:
		// A falling
		if ((GPIO_Pin == irq_a_pin) && !enc_a)
		{
			p_phase->a_fall = __HAL_TIM_GET_COUNTER(&htim6);
			p_phase->stage++;
		}
		break;

	case 4:
		// B falling
		if ((GPIO_Pin == irq_b_pin) && !enc_b)
		{
			p_phase->b_fall = __HAL_TIM_GET_COUNTER(&htim6);
			p_phase->stage++;
		}
		break;

	case 5:
		// A rise
		if ((GPIO_Pin == irq_a_pin) && enc_a)
		{
			p_phase->a_rise = __HAL_TIM_GET_COUNTER(&htim6);
			p_phase->stage++;
		}
		break;

	case 6:
		// B rise lastpulse
		if ((GPIO_Pin == irq_b_pin) && enc_b)
		{
			p_phase->b_rise_end = __HAL_TIM_GET_COUNTER(&htim6);
			p_phase->stage++;
		}
		break;

	case 7:
		// timer6 counts 48MHz / 96 = 500khz = 2 us
		// calculate values

		p_phase->frequency = 4800000 / (p_phase->a_rise);
		p_phase->duty_a = (p_phase->a_fall * 100) / p_phase->a_rise;
		p_phase->duty_b = ((p_phase->b_fall - p_phase->b_rise) * 100) / (p_phase->b_rise_end - p_phase->b_rise);
		p_phase->shift = ((p_phase->b_rise) * 100) / (p_phase->a_fall);

		HAL_TIM_Base_Stop_IT(&htim6);
		p_phase->stage = 0;
		break;

	default:
		break;
	}
}



/*
 * Debouncing the END switch
 */
static bool is_endX_debounce(void)
{
	static uint8_t tmr = NO_TIMER;

	if (isEND_X)
	{
		if (tmr == NO_TIMER)
		{
			tmr = timer_get();
			timer_start(tmr, DEBOUNCE_END, NULL);
		}
	}
	else
		timer_free(&tmr);

	return timer_elapsed(tmr) ? true : false;
}

/*
 * Debouncing the END switch
 */
static bool is_endY_debounce(void)
{
	static uint8_t tmr = NO_TIMER;

	if (isEND_Y)
	{
		if (tmr == NO_TIMER)
		{
			tmr = timer_get();
			timer_start(tmr, DEBOUNCE_END, NULL);
		}
	}
	else
		timer_free(&tmr);

	return timer_elapsed(tmr) ? true : false;
}

#if 0 // code for online compiler
#include  <stdio.h>
#include  <stdint.h>
#include <stdlib.h>

int32_t setpoint_x = 1006;
int32_t feedback_x = 1000;

int32_t integral_x = 0;

int32_t min_pwm = 30;
int32_t max_pwm = 100;

int32_t target_pwm_x = 0;
int32_t target_pwm_y = 0;

int16_t softstart = 0;

int32_t hysteresis_x = 2;

typedef enum
{
	NONE,
	FORWARD,
	REVERSE,
}dir_t;

static dir_t direction_x = NONE;
static dir_t old_x_direction = NONE;

static void soft_start_stop(int32_t target_x, int32_t target_y, int16_t ss);

#define FULLPWM 100
#define kp 1000

void calc(void)
{
	int32_t error = 0; // error offset

	error = setpoint_x - feedback_x;

// need to move?
	if (abs(error) > hysteresis_x)
	{
		if (error > 0)
		direction_x = FORWARD;
		else
		direction_x = REVERSE;

		// p below offset integrate
		if (abs(error) < FULLPWM)
		direction_x == FORWARD ? integral_x++ : integral_x--;
		else
		integral_x = 0;

		if (direction_x != old_x_direction)
		integral_x = 0;
		old_x_direction = direction_x;

		//target_pwm_x = ((error * kp) / 1000) + integral_x + (direction_x == FORWARD ? min_pwm : -min_pwm);
		target_pwm_x = ((error * kp) / 1000) + integral_x;
#if 1

#endif
	}

// enable bridge calculation
	else
	{
	}
//	printf("%d ", target_pwm_x);
	soft_start_stop(target_pwm_x, target_pwm_y, softstart);
}

static void soft_start_stop(int32_t target_x, int32_t target_y, int16_t ss)
{
	static int16_t actual_x = 0; // actual motor value: 0..100

// Soft start/stop
	{

		if (target_x > 0)
		target_x += min_pwm;
		else
		target_x -= min_pwm;

		if (actual_x > 0 && actual_x < min_pwm)
		actual_x = min_pwm;
#if 0
		if (actual_x < 0 && actual_x > min_pwm)
		actual_x = -min_pwm;
#endif
		// softstart with limiter -100..0..+100
		if (actual_x < target_x)
		actual_x++;

		if (actual_x > target_x)
		actual_x--;

		// max limiter
		if (actual_x > max_pwm)
		actual_x = max_pwm;

		// max limiter
		if (actual_x < -max_pwm)
		actual_x = -max_pwm;

//		printf("%d ", actual_x);

		printf("%d=%d ", target_x, actual_x);
	}
}

int main(void)
{
	printf("Pwm ");
	for (int var = 0; var < 300;++var)
	{
		if (var == 80)
		{
			printf("Reverse\n");
			feedback_x = 1020;
		}

		calc();
	}

	return 0;
}
#endif


