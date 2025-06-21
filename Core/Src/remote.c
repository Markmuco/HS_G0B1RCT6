/*
 * remote.c
 *
 *  Created on: 15 jan. 2019
 *      Author: Mark
 */

#include "main.h"
#include "ayct102.h"
#include "rtc.h"
#include "usart.h"
#include "vector.h"
#include "flash_ee.h"
#include "suncalc.h"
#include "time.h"
#include "vars.h"
#include "uart_sci.h"
#include "machine.h"
/*!
 * \brief This function handles the remote
 *
 * \param pointer to vars
 *
 * \return
 */
void remote_ctrl_process(void)
{
	static uint8_t recover_tmr = NO_TIMER;
	static uint16_t recover_counter = 0;
	ayct102_t ayct102;
//	motorpos_t tg;

	/**
	 * AYCT-102 Keys:
	 * 1= 4 - 0 (ON - OFF)
	 * 2= 5 - 1
	 * 3= 6 - 2
	 * 4= 7 - 3
	 *
	 * G 12 - 8
	 *
	 * Slider 0-1-2-3
	 * (1)
	 * 1= STOP - PARK
	 * 2= TG1 - SUN
	 * 3= TG2 - TG3
	 * 4= TG4 - TG5
	 *
	 * (2) Manual
	 * 1= STOP - ?
	 * 2= CL - CCL
	 * 3= NC - UP
	 * 4= NC - DOWN
	 *
	 *(3) End
	 * 1= STOP - NC
	 * 2= CL - CCL
	 * 3= NC - UP
	 * 4= NC - DOWN
	 *
	 *(4) Save
	 * 1= STOP - PARK
	 * 2= TG1 - SUN
	 * 3= TG2 - TG3
	 * 4= TG4 - TG5
	 *
	 */

	if (timer_elapsed(recover_tmr))
	{
		recover_counter = 0;
		timer_free(&recover_tmr);
	}

	if (recover_counter == 30)
	{
		recover_counter = 0;
		time_stamp();
		tty_printf("Recover_mode\r\n");
		vars.eevar.main_mode = ST_RECOVER;
	}

	if (get_ayct(&ayct102))
	{
		// save last received into globals for save remote purpose
		vars.lastrx_ayct102_home = ayct102.home;

		// my remote controller?
		if ((ayct102.home == vars.hwinfo.ayct102_home_1) || (ayct102.home != 0 && ayct102.home == vars.hwinfo.ayct102_home_2))
		{
			if (vars.screen_tmr == NO_TIMER)
			{
				tty_printf("Start screen tmr\r\n");
				vars.screen_tmr = timer_get();
			}

			timer_start(vars.screen_tmr, SCREEN_ON_TIME, NULL);
			LCD_BACK_ON;

			// if target is about to be saved only G_ON or G_OFF is allowed
			if (ayct102.key != KEYG_ON && ayct102.key != KEYG_OFF && vars.about_to_save != TG_NONE)
			{
				tty_printf("Abort about to save\r\n");
				vars.about_to_save = TG_NONE;
			}

			switch (ayct102.slide)
			{
			case SLIDER_I: // mode set target
				timer_free(&vars.tracking_tmr);
				timer_free(&vars.calc_sun_tmr);

				switch (ayct102.key)
				{
				case KEY1_ON:
					time_stamp();
					tty_printf("Stop\r\n");
					vars.eevar.main_mode = ST_STOP;
					vars.goto_motor = vars.eevar.actual_motor;
					break;

				case KEY1_OFF:
					time_stamp();
					tty_printf("Off, Parking\r\n");
					vars.goto_motor = vars.hwinfo.parkposition;
					vars.eevar.main_mode = ST_OFF;
					break;

				case KEY2_ON:
					time_stamp();
					tty_printf("Target 1\r\n");
					vars.eevar.target = vars.hwinfo.target[0].pos;
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_TARGET_1;

					break;
				case KEY2_OFF:
					time_stamp();
					tty_printf("Target SUN\r\n");
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_SUN;
					break;

				case KEY3_ON:
					time_stamp();
					tty_printf("Target 2\r\n");
					vars.eevar.target = vars.hwinfo.target[1].pos;
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_TARGET_2;

					break;
				case KEY3_OFF:
					time_stamp();
					tty_printf("Target 3\r\n");
					vars.eevar.target = vars.hwinfo.target[2].pos;
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_TARGET_3;
					break;

				case KEY4_ON:
					time_stamp();
					tty_printf("Target 4\r\n");
					vars.eevar.target = vars.hwinfo.target[3].pos;
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_TARGET_4;
					break;

				case KEY4_OFF:
					time_stamp();
					tty_printf("Target 5\r\n");
					vars.eevar.target = vars.hwinfo.target[4].pos;
					vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_TARGET_5;
					break;

					// YES on saving
				case KEYG_ON:
					if (vars.about_to_save != TG_NONE)
					{
						switch (vars.about_to_save)
						{
						case TG_PARK:
							// save parkposition
							cmd_savepark();
							break;

						case TG_SUN:
							// Save sun position
							cmd_savesun();
							break;

						case TG_1:
							// Save target to flash
							cmd_savetarget(1);
							break;

						case TG_2:
							// Save target to flash
							cmd_savetarget(2);
							break;

						case TG_3:
							// Save target to flash
							cmd_savetarget(3);
							break;

						case TG_4:
							// Save target to flash
							cmd_savetarget(4);
							break;

						case TG_5:
							// Save target to flash
							cmd_savetarget(5);
							break;

						default:
							break;
						}
					}
					break;

					// Not saving
				case KEYG_OFF:
					if (vars.about_to_save != TG_NONE)
					{
						vars.about_to_save = TG_NONE;
						time_stamp();
						tty_printf("Abort save Stop\r\n");
						vars.eevar.main_mode = ST_STOP;
					}

					// 5 seconds on the G-OFF key for recover mode
					if (recover_tmr == NO_TIMER)
						recover_tmr = timer_get();

					timer_start(recover_tmr, 100, NULL);
					recover_counter++;
					break;

				}
				break;

			case SLIDER_II: // mode manual
				switch (ayct102.key)
				{
				case KEY1_ON:
					time_stamp();
					tty_printf("Stop\r\n");
					vars.eevar.main_mode = ST_STOP;
					vars.goto_motor = vars.eevar.actual_motor;
					break;

				case KEY1_OFF:
					break;

				case KEY2_ON:
					// Move a litte Clockwise
					cmd_move_cw(vars.hwinfo.steps.x / 20);

					break;

				case KEY2_OFF:
					// Move a litte Counter Clockwise
					cmd_move_ccw(vars.hwinfo.steps.x / 20);
					break;

				case KEY3_ON:
					break;

				case KEY3_OFF:
					// Move a litte up
					cmd_move_up(vars.hwinfo.steps.y / 20);
					break;

				case KEY4_ON:
					break;

				case KEY4_OFF:
					// Move a litte down
					cmd_move_down(vars.hwinfo.steps.y / 20);
					break;

				case KEY_NONE:
				case KEYG_ON:
				case KEYG_OFF:
					break;

				}
				break;

			case SLIDER_III: // mode endswitch
				switch (ayct102.key)
				{
				case KEY1_ON:
					time_stamp();
					tty_printf("Stop\r\n");
					vars.eevar.main_mode = ST_STOP;
					vars.goto_motor = vars.eevar.actual_motor;
					break;

				case KEY1_OFF:
					break;

				case KEY2_ON:
					// Calibrate X Clockwise
					cmd_set_end_cw();
					break;

				case KEY2_OFF:
					// Calibrate X Counter Clockwise
					cmd_set_end_ccw();
					break;

				case KEY3_ON:
					break;

				case KEY3_OFF:
					// Calibrate Y up
					cmd_set_end_up();
					break;

				case KEY4_ON:
					break;

				case KEY4_OFF:
					// Calibrate Y down
					cmd_set_end_down();
					break;

				case KEYG_ON:
					break;

				case KEYG_OFF:
					break;
				}
				break;

			case SLIDER_VI: // mode save
				switch (ayct102.key)
				{
				case KEY1_ON:
					time_stamp();
					tty_printf("Stop\r\n");
					vars.eevar.main_mode = ST_STOP;
					vars.goto_motor = vars.eevar.actual_motor;
					break;

				case KEY1_OFF:
					time_stamp();
					tty_printf("Save Parking?\r\n");
					vars.about_to_save = TG_PARK;
					vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
					break;

				case KEY2_ON:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target 1?\r\n");
							vars.about_to_save = TG_1;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;

					break;

				case KEY2_OFF:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target SUN?\r\n");
							vars.about_to_save = TG_SUN;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;
					break;

				case KEY3_ON:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target 2?\r\n");
							vars.about_to_save = TG_2;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;

					break;

				case KEY3_OFF:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target 3?\r\n");
							vars.about_to_save = TG_3;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;

					break;

				case KEY4_ON:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target 4?\r\n");
							vars.about_to_save = TG_4;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;

					break;

				case KEY4_OFF:
					if (vars.gps_decode == 100)
					{
						if (out_learned_range(HORIZONTAL) || out_learned_range(VERTICAL))
						{
							st_main_mode = ST_INVALID_PARAMETERS;
						}
						else
						{
							time_stamp();
							tty_printf("Save Target 5?\r\n");
							vars.about_to_save = TG_5;
							vars.eevar.main_mode = ST_ABOUT_TO_SAVE;
						}
					}
					else
						st_main_mode = ST_WAIT_GPS;

					break;

				}
				break;

			}

		}

		// print remote clrl
//		time_stamp();
//		tty_printf("home 0x%06X slider %d key %d\r\n", ayct102.home, ayct102.slide, ayct102.key);
	}
}
