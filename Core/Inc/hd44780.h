/*
 * hd44780.h
 *
 *  Created on: 13 aug. 2019
 *      Author: Mark
 */

#ifndef HD44780_H_
#define HD44780_H_

/* Commands*/
#define HD44780_CLEARDISPLAY        0x01
#define HD44780_RETURNHOME          0x02
#define HD44780_ENTRYMODESET        0x04
#define HD44780_DISPLAYCONTROL      0x08
#define HD44780_CURSORSHIFT         0x10
#define HD44780_FUNCTIONSET         0x20
#define HD44780_SETCGRAMADDR        0x40
#define HD44780_SETDDRAMADDR        0x80

/* Flags for display entry mode */
#define HD44780_ENTRYRIGHT          0x00
#define HD44780_ENTRYLEFT           0x02
#define HD44780_ENTRYSHIFTINCREMENT 0x01
#define HD44780_ENTRYSHIFTDECREMENT 0x00

/* Flags for display on/off control */
#define HD44780_DISPLAYON           0x04
#define HD44780_CURSORON            0x02
#define HD44780_BLINKON             0x01

/* Flags for display/cursor shift */
#define HD44780_DISPLAYMOVE         0x08
#define HD44780_CURSORMOVE          0x00
#define HD44780_MOVERIGHT           0x04
#define HD44780_MOVELEFT            0x00

/* Flags for function set */
#define HD44780_8BITMODE            0x10
#define HD44780_4BITMODE            0x00
#define HD44780_2LINE               0x08
#define HD44780_1LINE               0x00
#define HD44780_5x10DOTS            0x04
#define HD44780_5x8DOTS             0x00

typedef struct
{
    uint8_t DisplayControl;
    uint8_t DisplayFunction;
    uint8_t DisplayMode;
    uint8_t Rows;
    uint8_t Cols;
    uint8_t currentX;
    uint8_t currentY;

} HD44780_Options_t;



#define HD44780_Delay(x)        Delay_us(x)

void HD44780_CursorSet(uint8_t col, uint8_t row);
void HD44780_Init(uint8_t cols, uint8_t rows);
void HD44780_DisplayOn(void);
void HD44780_DisplayOff(void);
void HD44780_Clear(void);
void HD44780_Puts(uint8_t x, uint8_t y, char* str);
void HD44780_BlinkOn(void);
void HD44780_BlinkOff(void);
void HD44780_CursorOn(void);
void HD44780_CursorOff(void);
void HD44780_ScrollLeft(void);
void HD44780_ScrollRight(void);
void HD44780_CreateChar(uint8_t location, uint8_t* data);
void HD44780_PutCustom(uint8_t x, uint8_t y, uint8_t location);
void dsp_init(void);

#endif /* HD44780_H_ */
