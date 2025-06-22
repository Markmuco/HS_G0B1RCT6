/*!
 * \file  shell.c
 * \brief This module provides functions command shell
 *
 */
#include "main.h"
#include "e2p.h"
#include "vars.h"
#include "usart.h"
#include "uart_sci.h"
#include "shell.h"
#include "tim.h"
#include "crc.h"
#include "time.h"
#include "main.h"
#include "rtc.h"
#include "crc.h"
#include "iwdg.h"
#include "adc.h"
#include "suncalc.h"
#include "vector.h"
#include "machine.h"
#include "flash_ee.h"
#include "protection.h"
#include "i2c.h"
#include "gps.h"
#include "gpio.h"
#include "external_io.h"

#if LOGIN_ENABLE>0
#include "md5.h"
#endif

// Private variables.
static uint8_t tmr = NO_TIMER;

static char mem_pool[SHELL_CMDLINE_SIZE * (SHELL_CMDLINE_HIST_COUNT + 1)];
static cmdline_obj_t cmdline;
char const * prn_summer[] = { "Off", "Off", "EU", "US"};

// Default must have function prototypes.
static void sh_help(char *argv);
static void sh_reboot(char *argv);
#if LOGIN_ENABLE>0
static bool logged_in = false;
static void sh_login(char *argv);
static void sh_logout(char *argv);
static bool md5compare(char * pwd, uint8_t * arry);
#endif
/* USER CODE BEGIN 1 */

static void sh_time(char * argv);
static void sh_show(char * argv);
static void sh_inputs(char * argv);
static void sh_factory(char * argv);
static void sh_list(char * argv);
static void sh_stop(char * argv);
static void sh_hang(char * argv);
static void sh_stepsx(char * argv);
static void sh_stepsy(char * argv);
static void sh_hystx(char * argv);
static void sh_hysty(char * argv);
static void sh_kp(char * argv);
static void sh_ki(char * argv);
static void sh_kd(char * argv);
static void sh_pidt(char * argv);
static void sh_pids(char * argv);
static void sh_debounce(char * argv);
static void sh_sundown(char * argv);
static void sh_turnback(char * argv);
static void sh_minpwm(char * argv);
static void sh_maxwm(char * argv);
static void sh_windpulse(char * argv);
static void sh_contrast(char * argv);
static void sh_remote(char * argv);
static void sh_tracking(char * argv);
static void sh_hours(char * argv);
static void sh_serial(char * argv);
static void sh_xmax(char * argv);
static void sh_xmin(char * argv);
static void sh_ymax(char * argv);
static void sh_ymin(char * argv);
static void sh_set_target(char * argv);
static void sh_follow_sun(char * argv);
static void sh_move_up(char * argv);
static void sh_move_down(char * argv);
static void sh_move_cw(char * argv);
static void sh_move_ccw(char * argv);
static void sh_save_sun(char * argv);
static void sh_save_target(char * argv);
static void sh_save_park(char * argv);
static void sh_pwmfreq(char *argv);
static void sh_ee_write(char * argv);
static void sh_ee_read(char * argv);
static void sh_scan(char *argv);
static void sh_phase(char *argv);
static void sh_desync(char * argv);
static void sh_dsp_init(char * argv);
static void sh_posx(char * argv);
static void sh_posy(char * argv);
static void sh_gps_debug(char * argv);
static void sh_targettime(char * argv);
static void sh_moonend(char * argv);

static void printlist(target_properties_t tg, uint8_t tgnr);

/* USER CODE END 1 */

// Internal Functions
static bool shell_strcmp(const char *s1, const char *s2);
static void shell_cmdline_init(cmdline_obj_t *object);
static void shell_cmdline_add(cmdline_obj_t *object);
static bool shell_next_argv(char ** p_buf);
static bool shell_getc(uint8_t *c);
#if (SHELL_CMDLINE_HIST_COUNT > 0)
static void shell_cmdline_get(cmdline_obj_t *object);
static void shell_cmdline_get_prev(cmdline_obj_t *object);
static void shell_cmdline_get_next(cmdline_obj_t *object);
#endif
static bool shell_cmdline_execute(cmdline_obj_t *object);
static bool shell_tabline_find(cmdline_obj_t *object);
static void shell_tabline_get(cmdline_obj_t *object);
#if LOGIN_ENABLE>0
static bool shell_cmdline_login(cmdline_obj_t *object);
#endif

// Private constants.
static const cmd_tbl_t cmd_tbl[] =
{
/* Default functions */
{ "help", "This tekst", sh_help },
{ "reboot", "reboot CPU", sh_reboot },
{ "ver", "Software version", sh_ver }, // declared global so main can use this for startup
#if LOGIN_ENABLE>0
		{	"login", "Login shell", sh_login},
		{	"logout", "Logout shell", sh_logout},
#endif
		/* USER CODE BEGIN 1 */

		{ "time", "Get RTC time", sh_time },
		{ "show", "argveters", sh_show },
		{ "list", "List Targets", sh_list },
		{ "factory", "Restore factory settings", sh_factory },
		{ "stepsx", "Steps per degrees horizontal", sh_stepsx },
		{ "stepsy", "Steps per degrees vertical", sh_stepsy },
		{ "hystx", "hysterese x", sh_hystx },
		{ "hysty", "hysterese y", sh_hysty },
		{ "kp", "Kp", sh_kp },
		{ "ki", "Ki", sh_ki },
		{ "kd", "Kd", sh_kd },
		{ "ktime", "Proces time(ms)", sh_pidt },
		{ "softstart", "PID softstart", sh_pids },
		{ "trackingtime", "Tracking interval", sh_tracking },

		{ "remote", "Save remote control", sh_remote },
		{ "debounce", "debounce end switch (ms)", sh_debounce },
		{ "sundown", "sundown angle", sh_sundown },
		{ "turnback", "turn back on switch", sh_turnback },

		{ "minpwm", "minimum pwm 0..100", sh_minpwm },
		{ "maxpwm", "maximum pwm 0..100", sh_maxwm },
		{ "windpulse", "windpulses per minute", sh_windpulse },

		{ "posx", "Set motor to position", sh_posx },
		{ "posy", "Set motor to position", sh_posy },

		// remote control copies:
		{ "stop", "Stop motors", sh_stop },
		{ "hang", "test", sh_hang },

		{ "eewrite", "test", sh_ee_write },
		{ "eeread", "test", sh_ee_read },
		{ "scan", "test", sh_scan },
		{ "pwmfreq", "set pwm", sh_pwmfreq },
		{ "phase", "measure phase", sh_phase },

		{ "settarget", "set target 1..16", sh_set_target },
		{ "followsun", "Follow sun", sh_follow_sun },
		 { "up", "up x steps", sh_move_up },
		 { "down", "down x steps", sh_move_down },
		 { "cw", "Clock wise x steps", sh_move_cw },
		 { "ccw", "Counter Clock wise x steps", sh_move_ccw },
		 { "savesun", "Save sun position", sh_save_sun },
		 { "savetarget", "Save target 1..16", sh_save_target },
		 { "savepark", "Save parkposition", sh_save_park },


		{ "calxmax", "calibrate X Max", sh_xmax },
		{ "calxmin", "calibrate X Min", sh_xmin },
		{ "calymax", "calibrate Y Max", sh_ymax },
		{ "calymin", "calibrate Y Min", sh_ymin },
		{ "targettime", "Target time", sh_targettime },
		{ "moonendtime", "stop time", sh_moonend },



		/*
		 * Test commands
		 */
		{ "inputs", "External inputs", sh_inputs },
		{ "gpsdebug", "Show GPS", sh_gps_debug },
		{ "serial", "Serial number", sh_serial },
		{ "hours", "Tracking hours", sh_hours },
		{ "contrast", "LCD contrast", sh_contrast },
		{ "desync", "Set GPS not sync", sh_desync },

		{ "dspinit", "Init display again", sh_dsp_init },

		/* USER CODE END 1 */

	};

