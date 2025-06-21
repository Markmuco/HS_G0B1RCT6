/*
 * flash_ee.h
 *
 *  Created on: 25 sep. 2021
 *      Author: Mark
 */

#ifndef FLASH_EE_H_
#define FLASH_EE_H_


#include <main.h>

typedef enum
{
	ERR_OK,
	ERR_OK_COPY,
	ERR_CRC,
	ERR_ERASE,
	ERR_WRITE,
	ERR_SIZE,
	ERR_MEM
}flash_err_t;


flash_err_t ReadStruct2Flash(void * p_data, uint16_t data_size);
flash_err_t WriteStruct2Flash(void * data, uint16_t len);


#endif /* FLASH_EE_H_ */
