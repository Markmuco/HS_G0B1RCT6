/*
 * test_functions.c
 *
 *  Created on: 5 dec. 2022
 *      Author: Mark
 */

#include "main.h"
#include "modbus.h"
#include "modbus_slave.h"
#include "shell.h"
#include "modbus_cmd.h"
#include "machine.h"
#include "flash_ee.h"

/*
 * Local variables
 */
static uint16_t modbus_count = 0;

/*
 * Local functions
 */


/*!
 * \brief
 *
 * \param
 *
 * \return -.
 */
void process_modbus_slave(void)
{
	// connection to globals
	modbus_slave();
}

/*!
 * \brief Values asked by the Modbus
 *
 * \param - Register to get
 *
 * \return - 16bit modbus value
 */
uint16_t modbus_getvalue(uint16_t RegIndex)
{
	modbus_count++;

	switch (RegIndex)
	{
	case CMD_ID:
		return 0x100;
		break;

	case CMD_READ_CNT:
		return modbus_count;
		break;

	case CMD_POS_X_HIGH:
		return HIGH_16B(vars.eevar.actual_motor.x);
		break;

	case CMD_POS_X_LOW:
		return LOW_16B(vars.eevar.actual_motor.x);
		break;

	case CMD_POS_Y_HIGH:
		return HIGH_16B(vars.eevar.actual_motor.y);
		break;

	case CMD_POS_Y_LOW:
		return LOW_16B(vars.eevar.actual_motor.y);

	case CMD_MOVE_X:
		// no return here
		return 0;
		break;

	case CMD_MOVE_Y:
		// no return here
		return 0;
		break;

	case CMD_OPMODE:
		return vars.eevar.main_mode;

		break;

	case CMD_SAVE:
		// no return here
		return 0;
		break;

	case CMD_STEPS_X:
		return vars.hwinfo.steps.x;
		break;

	case CMD_STEPS_Y:
		return vars.hwinfo.steps.y;
		break;

	case CMD_WIND:
		return vars.wind_ppm;
		break;

	case CMD_WIND_SET:
		return vars.hwinfo.max_windpulse;
		break;

	case CMD_MINUTES_HIGH:
		return HIGH_16B(vars.eevar.tracking_minutes);
		break;
	case CMD_MINUTES_LOW:
		return LOW_16B(vars.eevar.tracking_minutes);
		break;

	case CMD_GPS_VALID:
		return (vars.gps_decode == DECODING_RDY);
		break;

	case CMD_MAX_HOR_HIGH:
		return HIGH_16B(vars.hwinfo.maximum.x);
		break;

	case CMD_MAX_HOR_LOW:
		return LOW_16B(vars.hwinfo.maximum.x);
		break;

	case CMD_MAX_VER_HIGH:
		return HIGH_16B(vars.hwinfo.maximum.y);
		break;

	case CMD_MAX_VER_LOW:
		return LOW_16B(vars.hwinfo.maximum.y);
		break;

	case CMD_CALIBRATE_HOR:
		// no return here
		return 0;
		break;

	case CMD_CALIBRATE_VER:
		// no return here
		return 0;
		break;

	case CMD_DEVIATION_HOR:
		return (int16_t) vars.deviation.x;
		break;

	case CMD_DEVIATION_VER:
		return (int16_t) vars.deviation.y;
		break;

	case CMD_SUNDOWN_ANGLE:
		return (int16_t) vars.hwinfo.sun_down_angle;
		break;

	case CMD_SUN_ELEVATION:
		return (int16_t) vars.sunpos.elevation;
		break;

	case CMD_SUN_AZIMUTH:
		return (int16_t) vars.sunpos.azimuth;
		break;

	default:
		return 0xFFFF;
		break;

	}

	// Will not happen
	return 0x00;
}

/*!
 * \brief Values set by the master Modbus
 *
 * \param - Register to get
 *
 * \return - 16bit modbus value
 */