#define CMD_TBL_COUNT   (sizeof(cmd_tbl) / sizeof(cmd_tbl[0]))

/*!
 * \brief	This function opens the shell using SCI1
 *
 * \Parameters	vars pointer to vars_t
 *
 * \return -.
 */
void shell_open(void)
{
	if (tmr == NO_TIMER)
	{
		tmr = timer_get();
	}

	shell_cmdline_init(&cmdline);


#if FLEXIBLE_SCI1 || FLEXIBLE_SCI2 || FLEXIBLE_SCI3 || FLEXIBLE_SCI4 || FLEXIBLE_SCI5 || FLEXIBLE_SCI6 || FLEXIBLE_SCI7 || FLEXIBLE_SCI8
	shell_use_sci1();
#endif
}


/*!
 * \brief This function closes the shell.
 *
 * \Parameters -.
 *
 * \return -.
 */
void shell_close(void)
{
	timer_free(&tmr);
}

/*!
 * \brief This function handles the shell.
 *
 * \Parameters -.
 *
 * \return -.
 */
void shell_process(void)
{
	uint8_t c;
	static uint8_t left_cursor = 0;

	if (shell_getc(&c))
	{
		switch (c)
		{
#if (SHELL_CMDLINE_HIST_COUNT > 0)
		case VK_UPARROW:
			left_cursor = 0;
#if LOGIN_ENABLE>0
			if (logged_in)
#endif
			{
				// Get the previous item from the history list.
				shell_cmdline_get_prev(&cmdline);

				// Handle cursor position.
				tty_puts("\x1B[2K"); // VT100 erase line.
				tty_putc('\r');
				tty_putsn(cmdline.p_cmdline->str, cmdline.p_cmdline->size);
			}
			break;

		case VK_DOWNARROW:
			left_cursor = 0;
#if LOGIN_ENABLE>0
			if (logged_in)
#endif
			{
				// Get the next item from the history list.
				shell_cmdline_get_next(&cmdline);

				// Handle cursor position.
				tty_puts("\x1B[2K"); // VT100 erase line.
				tty_putc('\r');
				tty_putsn(cmdline.p_cmdline->str, cmdline.p_cmdline->size);
			}
			break;
#endif
		case VK_BS:
			if (cmdline.p_cmdline->size)
			{
				// Copy tab line to command line.
				shell_tabline_get(&cmdline);

				// Handle cursor position.
				tty_puts("\x1B[D"); // VT100 cursor backward.
#if ADVANCED_ENABLE>0
				// cursor left is used print again from current position
				if (left_cursor > 0)
				{
					for (int var = cmdline.p_cmdline->size - left_cursor; var < cmdline.p_cmdline->size; ++var)
					{
						tty_putc(cmdline.p_cmdline->str[var]);
						// move the captured chars back
						cmdline.p_cmdline->str[var - 1] = cmdline.p_cmdline->str[var];
					}
				}
				tty_puts("\x1B[K"); // VT100 erase end of line.

				for (int var = cmdline.p_cmdline->size - left_cursor; var < cmdline.p_cmdline->size; ++var)
					tty_puts("\x1B[1D"); // VT100 cursor left pn times - stop at far left
#else
							tty_puts("\x1B[K"); // VT100 erase end of line.
#endif
				cmdline.p_cmdline->size--;
			}
			break;

#if ADVANCED_ENABLE>0
		case VK_DEL:
			if (cmdline.p_cmdline->size)
			{
				// Copy tab line to command line.
				shell_tabline_get(&cmdline);

				// cursor left is used print again from current position
				if (left_cursor > 0)
				{
					for (int var = cmdline.p_cmdline->size - left_cursor + 1; var < cmdline.p_cmdline->size; ++var)
					{
						tty_putc(cmdline.p_cmdline->str[var]);
						// move the captured chars back
						cmdline.p_cmdline->str[var - 1] = cmdline.p_cmdline->str[var];
					}
				}
				tty_puts("\x1B[K"); // VT100 erase end of line.

				for (int var = cmdline.p_cmdline->size - left_cursor + 1; var < cmdline.p_cmdline->size; ++var)
					tty_puts("\x1B[1D"); // VT100 cursor left pn times - stop at far left

				cmdline.p_cmdline->size--;
				left_cursor--;
			}
			break;

		case VK_LEFTARROW:
			// cursor to first char
			if (cmdline.p_cmdline->size - left_cursor > 0)
			{
				left_cursor++;
				tty_puts("\x1B[1D"); // VT100 cursor left pn times - stop at far left
			}
			break;

		case VK_RIGHTARROW:
			// cursor limit to end
			if (left_cursor > 0)
			{
				left_cursor--;
				tty_puts("\x1B[1C"); // VT100 cursor right pn times - stop at far right
			}
			break;

		case VK_HOME:
			// cursor to home
			for (int var = 0; var < cmdline.p_cmdline->size - left_cursor; ++var)
				tty_puts("\x1B[1D"); // VT100 cursor left pn times - stop at far left

			left_cursor = cmdline.p_cmdline->size;
			break;

		case VK_END:
			// cursor to end
			if (left_cursor > 0)
			{
				for (int var = cmdline.p_cmdline->size - left_cursor; var < cmdline.p_cmdline->size; ++var)
					tty_puts("\x1B[1C"); // VT100 cursor right pn times - stop at far right

				left_cursor = 0;
			}
			break;
#endif

		case VK_HT:
#if LOGIN_ENABLE>0
			if (logged_in)
#endif
		{
			if (cmdline.p_cmdline->size)
			{
				if (shell_tabline_find(&cmdline))
				{
					// Handle cursor position.
					tty_puts("\x1B[2K"); // VT100 erase line.
					tty_putc('\r');
					tty_putsn(cmdline.tabline.str, cmdline.tabline.size);
				}
			}
		}
			break;

		case VK_CR:
			left_cursor = 0;
#if LOGIN_ENABLE>0
			// Handle cursor position.
			if (logged_in)
#endif
			{
				tty_puts("\r\n");
				if (cmdline.p_cmdline->size)
				{
					//tty_printf("(s=%d)", cmdline.p_cmdline->size);
					// Copy tab line to command line.
					shell_tabline_get(&cmdline);
					cmdline.p_cmdline->str[cmdline.p_cmdline->size] = '\0';

					if (!shell_cmdline_execute(&cmdline))
						tty_printf("'%s' is not recognized as an internal command.\r\n", cmdline.p_cmdline->str);

					shell_cmdline_add(&cmdline);
				}
			}
#if LOGIN_ENABLE>0
			else
			{
				tty_puts("\r\nLogin: ");
				// Copy tab line to command line.
				shell_tabline_get(&cmdline);
				cmdline.p_cmdline->str[cmdline.p_cmdline->size] = '\0';

				if (!shell_cmdline_login(&cmdline))
				tty_printf("'%s' is not recognised as an internal command.\r\n", cmdline.p_cmdline->str);

				// do not store passwords
				cmdline.cmd_idx = 0;
				cmdline.p_cmdline->str[0] = 0;
				cmdline.p_cmdline->size = 0;
			}
#endif
			break;

		case VK_ESC:
			left_cursor = 0;
			if (cmdline.p_cmdline->size)
			{
				// Erase tab line.
				shell_tabline_get(&cmdline);

				// Handle cursor position.
				tty_puts("\x1B[2K"); // VT100 erase line.
				tty_putc('\r');

				cmdline.p_cmdline->size = 0;
			}
			break;

		default:
			if ((cmdline.p_cmdline->size < (SHELL_CMDLINE_SIZE - 1)) && (c >= 32) && (c < 127))
			{
				// Copy tab line to command line.
				shell_tabline_get(&cmdline);

				// Handle cursor position.
#if LOGIN_ENABLE>0
				if (logged_in)
				tty_putc(c);
				else
				tty_putc('*');
#else
				tty_putc(c);
#endif

#if ADVANCED_ENABLE==0
				// add the char to the input position
				cmdline.p_cmdline->str[cmdline.p_cmdline->size] = c;
#else
				// cursor left is used print again from current position
				if (left_cursor > 0)
				{
					cmdline.p_cmdline->size++;

					// push chars forward
					for (int var = cmdline.p_cmdline->size; var > cmdline.p_cmdline->size - left_cursor - 1; --var)
						cmdline.p_cmdline->str[var] = cmdline.p_cmdline->str[var - 1];

					// insert new char
					cmdline.p_cmdline->str[cmdline.p_cmdline->size - left_cursor - 1] = c;

					// print line
					for (int var = cmdline.p_cmdline->size - left_cursor; var < cmdline.p_cmdline->size; ++var)
						tty_putc(cmdline.p_cmdline->str[var]);

					// cursor to input position
					for (int var = cmdline.p_cmdline->size - left_cursor; var < cmdline.p_cmdline->size; ++var)
						tty_puts("\x1B[1D"); // VT100 cursor left pn times - stop at far left
				}
				else
					// add the char to the input position
					cmdline.p_cmdline->str[cmdline.p_cmdline->size - left_cursor] = c;

				if (left_cursor == 0)
#endif
					// cursor on the end add length
					cmdline.p_cmdline->size++;
			}
			break;
		}
	}
}

