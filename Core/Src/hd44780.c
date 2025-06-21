/*
 * hd44780.c
 *
 *  Created on: 13 aug. 2019
 *      Author: Mark
 */

#include "main.h"
#include "hd44780.h"


/* Private functions */
static void HD44780_InitPins(void);
static void HD44780_Cmd(uint8_t cmd);
static void HD44780_Cmd4bit(uint8_t cmd);
static void HD44780_Data(uint8_t data);
static void Delay_us(uint32_t us);

/* Private variable */
static HD44780_Options_t HD44780_Opts;


void dsp_init(void)
{
	HD44780_Init(20, 4);
	HD44780_CreateChar(CHAR_NONE, (uint8_t*) noneChar);
	HD44780_CreateChar(CHAR_GPS, (uint8_t*) gpsChar);
}

/**
 * @brief  Initializes HD44780 LCD
 * @brief  cols: width of lcd
 * @param  rows: height of lcd
 * @retval None
 */
void HD44780_Init(uint8_t cols, uint8_t rows)
{
	/* Init pinout */
	HD44780_InitPins();

	/* At least 40ms */
	HAL_Delay(45);

	/* Set LCD width and height */
	HD44780_Opts.Rows = rows;
	HD44780_Opts.Cols = cols;

	/* Set cursor pointer to beginning for LCD */
	HD44780_Opts.currentX = 0;
	HD44780_Opts.currentY = 0;

	HD44780_Opts.DisplayFunction = HD44780_4BITMODE | HD44780_5x8DOTS | HD44780_1LINE;
	if (rows > 1)
	{
		HD44780_Opts.DisplayFunction |= HD44780_2LINE;
	}

	/* Try to set 4bit mode */
	HD44780_Cmd4bit(0x03);
	HAL_Delay(45);

	/* Second try */
	HD44780_Cmd4bit(0x03);
	HAL_Delay(45);

	/* Third goo! */
	HD44780_Cmd4bit(0x03);
	HAL_Delay(45);

	/* Set 4-bit interface */
	HD44780_Cmd4bit(0x02);
	HAL_Delay(5);

	/* Set # lines, font size, etc. */
	HD44780_Cmd(HD44780_FUNCTIONSET | HD44780_Opts.DisplayFunction);

	/* Turn the display on with no cursor or blinking default */
	HD44780_Opts.DisplayControl = HD44780_DISPLAYON;
	HD44780_DisplayOn();

	/* Clear lcd */
	HD44780_Clear();

	/* Default font directions */
	HD44780_Opts.DisplayMode = HD44780_ENTRYLEFT | HD44780_ENTRYSHIFTDECREMENT;
	HD44780_Cmd(HD44780_ENTRYMODESET | HD44780_Opts.DisplayMode);

	/* Delay */
	HAL_Delay(45);
}

/**
 * @brief  Clears entire LCD
 * @param  None
 * @retval None
 */
void HD44780_Clear(void)
{
	HD44780_Cmd(HD44780_CLEARDISPLAY);
	HAL_Delay(4);
}

/**
 * @brief  Puts string on lcd
 * @param  x location
 * @param  y location
 * @param  *str: pointer to string to display
 * @retval None
 */
void HD44780_Puts(uint8_t x, uint8_t y, char* str)
{
	HD44780_CursorSet(x, y);
	while (*str)
	{
		if (HD44780_Opts.currentX >= HD44780_Opts.Cols)
		{
			HD44780_Opts.currentX = 0;
			HD44780_Opts.currentY++;
			HD44780_CursorSet(HD44780_Opts.currentX, HD44780_Opts.currentY);
		}
		if (*str == '\n')
		{
			HD44780_Opts.currentY++;
			HD44780_CursorSet(HD44780_Opts.currentX, HD44780_Opts.currentY);
		}
		else if (*str == '\r')
		{
			HD44780_CursorSet(0, HD44780_Opts.currentY);
		}
		else
		{
			HD44780_Data(*str);
			HD44780_Opts.currentX++;
		}
		str++;
	}
}

/**
 * @brief  Turn display on
 * @param  None
 * @retval None
 */
