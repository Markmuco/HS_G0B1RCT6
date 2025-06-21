/*
 * build_display.h
 *
 *  Created on: 10 sep. 2019
 *      Author: Mark
 */

#ifndef BUILD_DISPLAY_H_
#define BUILD_DISPLAY_H_

#define LCD_LENGTH					20 // chars



#define st_lcd_screen				vars.screen_lcd
#define st_main_prn					vars.eevar.main_mode
#define get_err						vars.error_status

void process_display(void);

#endif /* BUILD_DISPLAY_H_ */