/*!
 * \brief This function reads and filters characters from the TTY port.
 *
 * \Parameters c Pointer to the character read.
 *
 * \return True if successful, false on error.
 *
 * \note The following virtual keys are supported:
 *
 *   1B 4F 50       : NUMLOCK
 *   1B 4F 51       : SLASH
 *   1B 4F 52       : MULTIPLY
 *   1B 4F 53       : MINUS
 *   1B 5B 31 7E    : HOME
 *   1B 5B 31 31 7E : F1
 *   1B 5B 31 32 7E : F2
 *   1B 5B 31 33 7E : F3
 *   1B 5B 31 34 7E : F4
 *   1B 5B 31 35 7E : F5
 *   1B 5B 31 37 7E : F6
 *   1B 5B 31 38 7E : F7
 *   1B 5B 31 39 7E : F8
 *   1B 5B 32 7E    : INS
 *   1B 5B 32 30 7E : F9
 *   1B 5B 32 31 7E : F10
 *   1B 5B 32 33 7E : F11
 *   1B 5B 32 34 7E : F12
 *   1B 5B 34 7E    : END
 *   1B 5B 35 7E    : PGUP
 *   1B 5B 36 7E    : PGDN
 *   1B 5B 41       : UPARROW
 *   1B 5B 42       : DOWNARROW
 *   1B 5B 43       : RIGHTARROW
 *   1B 5B 44       : LEFTARROW
 */
static bool shell_getc(uint8_t *c)
{
	static uint8_t state = 0;
	static uint8_t c_tmp;
	char c_raw;
	bool res = false;

	if (tty_getc(&c_raw))
	{
		switch (state)
		{
		case 0:
			switch (c_raw)
			{
			case 0x1B:
				timer_start(tmr, ESC_DELAY, NULL);
				state = 1;
				break;

			default:
				*c = c_raw;
				res = true;
				break;
			}
			break;

		case 1:
			switch (c_raw)
			{
			case 0x4F:
				state = 2;
				break;

			case 0x5B:
				state = 3;
				break;

			default:
				state = 0;
				break;
			}
			break;

		case 2:
			switch (c_raw)
			{
			case 0x50:
				*c = VK_NUMLOCK;
				res = true;
				state = 0;
				break;

			case 0x51:
				*c = VK_KP_SLASH;
				res = true;
				state = 0;
				break;

			case 0x52:
				*c = VK_KP_MULTIPLY;
				res = true;
				state = 0;
				break;

			case 0x53:
				*c = VK_KP_MINUS;
				res = true;
				state = 0;
				break;

			default:
				state = 0;
				break;
			}
			break;

		case 3:
			switch (c_raw)
			{
			case 0x31:
				c_tmp = VK_HOME;
				state = 4;
				break;

			case 0x32:
				c_tmp = VK_INS;
				state = 5;
				break;

			case 0x34:
				c_tmp = VK_END;
				state = 6;
				break;

			case 0x35:
				c_tmp = VK_PGUP;
				state = 6;
				break;

			case 0x36:
				c_tmp = VK_PGDN;
				state = 6;
				break;

			case 0x41:
				*c = VK_UPARROW;
				res = true;
				state = 0;
				break;

			case 0x42:
				*c = VK_DOWNARROW;
				res = true;
				state = 0;
				break;

			case 0x43:
				*c = VK_RIGHTARROW;
				res = true;
				state = 0;
				break;

			case 0x44:
				*c = VK_LEFTARROW;
				res = true;
				state = 0;
				break;

			default:
				state = 0;
				break;
			}
			break;

		case 4:
			switch (c_raw)
			{
			case 0x31:
				c_tmp = VK_F1;
				state = 6;
				break;

			case 0x32:
				c_tmp = VK_F2;
				state = 6;
				break;

			case 0x33:
				c_tmp = VK_F3;
				state = 6;
				break;

			case 0x34:
				c_tmp = VK_F4;
				state = 6;
				break;

			case 0x35:
				c_tmp = VK_F5;
				state = 6;
				break;

			case 0x37:
				c_tmp = VK_F6;
				state = 6;
				break;

			case 0x38:
				c_tmp = VK_F7;
				state = 6;
				break;

			case 0x39:
				c_tmp = VK_F8;
				state = 6;
				break;

			case 0x7E:
				*c = c_tmp;
				res = true;
				state = 0;
				break;

			default:
				state = 0;
				break;
			}
			break;

		case 5:
			switch (c_raw)
			{
			case 0x30:
				c_tmp = VK_F9;
				state = 6;
				break;

			case 0x31:
				c_tmp = VK_F10;
				state = 6;
				break;

			case 0x33:
				c_tmp = VK_F11;
				state = 6;
				break;

			case 0x34:
				c_tmp = VK_F12;
				state = 6;
				break;

			case 0x7E:
				*c = c_tmp;
				res = true;
				state = 0;
				break;

			default:
				state = 0;
				break;
			}
			break;

		case 6:
			switch (c_raw)
			{
			case 0x7E:
				*c = c_tmp;
				res = true;
				state = 0;
				break;

			default:
				state = 0;
				break;
			}
			break;
		}
	}
	else if (timer_elapsed(tmr) && (state == 1))
	{
		*c = VK_ESC;
		res = true;
		state = 0;
	}
	return (res);
}

