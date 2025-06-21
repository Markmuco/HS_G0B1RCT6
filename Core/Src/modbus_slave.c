/*!
 * \file  modbus_slave.c
 *
 * \brief This software module provides functions for the modbus interface.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "uart_sci.h"
#include "modbus.h"
#include "modbus_slave.h"
#include "main.h"
#include "time.h"
#include "crc16.h"

extern SMBErrorCode_t MBRegHoldingCB(uint8_t * RegBuffer, uint16_t Address, uint8_t NRegs, SLMBRegister_t eMode);

// Private variables
static sl_modbus_tx_state_t modbus_tx_state = SLMODBUS_TX_IDLE;
static uint8_t modbus_tx_tmr = NO_TIMER;
static sl_modbus_rx_state_t modbus_rx_state = SLMODBUS_RX_IDLE;
static uint8_t modbus_rx_tmr = NO_TIMER;

// Private Functions
static void modbus_rx_timeout(void);
static void modbus_reset(void);
static sl_modbus_transaction_t modbus_transaction;
static sl_modbus_status_t modbus_tx(sl_modbus_transaction_t *modbus_transaction, uint32_t timeout);
static sl_modbus_status_t modbus_rx(sl_modbus_transaction_t *modbus_transaction, uint32_t timeout);
static bool modbus_open(void);

static uint16_t crc16_init(void);
static uint16_t crc16_update(uint8_t c);
static uint16_t crc16_get(void);

static uint16_t modbus_crc;

/*!
 * \brief This function handles modbus slave, poll by main
 *
 * \param -.
 *
 * \return - Callback: MBRegHoldingCB when data received
 */
void modbus_slave(void)
{
	static sl_modbus_state_t state = SLMODBUS_INIT;
	sl_modbus_status_t ms;

	switch (state)
	{
	case SLMODBUS_INIT:
		if (modbus_open())
			state = SLMODBUS_RX;
		break;

	case SLMODBUS_RX:
		ms = modbus_rx(&modbus_transaction, 100);
		if (ms == SLMODBUS_OK)
		{
			debug_msg("modbus rx function: %02X\r\n", modbus_transaction.rx_buf[1]);
			state = SLMODBUS_TX;
		}
		if (ms == SLMODBUS_ERR)
		{
			debug_msg("ERROR: sl_modbus rx\r\n");
		}
		break;

	case SLMODBUS_TX:
		// get register 16 bits
		modbus_transaction.regster = ((modbus_transaction.rx_buf[SLMODBUS_REGISTER_MSB]) << 8 | modbus_transaction.rx_buf[SLMODBUS_REGISTER_LSB]);
		// default response with MSB bit set = ERROR
		modbus_transaction.function = modbus_transaction.rx_buf[SLMODBUS_FUNCTION] | 0x80;
		modbus_transaction.tx_len = 0;
		// Command handler.
		switch (modbus_transaction.rx_buf[SLMODBUS_FUNCTION])
		{
		case FUNCTION_READ_REQUEST: // 0x01 -> uit Fenix
			// Copy MODBUS_ADDR and MODBUS_CMD.
			memcpy(modbus_transaction.tx_buf, modbus_transaction.rx_buf, 8);
			// response copy of function
			modbus_transaction.function = modbus_transaction.rx_buf[SLMODBUS_FUNCTION];
			debug_msg("Received READ_REQUEST\r\n");
			break;

		case FUNCTION_READ_REGISTER: // 0x03
			debug_msg("Answer with len %d\r\n", modbus_transaction.rx_buf[SLMODBUS_LEN]);
			debug_msg("Ask register %d\r\n", modbus_transaction.rx_buf[SLMODBUS_REGISTER_LSB] | modbus_transaction.rx_buf[SLMODBUS_REGISTER_MSB] << 8);

			modbus_transaction.tx_len = modbus_transaction.rx_buf[SLMODBUS_LEN] * 2;

			// If data handling correct replay with Function
			if (MBRegHoldingCB(&modbus_transaction.tx_buf[SLMODBUS_DATA], modbus_transaction.regster, modbus_transaction.rx_buf[SLMODBUS_LEN], SLMB_REG_READ) == SLMB_OK)
				modbus_transaction.function = modbus_transaction.rx_buf[SLMODBUS_FUNCTION]; 					// response copy of function
			break;

		case FUNCTION_WRITE_REGISTER: // 0x06
			// copy replay data
			memcpy(&modbus_transaction.tx_buf[0], &modbus_transaction.rx_buf[SLMODBUS_DATA], 2);

			modbus_transaction.tx_len = 1;

			// If data handling correct replay with Function
			if (MBRegHoldingCB(&modbus_transaction.rx_buf[SLMODBUS_DATA], modbus_transaction.regster, 1, SLMB_REG_WRITE) == SLMB_OK)
				modbus_transaction.function = modbus_transaction.rx_buf[SLMODBUS_FUNCTION]; 					// response copy of function

			break;

		case FUNCTION_WRITE_DATA: // 0x10
			modbus_transaction.function = FUNCTION_WRITE_DATA;
			// copy replay data
			memcpy(&modbus_transaction.tx_buf[0], &modbus_transaction.rx_buf[SLMODBUS_DATA], 2);

			modbus_transaction.tx_len = modbus_transaction.rx_buf[SLMODBUS_LEN];

			// If data handling correct replay with Function
			if (MBRegHoldingCB(&modbus_transaction.rx_buf[SLMODBUS_F10], modbus_transaction.regster, modbus_transaction.rx_buf[SLMODBUS_LEN], SLMB_REG_WRITE) == SLMB_OK)
				modbus_transaction.function = modbus_transaction.rx_buf[SLMODBUS_FUNCTION]; 					// response copy of function

			break;

		default:
			break;
		}

		// Transmit response.
		if (modbus_tx(&modbus_transaction, 0) != SLMODBUS_ERR)
		{
			debug_msg("modbus tx func: %02X\r\n", modbus_transaction.tx_buf[1]);
			state = SLMODBUS_RX;
		}
		else
			debug_msg("RX error\r\n");
		break;
	}
}

