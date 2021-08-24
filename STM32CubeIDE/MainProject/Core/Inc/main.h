/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stm32l0xx_hal.h"

#include "stm32l0xx_ll_crc.h"
#include "stm32l0xx_ll_i2c.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern int usb_asking_numb_data;
extern uint8_t handle_loop_extra_stuff;
extern uint8_t handle_main_loop;
extern uint8_t current_frame;
extern uint8_t max_frames;
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
enum USB_ASKING_FOR {
	enum_usb_askingfor_section_data = 1,
};
enum MAIN_LOOP_TODO {
	enum_main_todo_got_rx_data = 1,
	enum_main_todo_display_frame = 2,
};
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_SWITCH_Pin LL_GPIO_PIN_0
#define POWER_SWITCH_GPIO_Port GPIOA
#define EPAPER_BUSY_Pin LL_GPIO_PIN_2
#define EPAPER_BUSY_GPIO_Port GPIOA
#define EPAPER_RST_Pin LL_GPIO_PIN_3
#define EPAPER_RST_GPIO_Port GPIOA
#define EPAPER_CS_Pin LL_GPIO_PIN_4
#define EPAPER_CS_GPIO_Port GPIOA
#define EPAPER_DC_Pin LL_GPIO_PIN_6
#define EPAPER_DC_GPIO_Port GPIOA
#define BUTTON1_Pin LL_GPIO_PIN_8
#define BUTTON1_GPIO_Port GPIOA
#define BUTTON1_EXTI_IRQn EXTI4_15_IRQn
#define BUTTON2_Pin LL_GPIO_PIN_9
#define BUTTON2_GPIO_Port GPIOA
#define BUTTON2_EXTI_IRQn EXTI4_15_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
