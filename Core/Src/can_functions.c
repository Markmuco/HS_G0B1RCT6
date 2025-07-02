/* fw_update.c
 *
 *  Created on: 10 sep. 2018
 *      Author: Mark
 */

#include "main.h"
#include "vars.h"

#include "can_functions.h"
#include "crc.h"
#include "uart_sci.h"
#include "time.h"
#include "fdcan.h"
#include "rtc.h"
#include "string.h"

/*
 ____    _    _   _   ____    ___    ____  ____  _____     _______ ____
 / ___|  / \  | \ | | |___ \  / _ \  |  _ \|  _ \|_ _\ \   / / ____|  _ \
 | |     / _ \ |  \| |   __) || | | | | | | | |_) || | \ \ / /|  _| | |_) |
 | |___ / ___ \| |\  |  / __/ | |_| | | |_| |  _ < | |  \ V / | |___|  _ <
 \____/_/   \_\_| \_| |_____(_)___/  |____/|_| \_\___|  \_/  |_____|_| \_
 Interface:
 init:				MX_CAN_Config
 send data:		can_send_msg
 poll from main:	handle_can will capture incoming data

 Settings:
 (X) Bootloader / Application
 (X) Shell

 */

#if BOOTLOADER>0
static uint8_t reboot_tmr = NO_TIMER;
#endif

#if 1

// Local functions
static bool compare_serial(uint8_t *data);
static void can_100_procedure(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);

/**
 * @brief		CAN send message to Queue
 *
 * @param		id		11 bit ID
 * 					len		Data length 0..8 bytes
 * 					data	Pointer to data
 * 					rtr		CAN_RTR_DATA / CAN_RTR_REMOTE
 *
 * @retval	TRUE if placed in queue
 */
bool can_send_msg(can_id_t id, uint8_t len, uint8_t *txdata)
{
	FDCAN_TxHeaderTypeDef TxHeader;

	/* Prepare Tx Header */
	TxHeader.Identifier = id;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = len;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	/* Start the Transmission process */
	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, txdata) != HAL_OK)
	{
		/* Transmission request Error */
		Error_Handler();
	}
}

/**
 * @brief Wait until there is space in the FDCAN Tx FIFO.
 * @param hfdcan Pointer to FDCAN handle (e.g., &hfdcan1)
 * @param timeout Timeout in milliseconds
 * @retval HAL_OK if space became available
 * @retval HAL_TIMEOUT if timeout occurred
 */
HAL_StatusTypeDef FDCAN_WaitForTxFifoSpace(FDCAN_HandleTypeDef *hfdcan, uint32_t timeout)
{
	uint32_t tickStart = HAL_GetTick();

	while ((hfdcan->Instance->TXFQS & FDCAN_TXFQS_TFQF) != 0)
	{
		if ((HAL_GetTick() - tickStart) >= timeout)
		{
			return HAL_TIMEOUT;
		}
	}
	return HAL_OK;
}

/**
 * @brief  Rx FIFO 0 callback.
 * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
 *         the configuration information for the specified FDCAN.
 * @param  RxFifo0ITs: indicates which Rx FIFO 0 interrupts are signalled.
 *         This parameter can be any combination of @arg FDCAN_Rx_Fifo0_Interrupts.
 * @retval None
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	static FDCAN_RxHeaderTypeDef RxHeader;
	static uint8_t RxData[8];

	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retrieve Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		// Goto Special Functions. Poll and ..
		if ((RxHeader.Identifier < 100) && (RxHeader.IdType == FDCAN_STANDARD_ID))
			can_100_procedure(&RxHeader, RxData);

		/*
		 * Other RX actions
		 */
	}
}

/**
 * @brief  Configures the FDCAN.
 * @param  None
 * @retval None
 */
