/*!
 * \file  modbus_slave.h
 *
 * \brief This software module provides functions for the modbus interface.
 */
#ifndef _MODBUS_SLAVE_H_
#define _MODBUS_SLAVE_H_

// RAM allocation
#define REG_HOLDING_START		0 // registers start at number
#define REG_HOLDING_NREGS		100 // number of 16bit registers

#define MODBUS_DEBUG   (0)

#define SLMODBUS_ADDRESS   		(0x01)
#define MODBUS_POLYNOM          (0xA001)

//#define modbus_putc(a)          (sci1_putc((char)a))
//#define modbus_putsn(a, b)      (sci1_putsn((char*)a, b))
//#define modbus_getc(a)          (sci1_getch((char*)a))

#define modbus_putc(a)          (mp.sci_putc((char)a))
#define modbus_putsn(a, b)      (mp.sci_putsn((char*)a, b))
#define modbus_getc(a)          (mp.sci_getch((char*)a))


#if (MODBUS_DEBUG)
#define debug_msg(...)  (sci1_printf(__VA_ARGS__))
#else
#define debug_msg(...)  NULL
#endif


typedef struct s_modbus_transaction
{
  uint8_t tx_buf[132+5];
  uint16_t tx_len;
  uint8_t rx_buf[132+5];
  uint16_t rx_len;
  uint16_t regster;
  uint8_t function;
} sl_modbus_transaction_t;

#if 0
typedef enum
{
  FUNCTION_READ_REQUEST = 0x01,
  FUNCTION_READ_REGISTER = 0x03,
  FUNCTION_WRITE_REGISTER = 0x06,
  FUNCTION_WRITE_DATA = 0x10,
} function_t;
#endif

typedef enum e_modbus_tx_state
{
  SLMODBUS_TX_IDLE = 0,
  SLMODBUS_TX_BUSY,
  SLMODBUS_TX_OK,
  SLMODBUS_TX_ERR
} sl_modbus_tx_state_t;

typedef enum e_modbus_rx_state
{
  SLMODBUS_RX_IDLE = 0,
  SLMODBUS_RX_BUSY,
  SLMODBUS_RX_OK,
  SLMODBUS_RX_ERR
} sl_modbus_rx_state_t;

typedef enum
{
	SLMB_OK,
	SLMB_NOREGISTER,
} SMBErrorCode_t;



typedef enum e_modbus_status
{
	SLMODBUS_IDLE = 0,
	SLMODBUS_BUSY,
	SLMODBUS_OK,
	SLMODBUS_ERR
} sl_modbus_status_t;



typedef enum e_modbus_index
{
	SLMODBUS_ADDR = 0,
	SLMODBUS_FUNCTION,
	SLMODBUS_REGISTER_MSB,
	SLMODBUS_REGISTER_LSB,
	SLMODBUS_DATA,
	SLMODBUS_LEN,
	SLMODBUS_F10 = 7, // start of data in function 0x10
} sl_modbus_index_t;


typedef enum
{
	SLMB_REG_READ,
	SLMB_REG_WRITE
}SLMBRegister_t;

typedef enum e_slmodbus_state
{
  SLMODBUS_INIT = 0,
  SLMODBUS_RX,
  SLMODBUS_TX
} sl_modbus_state_t;

#endif
// Public function prototypes

void modbus_slave(void);
bool modbus_close(void);

