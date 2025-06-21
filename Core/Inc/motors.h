/*
 * motors.h
 *
 *  Created on: 11 jan. 2019
 *      Author: VM
 */

#ifndef MOTORS_H_
#define MOTORS_H_

#define setpoint_x		vars.goto_motor.x
#define setpoint_y		vars.goto_motor.y

#define feedback_x		vars.eevar.actual_motor.x
#define feedback_y		vars.eevar.actual_motor.y

#define deviation_x		vars.deviation.x
#define deviation_y		vars.deviation.y

#define hysteresis_x	vars.hwinfo.hysteresis.x
#define hysteresis_y	vars.hwinfo.hysteresis.y
#define proces_ms		vars.hwinfo.pid.repeat_ms
#define ki			 	vars.hwinfo.pid.i
#define kp				vars.hwinfo.pid.p
#define softstart		vars.hwinfo.pid.softstart

#define debounce		vars.hwinfo.debounce
#define steps_x			vars.hwinfo.steps.x
#define steps_y			vars.hwinfo.steps.y
#define max_pwm			vars.max_pwm			// maximum is flexible, 100% at slider 3 min/max running, flash parameter at normal operation
#define min_pwm			vars.hwinfo.min_pwm
#define max_x			vars.hwinfo.maximum.x
#define max_y			vars.hwinfo.maximum.y
#define turnback		vars.hwinfo.turnback


#define NEWPOS_TIME		400 // ms After zero hit, set new value


typedef enum
{
	 NONE,
	 FORWARD,
	 REVERSE,
}dir_t;


bool motor_process(void);
void timer6_irq(void);

#endif /* MOTORS_H_ */
