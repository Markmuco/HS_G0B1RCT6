/*
 * flash.c
 *
 *  Created on: 24 jan. 2017
 *      Author: Mark Ursum
 */

#include "main.h"
#include "flash.h"
#include "iwdg.h"

#include "uart_sci.h"


static uint32_t GetPage(uint32_t Addr);

static FLASH_EraseInitTypeDef EraseInitStruct;



/*!
 * \brief Erase the application memory.
 *
 * \param -
 *
 * \return false if unsuccessful, else true
 */
HAL_StatusTypeDef stm32_flash_erase(uint32_t start, uint32_t size)
{
	uint32_t FirstPage = 0, NbOfPages = 0;
	uint32_t PageError = 0;

	tty_printf("got %08X size %02X\r\n", start,size);

	/* Get the 1st page to erase */
	FirstPage = GetPage(start);

	tty_printf("page %d\r\n", FirstPage);

	/* Get the number of pages to erase from 1st page */
	NbOfPages = size / FLASH_PAGE_SIZE;

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Page = FirstPage;
	EraseInitStruct.NbPages = NbOfPages;
	EraseInitStruct.Banks = FLASH_BANK_2;

	tty_printf("Erase page %d qty %d\r\n", EraseInitStruct.Page, NbOfPages);

	HAL_FLASH_Unlock();

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	 you have to make sure that these data are rewritten before they are accessed during code
	 execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	 DCRST and ICRST bits in the FLASH_CR register. */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	{
		/*
		 Error occurred while page erase.
		 User can add here some code to deal with this error.
		 PageError will contain the faulty page and then to know the code error on this page,
		 user can call function 'HAL_FLASH_GetError()'
		 */
		return (HAL_ERROR);
	}

	HAL_FLASH_Lock();

	return (HAL_OK);
}

/*!
 * \brief Write data to the application memory.
 *
 * \param address   Start address of write.
 * \param p_data   Pointer to data to write.
 * \param size   Size of data to write.
 *
 * \return	false if unsuccessful, else true
 *
 * \note The flash memory must be erased before it can be written.
 */
bool stm32_flash_write(uint32_t address, uint8_t * p_data, uint32_t size)
{

	HAL_FLASH_Unlock();

	uint64_t temp;

	// Iterate through the number of data bytes
	for (uint32_t var = 0; var < size; var += 8)
	{
		wdt_clr();

		memcpy(&temp, p_data, sizeof(temp));

		tty_printf("Write %08X\r\n", address);

// write block of 2*4 bytes
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, (uint64_t) temp) != HAL_OK) /*!< Fast program a 32 row double-word (64-bit) at a specified address */
		{
			return (0); // fout
		}

		address += 8;
		p_data = p_data + 8;
	}

	HAL_FLASH_Lock();

	return (1);
}

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t GetPage(uint32_t Addr)
{
	uint32_t page = 0;

	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	{
		/* Bank 1 */
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else
	{
		/* Bank 2 */
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}

	return page;
}


