/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DIS_BRIDGE_GPIO_Port, DIS_BRIDGE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_DB4_Pin|LCD_DB5_Pin|LCD_DB6_Pin|DIR6_Pin
                          |LCD_DB7_Pin|LCD_RS_Pin|LCD_E_Pin|LCD_RW_Pin
                          |LCD_BACK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, EN_X_Pin|EN_Y_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : nGPS_EZN_Pin EN_X_Pin EN_Y_Pin */
  GPIO_InitStruct.Pin = nGPS_EZN_Pin|EN_X_Pin|EN_Y_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : END_X_Pin */
  GPIO_InitStruct.Pin = END_X_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(END_X_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MY_A_Pin MY_B_Pin MX_A_Pin MX_B_Pin
                           RX433_Pin */
  GPIO_InitStruct.Pin = MY_A_Pin|MY_B_Pin|MX_A_Pin|MX_B_Pin
                          |RX433_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : END_Y_Pin FAULT1_Pin MID_X_Pin */
  GPIO_InitStruct.Pin = END_Y_Pin|FAULT1_Pin|MID_X_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DIS_BRIDGE_Pin */
  GPIO_InitStruct.Pin = DIS_BRIDGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DIS_BRIDGE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FAULT2_Pin */
  GPIO_InitStruct.Pin = FAULT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(FAULT2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DB4_Pin LCD_DB5_Pin LCD_DB6_Pin DIR6_Pin
                           LCD_DB7_Pin LCD_RS_Pin LCD_E_Pin LCD_RW_Pin
                           LCD_BACK_Pin */
  GPIO_InitStruct.Pin = LCD_DB4_Pin|LCD_DB5_Pin|LCD_DB6_Pin|DIR6_Pin
                          |LCD_DB7_Pin|LCD_RS_Pin|LCD_E_Pin|LCD_RW_Pin
                          |LCD_BACK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IN_1_Pin MID_Y_Pin */
  GPIO_InitStruct.Pin = IN_1_Pin|MID_Y_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 2 */

void gps_power(bool on)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//n is set as input pull low
	if (on)
	{
		  /*Configure GPIO pin : PtPin */
		  GPIO_InitStruct.Pin = nGPS_EZN_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(nGPS_EZN_GPIO_Port, &GPIO_InitStruct);
	}
	else
	{
		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(nGPS_EZN_GPIO_Port, nGPS_EZN_Pin, GPIO_PIN_SET);

		  /*Configure GPIO pin : PtPin */
		  GPIO_InitStruct.Pin = nGPS_EZN_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(nGPS_EZN_GPIO_Port, &GPIO_InitStruct);
	}
}

/* USER CODE END 2 */