/*!
 * \brief This function compares two strings.
 *
 * \Parameters s1 A pointer to a string.
 * \Parameters s2 A pointer to a string.
 *
 * \return True if successful, false on error.
 */
static bool shell_strcmp(const char *s1, const char *s2)
{
	// comparing until table ends
	for (; *s1 == *s2; s1++, s2++)
	{
		if (*s1 == '\0')
			return (true);
	}

	// MUR 22-5-2022 table must be ended and argveter follows
	if ((*s1 == 0) && ((*s2 == ' ') || (*s2 == '\f') || (*s2 == '\n') || (*s2 == '\r') || (*s2 == '\t') || (*s2 == '\v')))
			return (true);

	return (false);
}

/*!
 * \brief This function initializes the command line object.
 *
 * \Parameters object A pointer to a command line object.
 *
 * \return -.
 */
static void shell_cmdline_init(cmdline_obj_t *object)
{
	uint8_t i;

	for (i = 0; i < (SHELL_CMDLINE_HIST_COUNT + 1); i++)
	{
		object->cmdline[i].id = i;
		object->cmdline[i].str = &mem_pool[SHELL_CMDLINE_SIZE * i];
		object->cmdline[i].size = 0;

		if (object->cmdline[i].id == SHELL_CMDLINE_HIST_COUNT)
		{
			object->p_cmdline = &object->cmdline[i];
			object->p_cmdline->size = 0;
		}
	}
#if (SHELL_CMDLINE_HIST_COUNT > 0)
	object->cmd_idx = 0;
#endif
	object->tabline.str = NULL;
	object->tabline.size = 0;
	object->tab_idx = 0;
}

/*!
 * \brief This function adds an item to the command line object.
 *
 * \Parameters object A pointer to a command line object.
 *
 * \return -.
 */
static void shell_cmdline_add(cmdline_obj_t *object)
{
	uint8_t i;

	for (i = 0; i < (SHELL_CMDLINE_HIST_COUNT + 1); i++)
	{
		object->cmdline[i].id++;
		if (object->cmdline[i].id > SHELL_CMDLINE_HIST_COUNT)
			object->cmdline[i].id = 0;

		if (object->cmdline[i].id == SHELL_CMDLINE_HIST_COUNT)
		{
			object->p_cmdline = &object->cmdline[i];
			object->p_cmdline->size = 0;
		}
	}
#if (SHELL_CMDLINE_HIST_COUNT > 0)
	object->cmd_idx = 0;
#endif
}

#if (SHELL_CMDLINE_HIST_COUNT > 0)
/*!
 * \brief This function retrieves an item from the command line object.
 *
 * \Parameters object A pointer to a command line object.
 *
 * \return -.
 */
static void shell_cmdline_get(cmdline_obj_t *object)
{
	uint8_t i;

	for (i = 0; i < (SHELL_CMDLINE_HIST_COUNT + 1); i++)
	{
		if (object->cmdline[i].id == object->cmd_idx)
		{
			memcpy(object->p_cmdline->str, object->cmdline[i].str, object->cmdline[i].size);
			object->p_cmdline->size = object->cmdline[i].size;
			break;
		}
	}
}

/*!
 * \brief This function retrieves an item from the command line object.
 *
 * \Parameters object A pointer to a command line object.
 *
 * \return -.
 */
static void shell_cmdline_get_prev(cmdline_obj_t *object)
{
	shell_cmdline_get(object);
	if (!object->p_cmdline->size)
	{
		object->cmd_idx--;
		shell_cmdline_get(object);
	}

	if (object->cmd_idx < (SHELL_CMDLINE_HIST_COUNT - 1))
		object->cmd_idx++;

}

/*!
 * \brief This function retrieves an item from the command line object.
 *
 * \Parameters object A pointer to a command line object.
 *
 * \return -.
 */
static void shell_cmdline_get_next(cmdline_obj_t *object)
{
	if (object->p_cmdline->size)
	{
		if (object->cmd_idx > 0)
			object->cmd_idx--;

		shell_cmdline_get(object);
		if (!object->p_cmdline->size)
		{
			object->cmd_idx++;
			shell_cmdline_get(object);
		}
	}
}
#endif

static bool shell_cmdline_execute(cmdline_obj_t *object)
{
	uint8_t i;

	for (i = 0; i < CMD_TBL_COUNT; i++)
	{
		if (shell_strcmp(cmd_tbl[i].cmd, object->p_cmdline->str))
			break;
	}

	if (i < CMD_TBL_COUNT)
	{
		if (cmd_tbl[i].fxn)
		{
			char * p;
			p = strchr(cmdline.p_cmdline->str, ' ');
			
			// put pointer p to the end of the input
			if (p == NULL)
				p = cmdline.p_cmdline->str + cmdline.p_cmdline->size;
			
			cmd_tbl[i].fxn(p);
		}

		return (true);
	}

	return (false);
}

#if LOGIN_ENABLE>0
static bool shell_cmdline_login(cmdline_obj_t *object)
{
	uint8_t i;

	// when logged in search else command was login
	for (i = 0; i < CMD_TBL_COUNT; i++)
	{
		if (shell_strcmp(cmd_tbl[i].cmd, "login"))
		break;
	}

	if (i < CMD_TBL_COUNT)
	{
		if (cmd_tbl[i].fxn)
		{
			char * p;
			p = object->p_cmdline->str;
			cmd_tbl[i].fxn(p);
		}

		return (true);
	}

	return (false);
}

/*!
 * \brief
 *
 * \Parameters
 *
 * \return -.
 */
static void sh_login(char *argv)
{
	static uint8_t login_tmr = NO_TIMER;
	static uint8_t tries = 0;
	MD5_CTX mdContext;

	// release the login timer
	if (timer_elapsed(login_tmr))
	{
		timer_free(&login_tmr);
		tries = 0;
	}

	if (argv[0])
	{
        MD5Init(&mdContext);
        MD5Update(&mdContext, (uint8_t *)argv, strlen(argv));
        MD5Final(&mdContext);

		if (login_tmr == NO_TIMER)
		{
			if (md5compare(PASSWORD_1, mdContext.digest) || md5compare(PASSWORD_2, mdContext.digest))
			{
				tries = 0;
				logged_in = true;
				tty_printf("\r\nWelcome to MarXact\r\n");
			}
			else
			{
				tty_printf("Error login\r\n");
				if (tries++ == 4)
				{
					login_tmr = timer_get();
					timer_start(login_tmr, 5000, NULL);
				}
			}
		}
		else
		{
			timer_start(login_tmr, 5000, NULL);
			tty_printf("Please wait..\r\n");
		}
	}
}

