/*
 * can_functions.h
 *
 *  Created on: 10 sep. 2018
 *      Author: Mark
 */

#ifndef CAN100_H_
#define CAN100_H_

#include "../../../Project_Gateway/inc/can_id.h"

#define BOOTLOADER			(0) // This is the bootloader else application
#define SHELL				(0) // Include shell CAN commands

#define INRQ 1
#define INAK 1


/*
 * Global functions
 */

bool can_send_msg(can_id_t id, uint8_t len, uint8_t * txdata);
void FDCAN_Config(void);
HAL_StatusTypeDef FDCAN_WaitForTxFifoSpace(FDCAN_HandleTypeDef *hfdcan, uint32_t timeout);



#endif /* CAN100_H_ */