void HD44780_DisplayOn(void)
{
	HD44780_Opts.DisplayControl |= HD44780_DISPLAYON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Turn display off
 * @param  None
 * @retval None
 */
void HD44780_DisplayOff(void)
{
	HD44780_Opts.DisplayControl &= ~HD44780_DISPLAYON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Enables cursor blink
 * @param  None
 * @retval None
 */
void HD44780_BlinkOn(void)
{
	HD44780_Opts.DisplayControl |= HD44780_BLINKON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Disables cursor blink
 * @param  None
 * @retval None
 */
void HD44780_BlinkOff(void)
{
	HD44780_Opts.DisplayControl &= ~HD44780_BLINKON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Shows cursor
 * @param  None
 * @retval None
 */
void HD44780_CursorOn(void)
{
	HD44780_Opts.DisplayControl |= HD44780_CURSORON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Hides cursor
 * @param  None
 * @retval None
 */
void HD44780_CursorOff(void)
{
	HD44780_Opts.DisplayControl &= ~HD44780_CURSORON;
	HD44780_Cmd(HD44780_DISPLAYCONTROL | HD44780_Opts.DisplayControl);
}

/**
 * @brief  Scrolls display to the left
 * @param  None
 * @retval None
 */
void HD44780_ScrollLeft(void)
{
	HD44780_Cmd(HD44780_CURSORSHIFT | HD44780_DISPLAYMOVE | HD44780_MOVELEFT);
}

/**
 * @brief  Scrolls display to the right
 * @param  None
 * @retval None
 */
void HD44780_ScrollRight(void)
{
	HD44780_Cmd(HD44780_CURSORSHIFT | HD44780_DISPLAYMOVE | HD44780_MOVERIGHT);
}

/**
 * @brief  Creates custom character
 * @param  location: Location where to save character on LCD. LCD supports up to 8 custom characters, so locations are 0 - 7
 * @param *data: Pointer to 8-bytes of data for one character
 * @retval None
 */
void HD44780_CreateChar(uint8_t location, uint8_t *data)
{
	uint8_t i;
	/* We have 8 locations available for custom characters */
	location &= 0x07;
	HD44780_Cmd(HD44780_SETCGRAMADDR | (location << 3));

	for (i = 0; i < 8; i++)
	{
		HD44780_Data(data[i]);
	}
}

/**
 * @brief  Puts custom created character on LCD
 * @param  location: Location on LCD where character is stored, 0 - 7
 * @retval None
 */
void HD44780_PutCustom(uint8_t x, uint8_t y, uint8_t location)
{
	HD44780_CursorSet(x, y);
	HD44780_Data(location);
}

/* Private functions */

static void Delay_us(uint32_t us)
{
#if 0
	HAL_Delay(us);
#else
	/* So (2^32)/12 micros max, or less than 6 minutes */
	us *= 40; //12
	// TODO value ok??
	us -= 2; //offset seems around 2 cycles
	/* fudge for function call overhead */
	us--;
	__ASM volatile(" mov r0, %[us] \n\t"
			".syntax unified \n\t"
			"1: subs r0, #1 \n\t"
			".syntax divided \n\t"
			" bhi 1b \n\t"
			:
			: [us] "r" (us)
			: "r0");
#endif
}

static void HD44780_Cmd(uint8_t cmd)
{
	/* Command mode */
	LCD_RS_0;
	/* High nibble */
	HD44780_Cmd4bit(cmd >> 4);
	/* Low nibble */
	HD44780_Cmd4bit(cmd & 0x0F);
}

static void HD44780_Data(uint8_t data)
{
	/* Data mode */
	LCD_RS_1;
	/* High nibble */
	HD44780_Cmd4bit(data >> 4);
	/* Low nibble */
	HD44780_Cmd4bit(data & 0x0F);
}

static void HD44780_Cmd4bit(uint8_t cmd)
{
	/* Set output port*/
	HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, (cmd & 0x08));
	HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, (cmd & 0x04));
	HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, (cmd & 0x02));
	HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, (cmd & 0x01));

	//LCD_DB4_GPIO_Port->ODR = (LCD_DB4_GPIO_Port->ODR & 0xFFFFFFF0) | cmd;

	LCD_E_1;
	Delay_us(20);
	LCD_E_0;
	Delay_us(20);

}

/**
 * @defgroup TM_HD44780_Functions
 * @brief    Library Functions
 * @{
 */
void HD44780_CursorSet(uint8_t col, uint8_t row)
{
	uint8_t row_offsets[] =
	{ 0x00, 0x40, 0x14, 0x54 };

	/* Go to beginning */
	if (row >= HD44780_Opts.Rows)
	{
		row = 0;
	}

	/* Set current column and row */
	HD44780_Opts.currentX = col;
	HD44780_Opts.currentY = row;

	/* Set location address */
	HD44780_Cmd(HD44780_SETDDRAMADDR | (col + row_offsets[row]));
}

static void HD44780_InitPins(void)
{
	/* Set pins low*/
	LCD_RS_0;
	LCD_E_0;
	LCD_RW_0;
	HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_RESET);
}
