/*
 * protection.c
 *
 *  Created on: 16 sep. 2019
 *      Author: Mark
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "vars.h"
#include "crc.h"
#include "e2p.h"
#include "protection.h"
#include "flash_ee.h"
#include "uart_sci.h"


/*
 * Number of valid serials
 */
static const uint32_t romtable[] =
{
		685527807, // G0B1 #1
		2004982734,
		3166523955,
		660882292,
		233548783

		 };




/*!
 * \brief SET the protection in FLASH and shows the number for the ROM table
 *
 * \param
 *
 * \return - true if OK
 */
void set_protection(void)
{
	hw_info_t flash;

	ReadStruct2Flash(&flash, sizeof(hw_info_t));

	flash.stm_serial = stm_serial();

	tty_printf("<<<<<<<<<<Set protection NO USER PROGRAM>>>>>>>>>>>>>\r\n");
	tty_printf("Serial number [%u] PUT ROM THIS IN ROMTABLE\r\n", flash.stm_serial);
	tty_printf("<<<<<<<<<<Set protection NO USER PROGRAM>>>>>>>>>>>>>\r\n");

	WriteStruct2Flash(&flash, sizeof(hw_info_t));
}

/*!
 * \brief Compares the STM32 serial CRC to the ROM table
 *
 * \param
 *
 * \return - true if OK
 */
bool check_quick(void)
{
	uint32_t serial = stm_serial();

	for (int var = 0; var < sizeof(romtable) / 4; ++var)
	{
		if (romtable[var] == serial)
			return true;
	}
	return false;
}


/*!
 * \brief Get user shown serial number
 *
 * \param
 *
 * \return - CRC32 of STM serial
 */
uint32_t stm_serial(void)
{
	uint32_t challenge[3];

	uint32_t *p_mem;
	p_mem = (uint32_t*) UID_BASE;

	for (int var = 0; var < 3; var++)
		challenge[var] = *(p_mem + var);

	return HAL_CRC_Calculate(&hcrc, challenge, sizeof(challenge) / 4);
}