void FDCAN_Config(void)
{
#if 0
	FDCAN_FilterTypeDef sFilterConfig;

	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x000;
	sFilterConfig.FilterID2 = 0x7FF;  // Accept all standard IDs
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
#endif
	/* Configure global filter:
	 Filter all remote frames with STD and EXT ID
	 Reject non matching frames with STD ID and EXT ID */
	if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
	{
		Error_Handler();
	}

	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
		Error_Handler();
	}
}


/**
 * @brief  basic CAN id handling for below 100 id
 *
 * @param  can_trx_t packet of received data
 *
 * @retval None
 */
static void can_100_procedure(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData)
{
#if BOOTLOADER>0
	static uint32_t blocks_cnt = 0;
	can_trx_t tx_data;
	static bool communication = false;
#else
	uint8_t buff[8];
	uint32_t tmp;
	volatile int *p_key = (int*) RAM_KEY;
	static bool shell_active = false; // Can be activated by CAN command
#endif
	uint32_t *p_serial = (uint32_t*) RAM_SERIAL; // application fills the serial

	// My serial on fixed RAM address
	*p_serial = CAN_SERIAL;

	// response to POLL with 11
	if (pRxHeader->Identifier == ID_POLL)
	{
		tty_printf("Poll\r\n");
		tmp = CAN_SERIAL;
		memcpy(buff, &tmp, 4);
		can_send_msg(ID_POLL_RPY, 4, buff);
	}

#if BOOTLOADER>0
	// Response to receive serial number
	if (rx_data.RxHeader.StdId == ID_BL_SERIAL)
	{
		{
			memcpy(p_serial, rx_data.Data, 4 );
			sci1_puts("Set serial\r\n");

			memcpy(tx_data.Data, p_serial, 4 );
			can_send_msg(ID_BL_RESULT, 4,(uint8_t *) p_serial, CAN_RTR_DATA);
		}
	}

	// Response to JumpBoot
	if (rx_data.RxHeader.StdId == ID_BL_ACITVATE)
	{
		if (compare_serial(rx_data.Data))
		{
			sci1_puts("Already in bootloader\r\n");

			memcpy(tx_data.Data, p_serial, 4 );
			can_send_msg(ID_OC_RES, 4,(uint8_t *) p_serial, CAN_RTR_DATA);
		}
	}

	// Response to Open channel
	if (rx_data.RxHeader.StdId == ID_OPEN)
	{
		if (compare_serial(rx_data.Data))
		{
			sci1_puts("CAN Channel Open\r\n");
			communication = true;
			blocks_cnt = 0;
			can_send_msg(ID_OC_RES, 4,(uint8_t *) p_serial, CAN_RTR_DATA);
		}
	}

	// Response to Close communication
	if (rx_data.RxHeader.StdId == ID_CLOSE)
	{
		communication = false;

		if (compare_serial(rx_data.Data))
		{
			sci1_puts("CAN Close\r\n");
			can_send_msg(ID_OC_RES, 4,(uint8_t *) p_serial, CAN_RTR_DATA);
		}
	}

	// Response to Reset
	if (rx_data.RxHeader.StdId == ID_START)
	{
		if (compare_serial(rx_data.Data) && communication)
		{
			sci1_puts("Start App\r\n");
			tx_data.Data[0] = 0;
			can_send_msg(ID_FL_RESULT, 4, tx_data.Data, CAN_RTR_DATA);

			// set reboot function
			if (reboot_tmr == NO_TIMER)
			reboot_tmr = timer_get();
			timer_start(reboot_tmr, 200, f_start);//sh_st);
		}
	}

	// Response to Flash erase
	if (rx_data.RxHeader.StdId == ID_ERASE)
	{
		if (compare_serial(rx_data.Data) && communication)
		{
			sci1_puts("CAN Flash Erase\r\n");
			blocks_cnt = 0;
			tx_data.Data[0] = stm32_flash_erase(FLASH_APP_START_ADDR, APP_SIZE);
			can_send_msg(ID_FL_RESULT, 1, tx_data.Data, CAN_RTR_DATA);
		}
	}

	// Receive data
	if (rx_data.RxHeader.StdId == ID_FL_DATA)
	{
		if (communication)
		{
			//	sci1_printf("CAN Flash write 0x%08X\r\n", FLASH_SHADOW_START_ADDR + blocks_cnt * 8);
			tx_data.Data[0] = stm32_flash_write(FLASH_APP_START_ADDR + blocks_cnt * 8, rx_data.Data, 8);
			tx_data.Data[0] ^= 1;
			blocks_cnt++;

			can_send_msg(ID_FL_RESULT, 1, tx_data.Data, CAN_RTR_DATA);

		}
	}

	if (rx_data.RxHeader.StdId == ID_ASK_CRC)
	{
		app_info_t app_info;
		app_info_t *p_app_info = (app_info_t*) FLASH_APP_START_ADDR;

		if (compare_serial(rx_data.Data) && communication)
		{
			sci1_puts("CAN check CRC\r\n");
			crc_verify_flash(&app_info, FLASH_APP_START_ADDR, APP_SIZE);

//			sci1_printf("CRC = 0x%08X\r\n", app_info.crc32);

			tx_data.Data[0] = (app_info.crc32 >> 24) & 0xFF;
			tx_data.Data[1] = (app_info.crc32 >> 16) & 0xFF;
			tx_data.Data[2] = (app_info.crc32 >> 8) & 0xFF;
			tx_data.Data[3] = (app_info.crc32 >> 0) & 0xFF;
			tx_data.Data[4] = (p_app_info->crc32 >> 24) & 0xFF;
			tx_data.Data[5] = (p_app_info->crc32 >> 16) & 0xFF;
			tx_data.Data[6] = (p_app_info->crc32 >> 8) & 0xFF;
			tx_data.Data[7] = (p_app_info->crc32 >> 0) & 0xFF;

			can_send_msg(ID_CRC, 8, tx_data.Data, CAN_RTR_DATA);
		}
	}
#else
	// APPLICATION:
	// Response to JumpBoot
	if (pRxHeader->Identifier == ID_BL_ACITVATE)
	{
		if (compare_serial(pRxData))
		{
			//sci1_printf("Jump to bootloader\r\n");

			*p_key = CAN_KEY;
			*p_serial = CAN_SERIAL;

			NVIC_SystemReset();
		}
	}
#endif
#if SHELL>0

	// Response to Open Shell
	if (pRxHeader->Identifier == ID_SH_OPEN)
	{
		if (compare_serial(pRxData))
		{
			shell_active = true;

			tty_printf("CAN SH Open\r\n");
			tmp = CAN_SERIAL;
			memcpy(buff, &tmp, 4);
			can_send_msg(ID_SH_RES, 4, buff);
			shell_use_can();
		}
	}

	// Response to Close Shell
	if (pRxHeader->Identifier == ID_SH_CLOSE)
	{
		shell_active = false;

		if (compare_serial(pRxData))
		{
			shell_use_sci1();
			shell_active = false;
			tty_printf("CAN SH Close\r\n");

			tmp = CAN_SERIAL;
			memcpy(buff, &tmp, 4);
			can_send_msg(ID_SH_RES, 4, buff);
		}
	}

	// Response to RX data
	if (pRxHeader->Identifier == ID_SH_RX)
	{
		if (shell_active)
		{
			for (uint8_t var = 0; var < pRxHeader->DataLength; ++var)
				sci_can_keyboard(pRxData[var]);
		}
	}
#endif
}

/**
 * @brief  Compare own CAN id (RAM) with argument
 *
 * @param  4 byte ID
 *
 * @retval TRUE if identical
 */
static bool compare_serial(uint8_t *data)
{
	uint32_t *p_serial = (uint32_t*) RAM_SERIAL; // application fills the serial
	return (*p_serial == (data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0]));
}

#endif