/*!
 * \brief This function open the timers
 *
 * \param -.
 *
 * \return -
 */
static bool modbus_open(void)
{
	// Initialize timers.
	if (modbus_tx_tmr == NO_TIMER)
	{
		modbus_tx_tmr = timer_get();
	}
	if (modbus_tx_tmr == NO_TIMER)
		return (false);

	if (modbus_rx_tmr == NO_TIMER)
	{
		modbus_rx_tmr = timer_get();
	}

	if (modbus_rx_tmr == NO_TIMER)
		return (false);

	return (true);
}

/*!
 * \brief This function stops the Modbus slave
 *
 * \param -.
 *
 * \return -
 */
bool modbus_close(void)
{
	// Close the timers.
	if (modbus_tx_tmr != NO_TIMER)
	{
		timer_free(&modbus_tx_tmr);
	}

	if (modbus_rx_tmr != NO_TIMER)
	{
		timer_free(&modbus_rx_tmr);
	}

	return (true);
}

/*
 *
 */
static sl_modbus_status_t modbus_tx(sl_modbus_transaction_t *modbus_transaction, uint32_t timeout)
{
	uint16_t i;
	uint16_t crc;
	uint32_t fnction;

	if (modbus_tx_state != SLMODBUS_TX_IDLE)
		return (SLMODBUS_ERR);

	modbus_tx_state = SLMODBUS_TX_BUSY;
	crc16_init();

	fnction = modbus_transaction->function & 0x7F;

	switch (fnction)
	{
	case FUNCTION_READ_REQUEST: // 0x01: 8 byte transmit
		/*
		 Field Name (Hex)
		 00 Slave Address 11
		 01 Function 01
		 02 Byte Count 05
		 03 Data (Coils 27�20) CD
		 04 Data (Coils 35�28) 6B
		 05 Data (Coils 43�36) B2
		 06 Data (Coils 51�44) 0E
		 07 Data (Coils 56�52) 1B
		 08-09 Error Check (LRC or CRC) ��
		 */
		// [00] Slave address
		modbus_putc(SLMODBUS_ADDRESS);
		crc16_update(SLMODBUS_ADDRESS);

		// [01] Function
		modbus_putc(modbus_transaction->function);
		crc16_update(modbus_transaction->function);

		uint8_t a = 1;
		// [02] Byte Count 05
		modbus_putc(a);
		crc16_update(a);
		// [03] Data (Coils 27�20) CD
		modbus_putc(a);
		crc16_update(a);

		crc = crc16_get();
		// [06][07] Checksum low�order byte of the field is appended first,
		modbus_putc((crc >> 0) & 0xFF);
		modbus_putc((crc >> 8) & 0xFF);

		break;

	case FUNCTION_READ_REGISTER: // 0x03: 8 byte transmit
		// 0x00 Address                   0x01
		// 0x01 Function                  0x03   Read multiple registers
		// 0x02 Start address MSB         0x00
		// 0x03 Start address LSB         0x00   From address 40000
		// 0x04 Number of registers MSB   0x00
		// 0x05 Number of registers LSB   0x4E   Until address 40078
		// 0x06 Checksum MSB
		// 0x07 Checksum LSB

		// [00] Slave address
		modbus_putc(SLMODBUS_ADDRESS);
		crc16_update(SLMODBUS_ADDRESS);

		// [01] Function
		modbus_putc(modbus_transaction->function);
		crc16_update(modbus_transaction->function);

		// [03] Bytes count
		modbus_putc(modbus_transaction->tx_len & 0xFF);
		crc16_update(modbus_transaction->tx_len & 0xFF);

		// [04][..] Data
		modbus_putsn(modbus_transaction->tx_buf + SLMODBUS_DATA, modbus_transaction->tx_len);
		for (i = 0; i < modbus_transaction->tx_len; i++)
			crc16_update(modbus_transaction->tx_buf[i + SLMODBUS_DATA]);

		// [..][..] Checksum
		crc = crc16_get();
		modbus_putc((crc >> 0) & 0xFF);
		modbus_putc((crc >> 8) & 0xFF);
		break;

	case FUNCTION_WRITE_REGISTER:  // 0x06: 8 byte transmit
		// 0x00 Address                   0x01
		// 0x01 Function 0x06             0x06   Write single register
		// 0x02 Address MSB               0x00
		// 0x03 Address LSB               0x00   Address 40000
		// 0x04 Data MSB
		// 0x05 Data LSB
		// 0x06 Checksum MSB
		// 0x07 Checksum LSB

		// [00] Slave address
		modbus_putc(SLMODBUS_ADDRESS);
		crc16_update(SLMODBUS_ADDRESS);

		// [01] Function
		modbus_putc(modbus_transaction->function);
		crc16_update(modbus_transaction->function);

		// [02] Register MSB
		modbus_putc((modbus_transaction->regster >> 8) & 0xFF);
		crc16_update((modbus_transaction->regster >> 8) & 0xFF);
		// [03] Register LSB
		modbus_putc(modbus_transaction->regster & 0xFF);
		crc16_update(modbus_transaction->regster & 0xFF);

		// [04] Length MSB
		modbus_putc(modbus_transaction->tx_buf[0]); // 20-11-2019 MUR values were crossed
		crc16_update(modbus_transaction->tx_buf[0]);
		// [05] Length LSB
		modbus_putc(modbus_transaction->tx_buf[1]);
		crc16_update(modbus_transaction->tx_buf[1]);

		// [06][07] Checksum
		crc = crc16_get();
		modbus_putc((crc >> 0) & 0xFF);
		modbus_putc((crc >> 8) & 0xFF);
		break;

	case FUNCTION_WRITE_DATA: // 0x10 write block
		// 0x00 Address                   0x01
		// 0x01 Function 0x10             0x10   Write multiple registers
		// 0x02 Start address MSB         0x00
		// 0x03 Start address LSB         0x00   From address 40000
		// 0x04 Number of registers MSB   0x00
		// 0x05 Number of registers LSB   0x4E   Until address 40078
		// 0x06 Byte count
		// 0x07 Data MSB
		// 0x08 Data LSB
		// 0x09 Checksum MSB
		// 0x0A Checksum LSB

		// [00] Slave address
		modbus_putc(SLMODBUS_ADDRESS);
		crc16_update(SLMODBUS_ADDRESS);

		// [01] Function
		modbus_putc(modbus_transaction->function);
		crc16_update(modbus_transaction->function);

		// [02] Register MSB
		modbus_putc((modbus_transaction->regster >> 8) & 0xFF);
		crc16_update((modbus_transaction->regster >> 8) & 0xFF);
		// [03] Register LSB
		modbus_putc(modbus_transaction->regster & 0xFF);
		crc16_update(modbus_transaction->regster & 0xFF);

		// [04] Length MSB

		modbus_putc((modbus_transaction->tx_len >> 8) & 0xFF);
		crc16_update((modbus_transaction->tx_len >> 8) & 0xFF);
		// [05] Length LSB
		modbus_putc(modbus_transaction->tx_len & 0xFF);
		crc16_update(modbus_transaction->tx_len & 0xFF);

		// [..][..] Checksum
		crc = crc16_get();
		modbus_putc((crc >> 0) & 0xFF);
		modbus_putc((crc >> 8) & 0xFF);
		break;

	default:
		debug_msg("No function to send\r\n");
		break;

	}
	return (timeout ? SLMODBUS_BUSY : SLMODBUS_OK);
}

