/*
 * machine.h
 *
 *  Created on: 14 jan. 2019
 *      Author: Mark
 */

#ifndef MACHINE_H_
#define MACHINE_H_

#define mMAIN_MODE					vars.eevar.main_mode
#define show_screen					vars.screen_lcd
#define mSUN_DOWN_ANGLE				vars.hwinfo.sun_down_angle
#define mTRACK_INTERVAL_S			(vars.hwinfo.track_interval * 1000)

#define OUTOF_RANGE_PARK_T			2000 // ms

#define LOW_VOLTAGE					10000 // 10V
#define RESTORE_VOLTAGE				10500 // 10.5V

#define MINUTE						60 * 1000 // minute in ms
#define SECOND						1000 // second in ms

#define SUN							0x02
#define MOON						0x04

void machine_process(void);
void factory(hw_info_t * hwinfo);

void cmd_set_end_cw(void);
void cmd_set_end_ccw(void);
void cmd_set_end_down(void);
void cmd_set_end_up(void);
void cmd_move_cw(uint32_t steps);
void cmd_move_ccw(uint32_t steps);
void cmd_move_up(uint32_t steps);
void cmd_move_down(uint32_t steps);
void cmd_savesun(void);
void cmd_savetarget(uint8_t target);
void cmd_savepark(void);
char * print_mode_name(main_mode_st mode);
bool out_learned_range(uint8_t axle);

#endif /* MACHINE_H_ */