void modbus_setvalue(uint16_t RegIndex, uint16_t RegValue)
{
	static uint16_t Prev_RegIndex;
	static uint16_t Prev_RegValue;

	switch (RegIndex)
	{
	case CMD_ID:
		// No set possible
		break;

	case CMD_POS_X_HIGH:
		break;

	case CMD_POS_X_LOW:
		if (Prev_RegIndex == CMD_POS_X_HIGH)
			vars.eevar.actual_motor.x = (Prev_RegValue << 16) | RegValue;
		break;

	case CMD_POS_Y_HIGH:
		break;

	case CMD_POS_Y_LOW:
		if (Prev_RegIndex == CMD_POS_Y_HIGH)
			vars.eevar.actual_motor.y = (Prev_RegValue << 16) | RegValue;
		break;

	case CMD_MOVE_X:
		if ((int16_t)RegValue > 0)
			cmd_move_cw((int16_t)RegValue);
		else
			cmd_move_ccw(abs((int16_t)RegValue));
		break;

	case CMD_MOVE_Y:
		if ((int16_t)RegValue > 0)
			cmd_move_up((int16_t)RegValue);
		else
			cmd_move_down(abs((int16_t)RegValue));
		break;

	case CMD_OPMODE:
		switch (RegValue)
		{
			case ST_TRACK_SUN:
				vars.store_main_mode = vars.eevar.main_mode = ST_TRACK_SUN;
				break;
			case ST_TRACK_MANUAL:
				// No set possible
				break;

			case ST_TRACK_TARGET_1:
			case ST_TRACK_TARGET_2:
			case ST_TRACK_TARGET_3:
			case ST_TRACK_TARGET_4:
			case ST_TRACK_TARGET_5:
			case ST_TRACK_TARGET_6:
			case ST_TRACK_TARGET_7:
			case ST_TRACK_TARGET_8:
			case ST_TRACK_TARGET_9:
			case ST_TRACK_TARGET_10:
			case ST_TRACK_TARGET_11:
			case ST_TRACK_TARGET_12:
			case ST_TRACK_TARGET_13:
			case ST_TRACK_TARGET_14:
			case ST_TRACK_TARGET_15:
			case ST_TRACK_TARGET_16:
				vars.eevar.target = vars.hwinfo.target[RegValue - ST_TRACK_TARGET_1].pos;
				vars.store_main_mode = vars.eevar.main_mode = RegValue;
				break;

			case ST_STOP:
				vars.eevar.main_mode = ST_STOP;
				vars.goto_motor = vars.eevar.actual_motor;
				break;

			case ST_OFF:
				vars.eevar.main_mode = ST_OFF;
				vars.goto_motor = vars.hwinfo.parkposition;
				break;

			default:
				break;
		}
		break;

	case CMD_SAVE:
		switch (RegValue)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
				cmd_savetarget(RegValue);
				break;
			case 17:
				cmd_savepark();
				break;
			case 18:
				cmd_savesun();
				break;

			default:
				break;
		}
		break;

	case CMD_STEPS_X:
		if (RegValue > 1 && RegValue < 32000)
		{
			vars.hwinfo.steps.x = RegValue;
			WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
		}
		break;

	case CMD_STEPS_Y:
		if (RegValue > 1 && RegValue < 32000)
		{
			vars.hwinfo.steps.y = RegValue;
			WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
		}
		break;

	case CMD_WIND:
		// No set possible
		break;

	case CMD_WIND_SET:
		// 0 or 100..10000
		if (((RegValue < 10000) && (RegValue > 100)) || RegValue == 00)
		{
			vars.hwinfo.max_windpulse = RegValue;
			WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
		}
		break;

	case CMD_SUNDOWN_ANGLE:
		// 0 or 100..10000
		if (RegValue < 90)
		{
			vars.hwinfo.sun_down_angle = RegValue;
			WriteStruct2Flash(&vars.hwinfo, sizeof(hw_info_t));
		}
		break;

	case CMD_MINUTES_HIGH:
		// No set possible
		break;

	case CMD_MINUTES_LOW:
		break;

	case CMD_GPS_VALID:
		// No set possible
		break;

	case CMD_MAX_HOR_HIGH:
		// No set possible
		break;

	case CMD_MAX_HOR_LOW:
		// No set possible
		break;

	case CMD_MAX_VER_HIGH:
		// No set possible
		break;

	case CMD_MAX_VER_LOW:
		// No set possible
		break;

	case CMD_CALIBRATE_HOR:
		switch (RegValue)
		{
			case 0:
				vars.eevar.main_mode = ST_STOP;
				vars.goto_motor = vars.eevar.actual_motor;
			break;
			case 1:
				cmd_set_end_ccw();
				break;
			case 2:
				cmd_set_end_cw();
				break;

			default:
				break;
		}
		break;

	case CMD_CALIBRATE_VER:
		switch (RegValue)
		{
			case 0:
				vars.eevar.main_mode = ST_STOP;
				vars.goto_motor = vars.eevar.actual_motor;
			break;
			case 1:
				cmd_set_end_down();
				break;
			case 2:
				cmd_set_end_up();
				break;

			default:
				break;
		}
		break;

		case CMD_DEVIATION_HOR:
			// No set possible
			break;
		case CMD_DEVIATION_VER:
			// No set possible
			break;

	}

	// remenber previous values for 32 bit registers
	Prev_RegIndex = RegIndex;;
	Prev_RegValue = RegValue;
}

