/*!
 * \file  modbus.h
 *
 * \brief This software module provides functions for the modbus interface.
 */
#ifndef _MODBUS_H_
#define _MODBUS_H_


typedef struct s_modbus_transfer
{
  uint8_t addr;
  uint8_t function;
  uint16_t regster; // command
  uint8_t tx_buf[128];
  uint8_t tx_len;		// len in bytes
  uint8_t rx_buf[128];
  uint8_t rx_len;		// len in Bytes
  bool response_error;
} modbus_transfer_t;

typedef enum
{
  FUNCTION_READ_REQUEST = 0x01,
  FUNCTION_READ_REGISTER = 0x03,
  FUNCTION_WRITE_REGISTER = 0x06,
  FUNCTION_WRITE_DATA = 0x10,
} function_t;

typedef enum e_modbus_result
{
  MODBUS_BUSY = 0,
  MODBUS_OK,
  MODBUS_ERR,
  MODBUS_REJECTED,
} modbus_result_t;

typedef enum e_modbus_state
{
  MODBUS_STATE_TX = 0,
  MODBUS_STATE_RX_ADDR,
  MODBUS_STATE_RX_FUNCTION,
  MODBUS_STATE_RX_8BYTE_06,
  MODBUS_STATE_RX_8BYTE_10,
  MODBUS_STATE_RX_LEN,
  MODBUS_STATE_RX_DATA,
  MODBUS_STATE_RX_CRC,
  MODBUS_STATE_ERR,
} modbus_state_t;

typedef enum
{
	 MB_READY,
	 MB_BUSY,
	 MB_DATA_RDY,
	 MB_ERROR,
	 MB_RESET_BMS,
}send_st_t;


#endif