/*!
 * \brief
 *
 * \Parameters
 *
 * \return -.
 */
static void sh_logout(char *argv)
{
	logged_in = false;
}

/*!
 * \brief Compares string array with const array
 *
 * \Parameters
 *
 * \return - TRUE when match
 */
static bool md5compare(char * pwd, uint8_t * arry)
{
    char a[2];

     for (int i = 0; i < MD5_DIGEST_SIZE; i++)
     {
         a[0] = pwd[i*2];
         a[1] = pwd[i*2+1];

         if (strtol(a, 0, 16) != arry[i])
        	 return false;
     }
    return true;
}
#endif

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static bool shell_tabline_find(cmdline_obj_t *object)
{
	uint8_t i;

	for (object->tabline.str = NULL, i = 0; (object->tabline.str == NULL) && (i < CMD_TBL_COUNT); i++)
	{
		if (!strncmp(cmd_tbl[object->tab_idx].cmd, object->p_cmdline->str, object->p_cmdline->size))
		{
			object->tabline.str = (char *) cmd_tbl[object->tab_idx].cmd;
			object->tabline.size = strlen(object->tabline.str);
		}

		object->tab_idx++;
		if (object->tab_idx >= CMD_TBL_COUNT)
			object->tab_idx = 0;
	}

	return (object->tabline.str != NULL);
}

/*!
 * \brief This function copies the last tab line to the command line.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void shell_tabline_get(cmdline_obj_t *object)
{
	if (object->tabline.str)
	{
		memcpy(object->p_cmdline->str, object->tabline.str, object->tabline.size);
		object->p_cmdline->size = object->tabline.size;

		object->tabline.str = NULL;
	}
	object->tab_idx = 0;
}

/*!
 * \brief Browse pointer to next argveter
 *
 * \Parameters
 *
 * \return -.
 */
static bool shell_next_argv(char ** p_buf)
{
		// strchr did not found a space
	if ((*p_buf) == 0)
		return false;
	
	// end of line
	if (*(*p_buf) == 0)
		return false;

	// browse to separator (space, dot, semicolum)
	while (*(*p_buf) != 0 && *(*p_buf) != ' ' && *(*p_buf) != '.' && *(*p_buf) != ':')
		(*p_buf)++;

	// another separator?
	while ((*(*p_buf) == ' ' || (*(*p_buf) == '.') || (*(*p_buf) == ':')) && (*(*p_buf) != 0))
		(*p_buf)++;

	// next character must be available
	if (*(*(p_buf)) != 0)
		return true;

	return false;
}

/*
 * ---------------------------------- Default Shell functions ----------------------------------
 */

/*!
 * \brief This function handles the shell help command.
 *
 * \Parameters -.
 *
 * \return -.
 */
static void sh_help(char *argv)
{
	uint8_t i;

	tty_printf("Available commands:\r\n");
	for (i = 0; i < CMD_TBL_COUNT; i++)
		tty_printf("%-15s   %s\r\n", cmd_tbl[i].cmd, cmd_tbl[i].desc);
}

/*!
 * \brief
 *
 * \Parameters
 *
 * \return -.
 */
static void sh_reboot(char *argv)
{
	tty_printf("NVICReset\r\n");
	HAL_Delay(10);
	NVIC_SystemReset();
}