static sl_modbus_status_t modbus_rx(sl_modbus_transaction_t *modbus_transaction, uint32_t timeout)
{
	uint8_t c;
	bool busy = true;

	if ((modbus_tx_state == SLMODBUS_TX_ERR) || (modbus_rx_state == SLMODBUS_RX_ERR))
	{
		modbus_reset();
		return (SLMODBUS_ERR);
	}

	if (modbus_getc(&c))
	{
		// DEBUG!!
		debug_msg("(0x%02X) ", c);

		switch (modbus_rx_state)
		{
		case SLMODBUS_RX_IDLE:
			if (c == SLMODBUS_ADDRESS)
			{
				if ((modbus_rx_tmr != NO_TIMER) && (timeout > 0))
					timer_start(modbus_rx_tmr, timeout, modbus_rx_timeout);

				crc16_init();
				crc16_update(c);

				modbus_transaction->rx_len = 0;
				modbus_transaction->rx_buf[modbus_transaction->rx_len++] = c; // place 0 for address

				modbus_rx_state = SLMODBUS_RX_BUSY;
			}
			break;

		case SLMODBUS_RX_BUSY:
			if (modbus_transaction->rx_len < sizeof(modbus_transaction->rx_buf))
			{
				if ((modbus_rx_tmr != NO_TIMER) && (timeout > 0))
					timer_start(modbus_rx_tmr, timeout, modbus_rx_timeout);

				crc16_update(c);

				modbus_transaction->rx_buf[modbus_transaction->rx_len++] = c;
			}

			// 8 byte message
			if ((modbus_transaction->rx_buf[1] == FUNCTION_READ_REQUEST) || (modbus_transaction->rx_buf[1] == FUNCTION_READ_REGISTER) || (modbus_transaction->rx_buf[1] == FUNCTION_WRITE_REGISTER))
				if ((crc16_get() == 0) && (modbus_transaction->rx_len == 8))
				{
					modbus_reset();
					busy = false;
				}

			// various length message
			if (modbus_transaction->rx_buf[1] == FUNCTION_WRITE_DATA)
			{
				// check length
				if ((crc16_get() == 0) && (modbus_transaction->rx_len == modbus_transaction->rx_buf[6] + 9))
				{
					modbus_reset();
					busy = false;
				}

			}
			break;

		default:
			break;
		}
	}

	return (busy ? SLMODBUS_BUSY : SLMODBUS_OK);
}

