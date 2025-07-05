/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "vars.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern vars_t vars;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define nGPS_EZN_Pin GPIO_PIN_11
#define nGPS_EZN_GPIO_Port GPIOC
#define END_X_Pin GPIO_PIN_13
#define END_X_GPIO_Port GPIOC
#define MY_A_Pin GPIO_PIN_0
#define MY_A_GPIO_Port GPIOC
#define MY_A_EXTI_IRQn EXTI0_1_IRQn
#define MY_B_Pin GPIO_PIN_1
#define MY_B_GPIO_Port GPIOC
#define MY_B_EXTI_IRQn EXTI0_1_IRQn
#define MX_A_Pin GPIO_PIN_2
#define MX_A_GPIO_Port GPIOC
#define MX_A_EXTI_IRQn EXTI2_3_IRQn
#define MX_B_Pin GPIO_PIN_3
#define MX_B_GPIO_Port GPIOC
#define MX_B_EXTI_IRQn EXTI2_3_IRQn
#define END_Y_Pin GPIO_PIN_1
#define END_Y_GPIO_Port GPIOA
#define DIS_BRIDGE_Pin GPIO_PIN_4
#define DIS_BRIDGE_GPIO_Port GPIOA
#define FAULT1_Pin GPIO_PIN_5
#define FAULT1_GPIO_Port GPIOA
#define FAULT2_Pin GPIO_PIN_6
#define FAULT2_GPIO_Port GPIOA
#define RX433_Pin GPIO_PIN_4
#define RX433_GPIO_Port GPIOC
#define RX433_EXTI_IRQn EXTI4_15_IRQn
#define LCD_DB4_Pin GPIO_PIN_0
#define LCD_DB4_GPIO_Port GPIOB
#define LCD_DB5_Pin GPIO_PIN_1
#define LCD_DB5_GPIO_Port GPIOB
#define LCD_DB6_Pin GPIO_PIN_2
#define LCD_DB6_GPIO_Port GPIOB
#define IN_1_Pin GPIO_PIN_12
#define IN_1_GPIO_Port GPIOB
#define DIR6_Pin GPIO_PIN_14
#define DIR6_GPIO_Port GPIOB
#define MID_Y_Pin GPIO_PIN_15
#define MID_Y_GPIO_Port GPIOB
#define DIR_Y_Pin GPIO_PIN_6
#define DIR_Y_GPIO_Port GPIOC
#define MID_X_Pin GPIO_PIN_15
#define MID_X_GPIO_Port GPIOA
#define DIR_X_Pin GPIO_PIN_8
#define DIR_X_GPIO_Port GPIOC
#define LCD_DB7_Pin GPIO_PIN_3
#define LCD_DB7_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_4
#define LCD_RS_GPIO_Port GPIOB
#define LCD_E_Pin GPIO_PIN_5
#define LCD_E_GPIO_Port GPIOB
#define LCD_RW_Pin GPIO_PIN_6
#define LCD_RW_GPIO_Port GPIOB
#define LCD_BACK_Pin GPIO_PIN_7
#define LCD_BACK_GPIO_Port GPIOB
#define PWM_CONTRAST_Pin GPIO_PIN_10
#define PWM_CONTRAST_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */



#define CS_MOTOR_1		HAL_GPIO_WritePin(CS_HBRIDGE_GPIO_Port, CS_HBRIDGE_Pin, GPIO_PIN_SET)
#define CS_MOTOR_0		HAL_GPIO_WritePin(CS_HBRIDGE_GPIO_Port, CS_HBRIDGE_Pin, GPIO_PIN_RESET)

#define LCD_RW_1		HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_SET)
#define LCD_RW_0		HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET)

#define LCD_RS_1		HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET)
#define LCD_RS_0		HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET)

#define LCD_E_1			HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET)
#define LCD_E_0			HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET)