/**
 * @brief  This function is push and pulls the register data
 * @param  RegBuffer	- pointer to data to send or receive
 * @param  Address		- register number
 * @param  NRegs		- number of registers
 * @param  eMode		- read or write mode
 * @retval MB_ENOERR	- o OK
 */
SMBErrorCode_t MBRegHoldingCB(uint8_t * RegBuffer, uint16_t Address, uint8_t NRegs, SLMBRegister_t eMode)
{
	SMBErrorCode_t eStatus = SLMB_OK;
	uint16_t RegIndex;
	uint16_t RegVal;
	uint16_t temp;

// No pointer to data received
	if (RegBuffer == NULL)
		return SLMB_NOREGISTER;

// asked register is in range
	if ((Address >= REG_HOLDING_START) && (Address + NRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
	{
		RegIndex = (uint16_t) (Address - REG_HOLDING_START);
		switch (eMode)
		{
		/* Pass current register values to the protocol stack. */
		case SLMB_REG_READ:
			while (NRegs > 0)
			{
				// Get the value of register #RegIndex
				temp = modbus_getvalue(RegIndex);
				*RegBuffer++ = (uint8_t) (temp >> 8);
				*RegBuffer++ = (uint8_t) (temp & 0xFF);

//				*RegBuffer++ = (uint8_t) (RegHoldingBuf[RegIndex] >> 8);
//				*RegBuffer++ = (uint8_t) (RegHoldingBuf[RegIndex] & 0xFF);
				RegIndex++;
				NRegs--;
			}
			break;

			/* Update current register values with new values from the
			 * protocol stack. */
		case SLMB_REG_WRITE:
			while (NRegs > 0)
			{
				RegVal = ((RegBuffer[0] << 8) | RegBuffer[1]) & 0xFFFF;
				RegBuffer += 2;
				// Set the value of register #RegIndex
//			modbus_setvalue(RegIndex, RegHoldingBuf[RegIndex]);
				modbus_setvalue(RegIndex, RegVal);

//				RegHoldingBuf[RegIndex] = *RegBuffer++ << 8;
//				RegHoldingBuf[RegIndex] |= *RegBuffer++;
				RegIndex++;
				NRegs--;
			}
		}
	}
	else
	{
		eStatus = SLMB_NOREGISTER;
	}
	return eStatus;
}