static void modbus_rx_timeout(void)
{
	modbus_rx_state = SLMODBUS_RX_ERR;
}

static void modbus_reset(void)
{
	modbus_tx_state = SLMODBUS_TX_IDLE;
	modbus_rx_state = SLMODBUS_RX_IDLE;

	if (modbus_tx_tmr != NO_TIMER)
		timer_stop(modbus_tx_tmr);
	if (modbus_rx_tmr != NO_TIMER)
		timer_stop(modbus_rx_tmr);
}

/*!
 * \brief This function initializes the CRC16.
 *
 * \param -.
 *
 * \return CRC16 value.
 */
static uint16_t crc16_init(void)
{
	modbus_crc = 0xFFFF;

	return (modbus_crc);
}

/*!
 * \brief This function updates the CRC16.
 *
 * \param c Data to calculate the CRC32 over.
 *
 * \return CRC16 value.
 */
static uint16_t crc16_update(uint8_t c)
{
	uint8_t i;

	modbus_crc ^= c;

	for (i = 8; i != 0; i--)
	{
		if (modbus_crc & 1)
			modbus_crc = (modbus_crc >> 1) ^ MODBUS_POLYNOM;
		else
			modbus_crc >>= 1;
	}

	return (modbus_crc);
}

/*!
 * \brief This function returns the CRC16.
 *
 * \param -.
 *
 * \return CRC16 value.
 */
static uint16_t crc16_get(void)
{
	return (modbus_crc);
}

