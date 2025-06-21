/*!
 * @file	e2p.c
 * @brief	This software module provides functions for accessing a 24AA02E48 i2c EEPROM.
 * @author	Mark Ursum
 * @date	Created on: 5 apr. 2017
 *
 */


#include "main.h"
#include "e2p.h"
#include "crc.h"
#include "i2c.h"
#include "uart_sci.h"

#if 1

// Private function prototypes.
static bool m24xx64_ready(void);
//static bool e2p_read(uint8_t address, uint8_t *p_dst, uint8_t len);
//static bool e2p_write(uint8_t address, uint8_t *p_src, uint8_t len);

//extern I2C_HandleTypeDef hi2c2;
//extern CRC_HandleTypeDef hcrc;

/*!
 * \brief This function reads the 48-bit node address from the 24AA02E48 EEPROM.
 *
 * \param p_dst Pointer to the buffer where the 48-bit node address is stored (6 bytes).
 *              The buffer contains:
 *              uint8_t node_addr[0]
 *              uint8_t node_addr[1]
 *              uint8_t node_addr[2]
 *              uint8_t node_addr[3]
 *              uint8_t node_addr[4]
 *              uint8_t node_addr[5]
 *
 * \return -. true on error.
 */
bool e2p_get_serial_no(uint8_t *p_dst)
{
	uint8_t address = E2P_NODE_ADDRESS;

	if (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t) E2P_RIIC_ADDRESS, &address, 1, 2000) != HAL_OK)
		return true;
	if (HAL_I2C_Master_Receive(&hi2c2, (uint16_t) E2P_RIIC_ADDRESS, p_dst, 6, 1000) != HAL_OK)
		return true;

	return false;
}

/*!
 * \brief This function writes the struct to I2C eerom save on address 0x00 and duplicate 0x40.
 * 		  a CRC is caluclated and written to the datastructure
 *
 * \param struct to write
 *
 * \return false if successful, true on error.
 */
uint8_t WriteStruct2eerom(i2c_ee_t to_save)
{

	bool result;

	/*##-2- Compute the CRC of "aDataBuffer" ###################################*/
	to_save.crc = HAL_CRC_Calculate(&hcrc, (uint32_t *) &to_save, (sizeof(to_save)/4) - 4); //

	result = e2p_write(0x00, (uint8_t *) &to_save, sizeof(to_save));
	result |= e2p_write(0x40,(uint8_t *) &to_save, sizeof(to_save));

	return result;
}


/**
 * @brief This function read the struct from I2C. If the CRC if original fails the backup is used.
 * @param to_read pointer to i2c_ee_t
 * @return 0 if successful, true on CRC error.
 */
uint8_t ReadStruct2eerom(i2c_ee_t * to_read)
{
	uint8_t array[sizeof(i2c_ee_t)];

	e2p_read(0, array, sizeof(i2c_ee_t));

	memcpy(to_read, array, sizeof(i2c_ee_t));

	if (to_read->crc == HAL_CRC_Calculate(&hcrc, (uint32_t *) &array, (sizeof(array)/4) - 4))
		return (0);
	else
	{
		// CRC error read the backup
		e2p_read(0x40, array, sizeof(i2c_ee_t));

		memcpy(to_read, array, sizeof(i2c_ee_t));

		if (to_read->crc == HAL_CRC_Calculate(&hcrc, (uint32_t *) &array, (sizeof(array)/4) - 4))
			return (0);
		else
			return (1);
	}
}


/*!
 * \brief This function reads data from the 24AA02E48 EEPROM.
 *
 * \param address The address where to read from (NOTE: this must be between 0x00 and 0x7F).
 * \param p_dst Pointer to the buffer where the data is stored.
 * \param len Length of the data to read.
 *
 * example: https://github.com/sinadarvi/SD_HAL_AT24/blob/master/at24_hal_i2c.c
 *
 * \return false if successful, true on error.
 */
bool e2p_read(uint8_t address, uint8_t *p_dst, uint8_t len)
{
	// Sanity checks
	if (!len)
		return (true);
	if ((address + len) > (E2P_PAGE_COUNT * E2P_PAGE_SIZE))
		return (true);

	if (HAL_I2C_Mem_Read(&hi2c2, (uint16_t) E2P_RIIC_ADDRESS, (uint16_t) address, I2C_MEMADD_SIZE_8BIT, p_dst, (uint16_t) len, 100) != HAL_OK)
		return true;

	return false;
}

/*!
 * \brief This function writes data to the 24AA02E48 EEPROM.
 *
 * \param address The address where to write to (NOTE: this must be between 0x00 and 0x7F).
 * \param p_src Pointer to the buffer containig data to be written.
 * \param len Length of the data to write.
 *
 * \return false if successful, true on error.
 */
bool e2p_write(uint8_t address, uint8_t *p_src, uint8_t len)
{
	uint32_t pageaddr;
	uint32_t byteaddr;
	uint32_t remaining;
	uint32_t writelen;
	uint32_t addr;
	int32_t rtr = 50;

	// Sanity checks
	if (!len)
		return (true);
	if ((address + len) > (E2P_PAGE_COUNT * E2P_PAGE_SIZE))
		return (true);

	// Get the remaining number of bytes to write
	remaining = len;
	// Calculate the page address
	pageaddr = address / E2P_PAGE_SIZE;
	// Calculate the byte address within the page (for a partial page write)
	byteaddr = address % E2P_PAGE_SIZE;
	// Calculate the number of bytes to write
	if ((byteaddr + len) > E2P_PAGE_SIZE)
		writelen = E2P_PAGE_SIZE - byteaddr;
	else
		writelen = len;

	// Write data to the device
	while (remaining > 0)
	{
		addr = (pageaddr << E2P_PAGE_OFFSET) | byteaddr;

		rtr = 50;

		if (HAL_I2C_Mem_Write(&hi2c2, (uint16_t) E2P_RIIC_ADDRESS, (uint16_t) addr, I2C_MEMADD_SIZE_8BIT, p_src, (uint16_t) writelen, 100) != HAL_OK)
			return true;

		// 50 x 10ms = 500ms
		while ((--rtr) && !m24xx64_ready())
			;

		p_src += writelen;

		// Update the remaining number of bytes to write
		remaining -= writelen;
		// Update the page address
		pageaddr++;
		// The data is now page aligned
		byteaddr = 0;
		// Calculate the number of bytes to write
		if (remaining > E2P_PAGE_SIZE)
			writelen = E2P_PAGE_SIZE;
		else
			writelen = remaining;
	}

	return (false);
}

/*!
 * \brief This function checks if the memory is busy writing/erasing.
 *
 * \param -.
 *
 * \return True if successful, false on error.
 */
static bool m24xx64_ready(void)
{
	return (HAL_I2C_IsDeviceReady(&hi2c2, E2P_RIIC_ADDRESS, 1, 10) == HAL_OK);
}

#endif
