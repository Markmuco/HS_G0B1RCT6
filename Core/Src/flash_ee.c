/*
 * flash_ee.c
 *
 *  Created on: 25 sep. 2021
 *      Author: Mark
 */

#include "i2c.h"
#include "flash_ee.h"
#include "vars.h"
#include "string.h"
#include "flash.h"
#include "usart.h"
#include "stdlib.h"
#include "shell.h"
#include "crc.h"
#include "uart_sci.h"

static flash_err_t do_WriteStruct2Flash(void *data, uint32_t start_addr, uint16_t block_size, uint16_t data_size);

/*!
 * \brief This function Reads a struct from internal Flash
 *
 * \param - p_data pointer to structure to save
 * 		  - sizeof structure
 *
 * \return ERR_OK / ERR_CRC
 */
flash_err_t ReadStruct2Flash(void *p_data, uint16_t data_size)
{
	// round 4 bytes
	uint16_t aligned_data_size = ((data_size + 3) / 4) * 4;

	uint32_t *p_flash = (uint32_t*) FLASH_EE_START_ADDR;

	// CRC check the block including the CRC itself will return 0
	if (HAL_CRC_Calculate(&hcrc, p_flash, (aligned_data_size + 4) / 4) == 0)
	{
		// copy the data back
		memcpy(p_data, p_flash, data_size);
		return ERR_OK;
	}
	// checksum fail try block 2
	else
	{
		p_flash = (uint32_t*) FLASH_EE_COPY_START_ADDR;

		// CRC check the block including the CRC itself will return 0
		if (HAL_CRC_Calculate(&hcrc, p_flash, (aligned_data_size + 4) / 4) == 0)
		{
			//read the block including crc32
			memcpy(p_data, p_flash, data_size);
			return ERR_OK_COPY;
		}
	}

	return ERR_CRC;
}

/*!
 * \brief This function Saves a struct to internal Flash
 * 		Set the CRC module to WORD
 *
 * \param	- p_data pointer to structure to save
 * 			- sizeof structure
 *
 * \return		ERR_OK / ERR_CRC / ERR_WRITE / ERR_ERASE / ERR_SIZE
 */
flash_err_t WriteStruct2Flash(void *p_data, uint16_t data_size)
{
	if (data_size + 4 > FLASH_EE_SIZE)
		return ERR_SIZE;

	flash_err_t res = 0;
	// write block 1
	res = do_WriteStruct2Flash(p_data, FLASH_EE_START_ADDR, FLASH_EE_SIZE, data_size);
	// write block 2
	res |= do_WriteStruct2Flash(p_data, FLASH_EE_COPY_START_ADDR, FLASH_EE_SIZE, data_size);

	return res;
}

/*!
 * \brief This function Saves a struct to internal Flash
 *
 * \param	- p_data pointer to structure to save
 *			- start_addr Flash save adress
 *			- block_size Flash erase block size
 * 			- data_size sizeof structure
 *
 * \return ERR_OK / ERR_CRC / ERR_WRITE / ERR_ERASE
 */
static flash_err_t do_WriteStruct2Flash(void *data, uint32_t start_addr, uint16_t block_size, uint16_t data_size)
{
	uint32_t *p_heap;
	// round 4 bytes
	data_size = ((data_size + 3) / 4) * 4;

	/*##-1- Erase flash ###################################*/
	if (stm32_flash_erase(start_addr, block_size) != HAL_OK)
		return ERR_ERASE;

	// allocate datablock+crc32
	p_heap = malloc(data_size + 4);
	if (p_heap == NULL)
		return ERR_MEM;

	// copy the datablock to the heap to add the crc32 on top
	memcpy(p_heap, data, data_size);

	/*##-2- Compute the CRC in Words ###################################*/
	*(p_heap + data_size / 4) = HAL_CRC_Calculate(&hcrc, p_heap, data_size / 4);

	if (stm32_flash_write(start_addr, (uint8_t*) p_heap, data_size + 4) == false)
	{
		free(p_heap);
		return ERR_WRITE;
	}

	/*##-3- Check written data ###################################*/
	if (HAL_CRC_Calculate(&hcrc, (uint32_t*) start_addr, (data_size + 4) / 4) != 0)
	{
		free(p_heap);
		return ERR_CRC;
	}

	free(p_heap);
	return ERR_OK;
}



