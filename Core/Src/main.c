/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "fdcan.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "vars.h"
#include "shell.h"
#include "time.h"
#include "gps.h"
#include "motors.h"
#include "machine.h"
#include "remote.h"
#include "uart_sci.h"
#include "build_display.h"
#include "external_io.h"
#include "protection.h"
#ifdef ENABLE_MODBUS
#include "modbus_cmd.h"
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/*
 *
 *  _    _          _   _                 _                     _
 * | |  | |        | | (_)               | |                   | |
 * | |__| |   ___  | |  _    ___    ___  | |_    __ _    __ _  | |_
 * |  __  |  / _ \ | | | |  / _ \  / __| | __|  / _` |  / _` | | __|
 * | |  | | |  __/ | | | | | (_) | \__ \ | |_  | (_| | | (_| | | |_
 * |_|  |_|  \___| |_| |_|  \___/  |___/  \__|  \__,_|  \__,_|  \__|


 DESCRIPTION: Suntrack controller version 5.00

 REMARKS:

 arm-none-eabi-objcopy -O binary "${BuildArtifactFileBaseName}.elf" "${BuildArtifactFileBaseName}.bin" && arm-none-eabi-size "${BuildArtifactFileName}";${ProjDirPath}/writeCRC.exe "${BuildArtifactFileBaseName}.bin"

 - note: the CAN receiver has to be occupied, else startup delays


 NOTES
 CPU freq 48 Mhz
 Uart1 Onboard FTDI 115200
 Uart2 Modbus 115200
 Uart6 GPS 9600

 IRQ0	Y ENC_A
 IRQ1	Y ENC_B
 IRQ2	X ENC_A
 IRQ3	X ENC_A
 IRQ4 	RX_433
 IRQ12	BLE

 SPI0 3.0 mhz For motor debug
 I2C for eerom
 IWDT 40khz/4095/32 = 1.2 sec
 Timer3 PWM 20Khz 4ch Motors
 Timer16 PWM Remote control 5ms IRQ
 Timer17 contrast negative voltage generation


 TOOLS
 - CubeMX 6.1.1
 - AC6 System Workbench

 - HAL driver F0 V1.11.2

 PRINCIPE
 0x08000000 Bootloader 16K
 0x08004000 Application 24kb

 VERSION OVERVIEW:
 4.XX
 - early versions on Microchip 18F6722
 5.00 31-8-2019
 - fist edition
 - Save target screen change
 - manual adjust smaller jumps.
 5.01
 - disable UART2 when GPS poweroff
 5.02
 - enable remote after 5 second on ENDX/Y
 5.03
 - readout protection level 1
 5.04
 - tracking interval 10 seconds
 5.05
 - flexible hysterese
 5.06
 - counter clockwise mode 2 improved
 6.00
 - adoption for G070
 6.01
 -
 6.02
 - for both bridge drivers
 - solved the 4 missing chars of LCD
 - contrast default 30
 6.03
 - encoder debounce 1ms
 6.04
 - encoder debounce 2ms
 6.05
 - more windinfo on 'windpulse' command
 - only abort operating modes on maxwind.
 6.06
 - command targettime tg 23:00 for times actions
 - saving sun on target only if GPS sync
 - after factory sun and max first need to be set else 'please configure'
 - two remote controls possible
 - updated shell
 - target 1..5 by remote control 5..16 by command line
 6.07
 - added command turnback, the angle decidecrees it turn back at the end
 - optional Modbus, switched by changing the printing function pointer
 6.08
 - when max_windpulse is zero, disable wind function.
 6.09
 - at saving positions the maximum learned values has to be at least 10 decrees.
 - debouncing encoder uses not equal instead of +1ms for rollover risk
 - HAL Driver G0 V1.6.1
 - init disable bridge to off
 6.10
 - goto park not on sundown but on: out of range
 - limited motor power when goto park/ start of the day, default 50%
 6.11
 - debouncing the end switches
 - when position is out of learned values show calib hor/vert
 - global variables instead of pointer to them
 6.12
 - vars.max_pwm needs to be initialisized, starting the system with sundown went wrong
 6.13
 - Allow both GPRMC and GNRMC GPS/ GLONASS
 - no parking in Stop mode
 6.14
 - error on LCD if too long on end switch
 6.15
 - timers to 32, problem with zero detection
 6.16
 - modbus uses also 16 targets
 - more modbus commands
 7.00
 - moving to 256K micro and used Sampa for SUN and MOON calculation

 *
 */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