/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_time(char * argv)
{
	time_date_t dt;

	if (shell_next_argv(&argv))
	{
		override_gps();
		vars.gps_decode = DECODING_RDY;
		dt.Epoch = strtol(argv, 0, 10);
		tty_printf("Set %d\r\n", dt.Epoch);
		epoch_to_datetime(&dt);
		set_rtc(dt);
	}

	rtc_get(&dt);
	tty_printf("GET %02d:%02d:%02d ", dt.Hours, dt.Minutes, dt.Seconds);
	tty_printf("GET %02d-%02d-%02d\r\n", dt.Date, dt.Month, dt.Year);

	//tty_printf("WeekDay %d\r\n", dt.WeekDay);
	//tty_printf("Epoch %d\r\n", dt.Epoch);

	tty_printf("%s sync: %d%%\r\n", vars.gps_system == SYS_GPS ? "GPS" : "GLONASS", vars.gps_decode);

}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_show(char * argv)
{
	float x, y;
	tty_printf("\r\nNow-----------------------------------------\r\n");
	tty_printf(" hours      = %d.%02d boots: %d\r\n", vars.eevar.tracking_minutes / 60, vars.eevar.tracking_minutes % 60, vars.eevar.bootcounter);
	tty_printf(" GPS receiver %s valid for %d.%02dh\r\n", isGPS_ON ? "ON" : "OFF", gps_remain_valid() / 60, gps_remain_valid() % 60);
	tty_printf(" Location latitude  %3.3f' longitude %3.3f'\r\n", vars.hwinfo.home_location.latitude, vars.hwinfo.home_location.longitude);
	tty_printf(" Sun azimuth        %3.3f' elevation %3.3f'\r\n", vars.sunpos.azimuth, vars.sunpos.elevation);
	if (vars.hwinfo.moonend_mod != MOON_OFF)
		tty_printf(" Moon azimuth       %3.3f' elevation %3.3f'\r\n", vars.moonpos.azimuth, vars.moonpos.elevation);

	x = (float) (vars.eevar.actual_motor.x + vars.hwinfo.hw_offset.x) / vars.hwinfo.steps.x;
	if (x > 360)
		x = x - 360;
	y = ((float) ((vars.eevar.actual_motor.y + vars.hwinfo.hw_offset.y) / vars.hwinfo.steps.y));
#if 1

	tty_printf(" Mirror x= %d calc= %d at %3.3f'\r\n", vars.eevar.actual_motor.x, vars.goto_motor.x, x);
	tty_printf(" Mirror y= %d calc= %d at %3.3f'\r\n", vars.eevar.actual_motor.y, vars.goto_motor.y, y);
	tty_printf(" Target Azimuth = %3.3f' Elevation = %3.3f'\r\n", ((float) vars.eevar.target.x / vars.hwinfo.steps.x), ((float) vars.eevar.target.y / vars.hwinfo.steps.y));

	tty_printf("Hardware------------------------------------\r\n");
	tty_printf(" serial number        = %u\r\n", stm_serial());
	tty_printf(" maximum          x,y = %d,%d\r\n", vars.hwinfo.maximum.x, vars.hwinfo.maximum.y);
	tty_printf(" Hardware_atimuth     = %d\r\n", vars.hwinfo.hw_offset.x);
	tty_printf(" Hardware_elevation   = %d\r\n", vars.hwinfo.hw_offset.y);
	tty_printf(" Park position    x,y = %d,%d\r\n", vars.hwinfo.parkposition.x, vars.hwinfo.parkposition.y);
	tty_printf(" Sun down angle       = %d\r\n", vars.hwinfo.sun_down_angle);
	//tty_printf(" Time sync            = %u\r\n", time_sync);
	//tty_printf(" Wind pulses          = %lu new value in %u sec.\r\n", memwind, 60 - local_sec);

	tty_printf("User settings-------------------------------\r\n");
	tty_printf(" Tracking interval    = %d sec\r\n", vars.hwinfo.track_interval);

//	tty_printf(" Nightparking enabled = %u\r\n", sundownparking_enabled);
//	tty_printf(" Hourmode             = %u\r\n", hourmode);
	if (vars.hwinfo.ayct102_home_1)
		tty_printf(" Remote control a:    = %d\r\n", vars.hwinfo.ayct102_home_1);
	else
		tty_printf(" No Remote control A\r\n");

	if (vars.hwinfo.ayct102_home_2)
		tty_printf(" Remote control b:    = %d\r\n", vars.hwinfo.ayct102_home_2);

	if (vars.hwinfo.max_windpulse != 0)
		tty_printf(" MaxWind              = %d\r\n", vars.hwinfo.max_windpulse);
	else
		tty_printf(" MaxWind              = OFF\r\n");
	if (vars.hwinfo.moonend_mod == MOON_OFF)
		tty_printf(" Moon tracking        disabeled\r\n");
	else
		tty_printf(" Moon end time        %d:%02d (UTC)\r\n", vars.hwinfo.moonend_mod / 60, vars.hwinfo.moonend_mod % 60);

	tty_printf("Motor---------------------------------------\r\n");
	tty_printf(" Step per decrees x,y = %d,%d\r\n", vars.hwinfo.steps.x, vars.hwinfo.steps.y);
	tty_printf(" Pwm frequency        = %d Hz\r\n", vars.hwinfo.pwmfreq);
	tty_printf(" min pwm              = %d (0..100)\r\n", vars.hwinfo.min_pwm);
	tty_printf(" max pwm              = %d (%d..100)\r\n", vars.hwinfo.max_pwm, vars.hwinfo.min_pwm);
	tty_printf(" hysteresis       x,y = %d,%d\r\n", vars.hwinfo.hysteresis.x, vars.hwinfo.hysteresis.y);
	tty_printf(" zero deviation   x,y = %d,%d\r\n", vars.deviation.x, vars.deviation.y);
#endif
	tty_printf("PID-----------------------------------------\r\n");
	tty_printf(" process time         = %d ms\r\n", vars.hwinfo.pid.repeat_ms);
	tty_printf(" Kp                   = %d.%03d\r\n", vars.hwinfo.pid.p / 1000, vars.hwinfo.pid.p % 1000);
	tty_printf(" Ki                   = %d ms\r\n", vars.hwinfo.pid.i);
	tty_printf(" Kd                   = %d\r\n", vars.hwinfo.pid.d);
	tty_printf(" softstart            = %d ms\r\n", vars.hwinfo.pid.softstart);
	tty_printf(" end switch       x,y = %s,%s\r\n", isEND_X ? "YES" : "NO", isEND_Y ? "YES" : "NO");
//	tty_printf("Switch mid x:%s\r\n", isMID_X ? "YES" : "NO");
//	tty_printf("Switch mid y:%s\r\n", isMID_Y ? "YES" : "NO");
	tty_printf("--------------------------------------------\r\n");
	tty_printf(" Mode %d state [%s]\r\n", vars.eevar.main_mode, print_mode_name(vars.eevar.main_mode));
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_inputs(char * argv)
{
	tty_printf("Adapter voltage = %d.%dV\r\n", read_adapter() / 1000, (read_adapter() % 1000) / 10);
	tty_printf("External input 1= %d\r\n", isEXT_IN_1);

	tty_printf("Motor X Fault   = %d\r\n", isFAULT_1);
	tty_printf("Motor Y Fault   = %d\r\n", isFAULT_2);

	tty_printf("used_timers     = %d\r\n", used_timers());

}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_list(char * argv)
{
	for (int var = 0; var < NR_OF_TARGETS; ++var)
	{
		printlist(vars.hwinfo.target[var], var);
	}
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void printlist(target_properties_t tg, uint8_t tgnr)
{
	if (tg.pos.x + tg.pos.y)
	{
		tty_printf("Target %2d Azimuth = %3.3f' Elevation = %3.3f'", tgnr + 1, ((float) tg.pos.x / vars.hwinfo.steps.x), ((float) tg.pos.y / vars.hwinfo.steps.y));
		if (tg.mode != TIMED_OFF)
		{
			uint16_t mod = get_dst_correction(tg.mod, tg.mode);
			tty_printf(" Activation time %d:%02d Summertime: %s", mod / 60, mod % 60, prn_summer[tg.mode]);
		}

		tty_printf("\r\n");
	}
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_factory(char * argv)
{
	tty_printf("Restore factory settings\r\n");
	factory(&vars.hwinfo);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_stepsx(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 10000)
		{
			vars.hwinfo.steps.x = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..10000");
	}
	tty_printf("Steps per degrees X %d\r\n", vars.hwinfo.steps.x);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_stepsy(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 10000)
		{
			vars.hwinfo.steps.y = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..10000");
	}
	tty_printf("Steps per degrees Y %d\r\n", vars.hwinfo.steps.y);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_hystx(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 10000)
		{
			vars.hwinfo.hysteresis.x = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..10000");
	}
	tty_printf("Hysteresis X %d\r\n", vars.hwinfo.hysteresis.x);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_hysty(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 100)
		{
			vars.hwinfo.hysteresis.y = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..100");
	}
	tty_printf("Hysteresis Y %d\r\n", vars.hwinfo.hysteresis.y);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_kp(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 100001)
		{
			vars.hwinfo.pid.p = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..100000\r\n");
	}
	tty_printf("Kp %d.%03d\r\n", vars.hwinfo.pid.p / 1000, vars.hwinfo.pid.p % 1000);
}

static uint8_t gps_debug_tmr = NO_TIMER;

static void f_gps(void)
{
	GPS_1;
	vars.gps_debug = true;
	timer_free(&gps_debug_tmr);
	tty_printf("GPS debug rst-on\r\n");

}


/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_moonend(char * argv)
{
	uint16_t hour;
	uint16_t min;

	if (shell_next_argv(&argv))
	{
		hour = strtol(argv, 0, 10);

		if (strncmp(argv, "off", 3) == 0)
		{
			vars.hwinfo.moonend_mod = MOON_OFF;
			tty_printf("Set moon tracking off\r\n");
			WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
			return;
		}
		if ((shell_next_argv(&argv)) && hour < 24)
		{
			min = strtol(argv, 0, 10);
			if (min < 60)
			{
				tty_printf("Set moon end time to %d:%02d (UTC)\r\n", hour, min);
				vars.hwinfo.moonend_mod = (hour * 60) + min;
				WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
			}
			else
				tty_printf("Use Moonend xx:xx or 'off'\r\n");
		}
		else
			tty_printf("Use Moonend xx:xx or 'off'\r\n");
	}
	else
		tty_printf("Moon end time %d:%02d (UTC)\r\n", vars.hwinfo.moonend_mod / 60, vars.hwinfo.moonend_mod % 60);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_targettime(char * argv)
{
	uint16_t tg;
	uint16_t hour;
	uint16_t min;
	pmode_t mode = TIMED_EU_SUMMER;

	if (shell_next_argv(&argv))
	{
		tg = strtol(argv, 0, 10);
		if (shell_next_argv(&argv))
		{
			hour = strtol(argv, 0, 10);
			if (shell_next_argv(&argv))
			{
				min = strtol(argv, 0, 10);

				if (shell_next_argv(&argv))
				{
					if (strncmp(argv, "eu", 2) == 0)
						mode = TIMED_EU_SUMMER;
					else if (strncmp(argv, "us", 2) == 0)
						mode = TIMED_US_SUMMER;
					else if (strncmp(argv, "none", 4) == 0)
						mode = TIMED_ON;
					else if (strncmp(argv, "off", 3) == 0)
						mode = TIMED_OFF;
				}

				if ((mode == TIMED_OFF) && (tg > 0)  && (tg <= 16))
				{
					tty_printf("%d %d\r\n", vars.hwinfo.target[tg-1].pos.x, vars.hwinfo.target[tg-1].pos.y);
					tty_printf("Setting target %d timed mode off\r\n", tg);
					vars.hwinfo.target[tg-1].mode = TIMED_OFF;
					WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
				}
				else if ((tg > 0) && (tg < 17) && (hour < 24) && (min < 60))
				{
					if (vars.hwinfo.target[tg-1].pos.x != 0)
					{
						tty_printf("setting target %d on %d:%02d summertime %s\r\n", tg, hour, min, prn_summer[mode]);
						vars.hwinfo.target[tg-1].mod = set_dst_correction(hour * 60 + min, mode);

						vars.hwinfo.target[tg-1].mode = mode;
						WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
					}
					else
						tty_printf("Error Target %d has not been set\r\n", tg);
				}
				else
					tty_printf("Invalid argument target %d on %d:%02d\r\n", tg, hour, min);
			}
			else
				tty_printf("example: targettime 1 14:00 eu\r\n");
		}
		else
			tty_printf("example: targettime 1 14:00 eu\r\n");
	}
	else
	{
		tty_printf("targettime [target] 1..16 [time] 12:00 [summer] off/none/eu/us\r\n");
		tty_printf("use 'list' for available targets\r\n");
	}
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_gps_debug(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val == 1)
		{
			vars.gps_debug = true;
			tty_printf("GPS debug on\r\n");
		}
		else if (val == 2)
		{
			GPS_0;
			gps_debug_tmr = timer_get();
			timer_start(gps_debug_tmr, 1000, f_gps);
		}
		else
		{
			vars.gps_debug = false;
			tty_printf("GPS debug off\r\n");
		}
	}
	else
		tty_printf("GPS debug 0:off, 1:on 2:reset on\r\n");
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_ki(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 1001)
		{
			vars.hwinfo.pid.i = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..1000");
	}
	tty_printf("Ki %d ms\r\n", vars.hwinfo.pid.i);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_kd(char * argv)
{
	tty_printf("Kd not implemented\r\n");
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_pidt(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 1000)
		{
			vars.hwinfo.pid.repeat_ms = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..1000");
	}
	tty_printf("PID Time %d ms\r\n", vars.hwinfo.pid.repeat_ms);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_pids(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0 && val < 5001)
		{
			vars.hwinfo.pid.softstart = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 1..5000ms");
	}
	tty_printf("Softstart %d ms\r\n", vars.hwinfo.pid.softstart);
}


/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_turnback(char * argv)
{
	uint16_t val;
	uint16_t decimal = 0;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);

		if (shell_next_argv(&argv))
			decimal = strtol(argv, 0, 10);

		if ((val > 0) && (val < 6) && (decimal < 10))
		{
			vars.hwinfo.turnback = (val * 10) + decimal;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 1.0 - 5.0\r\n");
	}
	tty_printf("Turning back %d.%d'\r\n", vars.hwinfo.turnback / 10, vars.hwinfo.turnback % 10);
}


/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_debounce(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 5001)
		{
			vars.hwinfo.debounce = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..5000");
	}
	tty_printf("Debounce endswitch %d ms\r\n", vars.hwinfo.debounce);
}



/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_minpwm(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val <= 100)
		{
			vars.hwinfo.min_pwm = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..100");
	}
	tty_printf("Minimum pwm %d\r\n", vars.hwinfo.min_pwm);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_sundown(char * argv)
{
	int16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if ((val < 180) && (val > 0))
		{
			vars.hwinfo.sun_down_angle = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range -180..180");
	}
	tty_printf("Sundown angle %dº\r\n", vars.hwinfo.sun_down_angle);
}


/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_maxwm(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val <= 100)
		{
			vars.hwinfo.max_pwm = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 0..100");
	}
	tty_printf("Maximum pwm %d\r\n", vars.hwinfo.max_pwm);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_windpulse(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if ((val < 10000) && (val > 100))
		{
			vars.hwinfo.max_windpulse = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 100..10000");
	}
	tty_printf("Maximum windpulse %d per minute\r\n", vars.hwinfo.max_windpulse);

	tty_printf("Counted %d pulses/ minute. Next value in %d seconds\r\n", vars.wind_ppm, get_wind_counter() / 1000);


}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_contrast(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val < 100)
		{
			vars.hwinfo.contrast = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
			set_contrast(vars.hwinfo.contrast);
		}
		else
			tty_printf("argveter out of range 0..100");
	}
	tty_printf("Contrast %d\r\n", vars.hwinfo.contrast);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_tracking(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0 && val < 10000)
		{
			vars.hwinfo.track_interval = val;
			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
		else
			tty_printf("argveter out of range 1..10000");
	}
	tty_printf("Tracking interval %d sec\r\n", vars.hwinfo.track_interval);
}