#define LCD_BACK_ON		HAL_GPIO_WritePin(LCD_BACK_GPIO_Port, LCD_BACK_Pin, GPIO_PIN_SET)
#define LCD_BACK_OFF	HAL_GPIO_WritePin(LCD_BACK_GPIO_Port, LCD_BACK_Pin, GPIO_PIN_RESET)

#define BRIDGE_DRIVE_DIS	HAL_GPIO_WritePin(DIS_BRIDGE_GPIO_Port, DIS_BRIDGE_Pin, GPIO_PIN_SET)
#define BRIDGE_DRIVE_EN		HAL_GPIO_WritePin(DIS_BRIDGE_GPIO_Port, DIS_BRIDGE_Pin, GPIO_PIN_RESET)

#define GPS_0			HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_SET)
#define GPS_1			HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_RESET)

#define DIR_X_0			HAL_GPIO_WritePin(DIR_X_GPIO_Port, DIR_X_Pin, GPIO_PIN_SET)
#define DIR_X_1			HAL_GPIO_WritePin(DIR_X_GPIO_Port, DIR_X_Pin, GPIO_PIN_RESET)

#define DIR_Y_0			HAL_GPIO_WritePin(DIR_Y_GPIO_Port, DIR_Y_Pin, GPIO_PIN_SET)
#define DIR_Y_1			HAL_GPIO_WritePin(DIR_Y_GPIO_Port, DIR_Y_Pin, GPIO_PIN_RESET)

// not used, for powering on the GPS pin is set to input with pull down, the display crapped because of the inrush peak
#define GPS_0_OFF			HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_SET)
#define GPS_1_ON			HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_RESET)

#define LEDD2_1		HAL_GPIO_WritePin(TP1_GPIO_Port, TP1_Pin, GPIO_PIN_SET)
#define LEDD2_0		HAL_GPIO_WritePin(TP1_GPIO_Port, TP1_Pin, GPIO_PIN_RESET)
#define LEDD2_TOG	HAL_GPIO_TogglePin(TP1_GPIO_Port, TP1_Pin)

#define  isMX_A		HAL_GPIO_ReadPin(MX_A_GPIO_Port, MX_A_Pin)
#define  isMX_B		HAL_GPIO_ReadPin(MX_B_GPIO_Port, MX_B_Pin)
#define  isMY_A		HAL_GPIO_ReadPin(MY_A_GPIO_Port, MY_A_Pin)
#define  isMY_B		HAL_GPIO_ReadPin(MY_B_GPIO_Port, MY_B_Pin)

#define  isEND_X	(!HAL_GPIO_ReadPin(END_X_GPIO_Port, END_X_Pin))
#define  isEND_Y	(!HAL_GPIO_ReadPin(END_Y_GPIO_Port, END_Y_Pin))

#define  isMID_X	(!HAL_GPIO_ReadPin(MID_X_GPIO_Port, MID_X_Pin))
#define  isMID_Y	(!HAL_GPIO_ReadPin(MID_Y_GPIO_Port, MID_Y_Pin))

#define  is433_INT 		HAL_GPIO_ReadPin(RX433_GPIO_Port, RX433_Pin)
#define  isEXT_IN_1		(!HAL_GPIO_ReadPin(IN_1_GPIO_Port, IN_1_Pin))

#define  isBRIGE_OFF	HAL_GPIO_ReadPin(DIS_BRIDGE_GPIO_Port, DIS_BRIDGE_Pin)
#define  isDISPLAY_ON	HAL_GPIO_ReadPin(LCD_BACK_GPIO_Port, LCD_BACK_Pin)
#define  isGPS_ON		(!HAL_GPIO_ReadPin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin))
#define  isFAULT_1		(HAL_GPIO_ReadPin(FAULT1_GPIO_Port, FAULT1_Pin))
#define  isFAULT_2		(HAL_GPIO_ReadPin(FAULT2_GPIO_Port, FAULT2_Pin))


#define LOW_16B(x)     ((x)&0xFFFF)
#define HIGH_16B(x)    (((x)>>16)&0xFFFF)

void _Error_Handler(char *file, int line);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
