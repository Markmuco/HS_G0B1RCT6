/*
 * ayct102.h
 *
 *  Created on: 2 okt. 2016
 *      Author: Mark
 */

#ifndef AYCT102_H_
#define AYCT102_H_

// constants for decoding
#define AYCT_ADR   0xFFFFFF00
#define AYCT_SLIDE 0x0C
#define AYCT_KEY   0x33

//#define BITS_MINIMUM 210 //150 // actual measured time 270us
//#define BITS_MAXIMUM 325
//
//#define BITL_MINIMUM 1050 // actual measured time 1300us
//#define BITL_MAXIMUM 1600

#define BITS_MINIMUM 150 //150 // actual measured time 270us
#define BITS_MAXIMUM 400

#define BITL_MINIMUM 900 // actual measured time 1300us
#define BITL_MAXIMUM 2000

#define NO_DATA			0
#define SHORT_LOW		1
#define LONG_LOW		3
#define SHORT_HIGH		10

#define bit_set(D,i) ((D) |= (0x01 << i))
#define bit_clear(D,i) (D &= ~(0x01 << i))

typedef enum
{
	KEY1_ON =	4,
	KEY1_OFF =	0,
	KEY2_ON	=	5,
	KEY2_OFF =	1,
	KEY3_ON =	6,
	KEY3_OFF =	2,
	KEY4_ON =	7,
	KEY4_OFF =	3,
	KEYG_ON =	12,
	KEYG_OFF =	8,
	KEY_NONE = 0xFF
} ayct102_key_t;


typedef enum
{
	SLIDER_I =	0,
	SLIDER_II =	1,
	SLIDER_III=	2,
	SLIDER_VI =	3
} ayct102_slide_t;

typedef struct
{
		uint32_t home;
		ayct102_slide_t slide;
		ayct102_key_t key;
} ayct102_t;


bool get_ayct(ayct102_t * ayct102);
void AYCT_EXTI_IRQHandler(void);
void AYCT_TIM_IRQHandler(void);

#endif /* AYCT102_H_ */