/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_hours(char *argv)
{
	uint32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val == 376815086)
		{
			vars.eevar.bootcounter = 0;
			vars.eevar.tracking_minutes = 0;
			tty_printf("Reset tracking time\r\n");

			if (WriteStruct2eerom(vars.eevar))
				tty_printf("  Error write eeprom\r\n");
		}
	}
	tty_printf("Tracking hours = %d.%02d h\r\n", vars.eevar.tracking_minutes / 60, vars.eevar.tracking_minutes % 60);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_pwmfreq(char *argv)
{
	uint32_t val;



	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 400)
		{
			vars.hwinfo.pwmfreq = val;

			tty_printf("Set div %d\r\n", APB_CLK / val / 200);

			if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
				tty_printf("Error saving flash");
		}
	}
	tty_printf("Pwm frequency = %d Hz\r\n", vars.hwinfo.pwmfreq);
}



static void sh_scan(char *argv)
{
	for (int var = 0; var < 0xFF; var += 2)
	{
		if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t) var, 1, 100) == HAL_OK)
			tty_printf("I2C2 0x%02X %d\r\n", var, var);
	}
}

static uint8_t result_tmr = NO_TIMER;

void f_result(void)
{
	timer_free(&result_tmr);
	tty_printf("FreqX %d\r\n", vars.phase_x.frequency);
	tty_printf("dutya %d\r\n", vars.phase_x.duty_a);
	tty_printf("dutyb %d\r\n", vars.phase_x.duty_b);
	tty_printf("shift %d\r\n", vars.phase_x.shift);

	tty_printf("FreqY %d\r\n", vars.phase_y.frequency);
	tty_printf("dutya %d\r\n", vars.phase_y.duty_a);
	tty_printf("dutyb %d\r\n", vars.phase_y.duty_b);
	tty_printf("shift %d\r\n", vars.phase_y.shift);
}