// Globals
vars_t vars;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void check_rdp_level(void);
static void printf_bridge_fault(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile const app_info_t __attribute__((section (".mySection"))) c_app_info =
{ .key = APP_KEY,    // Key
.version = VERSION,    // Version
.crc32 = 0xFFFFFFFF, // CCITT-CRC32
.size = 0xFFFFFFFF, // File size
.build_date = __DATE__,   // Build date
.build_time = __TIME__,   // Build time
. dummy =
{ 0xFF, 0xFF, 0xFF } };

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	extern uint32_t _main_app_start_address;
	uint32_t AppAddr = (uint32_t) &_main_app_start_address;
	uint32_t *ram_key = (uint32_t*) RAM_KEY;

	char ch;

	// Vector relocatie
	SCB->VTOR = AppAddr;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_I2C2_Init();
  MX_IWDG_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_FDCAN1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

	__enable_irq();

	wdt_clr();

	init_sci();
	timer_open();

	shell_open();

	sh_ver(NULL);

	// set protection level 1
	check_rdp_level();

	if (RCC->CSR & RCC_CSR_IWDGRSTF)
		tty_printf("WDT reboot\r\n");

	RCC->CSR = RCC_CSR_RMVF;

	if (sizeof(hw_info_t) > FLASH_PAGE_SIZE)
		tty_printf("  ERR flashvar size overflow\r\n");

	init_vars();

	// Timer for contrast
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	set_x_pwm(100);
	set_y_pwm(100);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	// AYCT102
	__HAL_TIM_SET_COUNTER(&htim16, 0);
	HAL_TIM_Base_Start_IT(&htim16); // Start Timer 10 IRQ

#ifdef RELEASE_BUILD
	// If serial number is valid continue

	if (!check_quick())
		tty_printf("  ERR Wrong firmware version\r\n");

	while (!check_quick())
		wdt_clr();

	if (stm_serial() != vars.hwinfo.stm_serial)
	{
		tty_printf("  ERR firmware corrupt\r\n");
		while (1)
			wdt_clr();
	}

#else
	set_protection();
	while (1)
	{
		HAL_Delay(10);
		wdt_clr();
	}

#endif

	//GPS_1;
	gps_power(true);

	*ram_key = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		wdt_clr();

		printf_bridge_fault();

		if (vars.gps_debug)
		{
			if (sci6_getch(&ch))
				sci1_putc(ch);
		}
		else
		{
			// get GPS data, true on SYNC
			if ((vars.gps_decode = gps_process(&vars.hwinfo.home_location)) == DECODING_RDY)
			{
				if (vars.eevar.main_mode == ST_WAIT_GPS)
				{
					// New GPS value, restore valid operating mode
					if (vars.store_main_mode > ST_STOP)
						vars.eevar.main_mode = ST_STOP;
					else
						vars.eevar.main_mode = vars.store_main_mode;

					if (vars.screen_tmr == NO_TIMER)
						vars.screen_tmr = timer_get();
					timer_start(vars.screen_tmr, SCREEN_ON_TIME, NULL);
					time_stamp();
					tty_printf("GPS SYNC\r\n");
				}
			}
		}

		// place in TFT library
		if (timer_elapsed(vars.screen_tmr) && (vars.gps_decode == DECODING_RDY) && (vars.screen_lcd != LCD_ERROR))
			LCD_BACK_OFF;

		// command shell
		shell_process();

		// calculate the motors
		motor_process();

		// remote control
		remote_ctrl_process();

		// main process
		machine_process();

		// 4x20 Display
		process_display();

		// Wind pulse counter, options
		external_io_functions();

#ifdef ENABLE_MODBUS
		process_modbus_slave();
#endif

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */





/**
 * @brief  This function is executed in case of error occurrence.
 * @param  file: The file name as string.
 * @param  line: The line in file as a number.
 * @retval None
 */
void _Error_Handler(char *file, int line)
{
	tty_printf("Error file %s line %d\r\n", file, line);
	while (1)
	{
	}
}

/**
 * Check if RDP Level is 1. Update if necessary.
 */
static void check_rdp_level(void)
{
	FLASH_OBProgramInitTypeDef config;

	HAL_FLASHEx_OBGetConfig(&config);
	if (config.RDPLevel != OB_RDP_LEVEL_1)
	{
		config.OptionType = OPTIONBYTE_RDP;
		config.RDPLevel = OB_RDP_LEVEL_1;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();

		if (HAL_FLASHEx_OBProgram(&config) == HAL_OK)
		{
			tty_printf("Set RDPLevel = %X\r\n", config.RDPLevel);
			HAL_FLASH_OB_Launch();
		}

		HAL_FLASH_OB_Lock();
	}
}


static void printf_bridge_fault(void)
{
	static bool old1=1, old2=1;
	static uint8_t fault_tmr = NO_TIMER;
	static uint8_t nfault_tmr = NO_TIMER;

	if (isFAULT_1 || isFAULT_2)
	{
		timer_free(&nfault_tmr);
		if (fault_tmr == NO_TIMER)
		{
			fault_tmr = timer_get();
			timer_start(fault_tmr, 500, NULL);
		}
	}
	else
	{
		timer_free(&fault_tmr);
		if (nfault_tmr == NO_TIMER)
		{
			nfault_tmr = timer_get();
			timer_start(nfault_tmr, 500, NULL);
		}
	}

	if (timer_elapsed(fault_tmr) || timer_elapsed(nfault_tmr))
	{
		if (old1 != isFAULT_1)
		{
			old1 = isFAULT_1;
			tty_printf(" ERR Fault1 %d\r\n", isFAULT_1);
		}
		if (old2 != isFAULT_2)
		{
			old2 = isFAULT_2;
			tty_printf(" ERR Fault2 %d\r\n", isFAULT_2);
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	printf("Error_Handler\r\n");

	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