static void sh_phase(char *argv)
{
	tty_printf("Measure phase\r\n");
	memset(&vars.phase_x, 0x00, sizeof(vars.phase_x));
	memset(&vars.phase_y, 0x00, sizeof(vars.phase_y));

	vars.phase_x.stage = 1;
	vars.phase_y.stage = 1;

	if (result_tmr == NO_TIMER)
		result_tmr = timer_get();

	timer_start(result_tmr, 500, f_result);
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_serial(char * argv)
{
	tty_printf("Serial number %u\r\n", stm_serial());
	tty_printf("Flash check   %s\r\n", stm_serial() == vars.hwinfo.stm_serial ? "OK" : "FAIL");
	tty_printf("Table check   %s\r\n", check_quick() ? "OK" : "FAIL");
}

/*!
 * \brief This function finds a (partial) command in a table.
 *
 * \Parameters object A pointer to a tab line object.
 *
 * \return -.
 */
static void sh_remote(char * argv)
{
	uint16_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val == 16)
		{
			if (vars.lastrx_ayct102_home)
			{
				vars.hwinfo.ayct102_home_1 = vars.lastrx_ayct102_home;
				tty_printf("Saving remote a:%d\r\n", vars.hwinfo.ayct102_home_1);

				if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
					tty_printf("Error saving flash");
			}
			else
				tty_printf("No remote received\r\n");
		}
		if (val == 17)
		{
			if (vars.lastrx_ayct102_home)
			{
				vars.hwinfo.ayct102_home_2 = vars.lastrx_ayct102_home;
				tty_printf("Saving remote b:%d\r\n", vars.hwinfo.ayct102_home_2);

				if (WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t)))
					tty_printf("Error saving flash");
			}
			else
				tty_printf("No remote received\r\n");
		}

	}
	else
		tty_printf("Current Remote a:%d b:%d, Last received %d [save: remote 16/17]\r\n", vars.hwinfo.ayct102_home_1, vars.hwinfo.ayct102_home_2, vars.lastrx_ayct102_home);
}

static void sh_posx(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		vars.goto_motor.x = val;
		vars.eevar.main_mode = ST_TRACK_MANUAL;
		tty_printf("GotoX %d\r\n", vars.goto_motor.x);
	}
	else
		tty_printf("Pos X = %d\r\n", vars.goto_motor.x);

}

static void sh_posy(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		vars.goto_motor.y = val;
		vars.eevar.main_mode = ST_TRACK_MANUAL;
		tty_printf("GotoY %d\r\n", vars.goto_motor.y);
	}
	else
		tty_printf("Pos Y = %d\r\n", vars.goto_motor.y);

}

static void sh_stop(char * argv)
{
	vars.eevar.main_mode = ST_STOP;
	vars.goto_motor = vars.eevar.actual_motor;

	tty_printf("STOP %d,%d\r\n", vars.eevar.actual_motor.x, vars.eevar.actual_motor.y);

}

static void sh_hang(char * argv)
{
	tty_printf("WDT test\r\n");
	while (1)
		;

}



static void sh_xmax(char * argv)
{
	// Calibrate X Clockwise
	cmd_set_end_cw();
}

static void sh_xmin(char * argv)
{
	// Calibrate X Counter Clockwise
	cmd_set_end_ccw();
}

static void sh_ymax(char * argv)
{
	// Calibrate Y up
	cmd_set_end_up();

}

static void sh_ymin(char * argv)
{
	// Calibrate Y down
	cmd_set_end_down();
}

static void sh_follow_sun(char * argv)
{
	time_stamp();
	tty_printf("Target SUN\r\n");
	vars.eevar.main_mode = ST_TRACK_SUN;

	// so tracking starts immidiate
	timer_free(&vars.tracking_tmr);
	timer_free(&vars.calc_sun_tmr);
}

static void sh_set_target(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val <= 16 && val > 0)
		{
			time_stamp();
			tty_printf("Target %d\r\n", val);
			vars.eevar.target = vars.hwinfo.target[val - 1].pos;
			vars.eevar.main_mode = val + 1;

			// so tracking starts immidiate
			timer_free(&vars.tracking_tmr);
			timer_free(&vars.calc_sun_tmr);
		}
		else
			tty_printf("Invalid argveter 1..16\r\n");
	}
	else
		tty_printf("settarget 1..16\r\n");
}


static void sh_move_up(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0)
		{
			cmd_move_up(val);
		}
	}
}

static void sh_move_down(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0)
		{
			cmd_move_down(val);
		}
	}
}

static void sh_move_cw(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0)
		{
			cmd_move_cw(val);
		}
	}
}

static void sh_move_ccw(char * argv)
{
	int32_t val;

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);
		if (val > 0)
		{
			cmd_move_ccw(val);
		}
	}
}

static void sh_save_sun(char * argv)
{
	cmd_savesun();
}

static void sh_save_target(char * argv)
{
	int32_t tg;

	if (shell_next_argv(&argv))
	{
		tg = strtol(argv, 0, 10);
		if (tg <= 16 && tg > 0)
		{
			cmd_savetarget(tg);
		}
	}
	else
		tty_printf("Invalid argveter 1..16\r\n");
}

static void sh_ee_write(char * argv)
{
	int32_t val = 0;
	uint8_t data[254];
	uint8_t block = 0;

	if (shell_next_argv(&argv))
	{
		block = strtol(argv, 0, 16);

		if (shell_next_argv(&argv))
			val = strtol(argv, 0, 16);

		memset(data, val, sizeof(data));

		tty_printf("addr 0x%02X result:%d\r\n", block, e2p_write(block, data, 10));
	}
}

static void sh_ee_read(char * argv)
{
	int32_t val;
	uint8_t data[254];

	if (shell_next_argv(&argv))
	{
		val = strtol(argv, 0, 10);

		memset(data, 0x00, sizeof(data));

		tty_printf("read block %d result %d\r\n", val, e2p_read(0, data, sizeof(data)));

		for (int var = 0; var < sizeof(data); ++var)
		{
			tty_printf("0x%02X ", data[var]);
		}
		tty_printf("\r\n");

	}
}


static void sh_save_park(char * argv)
{
	// save parkposition
	cmd_savepark();
}



static void sh_desync(char * argv)
{
	tty_printf("GPS receiver on\r\n");
	gps_power(true);
//	GPS_1;
	HAL_Delay(20);
	MX_USART2_UART_Init();
}

/*!
 * \brief This function handles the shell help command.
 *
 * \Parameters -.
 *
 * \return -.
 */
void sh_ver(char *argv)
{
	extern app_info_t c_app_info;

	uint32_t *Bootloader_info = (uint32_t*) (0x08005000 - 40);
	app_info_t *p_bl_info = (app_info_t*) Bootloader_info;

	tty_printf("Bootloader %X.%02X\r\n", ((p_bl_info->version >> 24) & 0xFF), ((p_bl_info->version >> 16) & 0xFF));

#ifdef ENABLE_MODBUS
	tty_printf("Modbus ");
#endif
	tty_printf("Suntrack %lx.%02lx (%s %s)\r\n", ((c_app_info.version >> 24) & 0xFF), ((c_app_info.version >> 16) & 0xFF), c_app_info.build_date, c_app_info.build_time);



}

static void sh_dsp_init(char * argv)
{
	dsp_init();
}


