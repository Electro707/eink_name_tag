/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "paper.h"
#include "24FC64F.h"
#define DISPLAY_ON POWER_SWITCH_GPIO_Port->ODR &= ~(POWER_SWITCH_Pin)
#define DISPLAY_OFF POWER_SWITCH_GPIO_Port->ODR |= (POWER_SWITCH_Pin)

// The section size is really dependent on USB CDC's write capabilities which is best done
// 64 bytes at a time
#define SECTION_SIZE 64
// The numbmer of "sections" to make up a single frame, in this case (64*44) = 22528 bits.
// The display has 212*104=22048 bits
#define SECTION_PER_FRAME 44
#define BYTES_PER_FRAME SECTION_SIZE * SECTION_PER_FRAME
// A section from the start of the EEPROM reserved for user settings.
// This should be a multiple of 32 due to the EEPROM's page write size
#define EEPROM_USER_SETTING_SIZE 32

uint8_t current_frame = 0;
uint8_t display_buffer[BYTES_PER_FRAME];
uint8_t version_string[] = "Epaper Rev C\n";


int usb_asking_numb_data = 0;
uint8_t usb_asking_for = 0;
uint8_t ee_write_frame = 0;
uint8_t ee_write_section = 0;

uint8_t handle_loop_extra_stuff = 0;
uint8_t handle_main_loop = 0;

void handle_usb_packet(void);
void handle_usb_extra_data(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
UserSettings_t user_settings = {
	.numb_of_frames = 3,
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
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
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
//  get_setting_from_ee();
  DISPLAY_ON;
  LL_SPI_Enable(SPI1);
  Paper_Init();
  Paper_Clear(); Paper_TurnOnDisplay();
  Paper_Sleep();
  DISPLAY_OFF;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(handle_loop_extra_stuff == 1){
	  NVIC_DisableIRQ(USB_IRQn);
	  handle_usb_extra_data();
	  NVIC_EnableIRQ(USB_IRQn);
	  handle_loop_extra_stuff = 0;
	}
	else if(handle_loop_extra_stuff == 2){
	  NVIC_DisableIRQ(USB_IRQn);
	  display_frame_number(ee_write_frame);
	  NVIC_EnableIRQ(USB_IRQn);
	  handle_loop_extra_stuff = 0;
	}

	if(handle_main_loop == 1){
	  NVIC_DisableIRQ(USB_IRQn);
	  handle_usb_packet();
	  NVIC_EnableIRQ(USB_IRQn);
	  handle_main_loop = 0;
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_USB;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
  LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
  LL_CRC_SetInitialData(CRC, 0);
  LL_CRC_SetPolynomialCoef(CRC, 79764919);
  LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00707CBB;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(POWER_SWITCH_GPIO_Port, POWER_SWITCH_Pin);

  /**/
  LL_GPIO_ResetOutputPin(EPAPER_RST_GPIO_Port, EPAPER_RST_Pin);

  /**/
  LL_GPIO_ResetOutputPin(EPAPER_CS_GPIO_Port, EPAPER_CS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(EPAPER_DC_GPIO_Port, EPAPER_DC_Pin);

  /**/
  GPIO_InitStruct.Pin = POWER_SWITCH_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(POWER_SWITCH_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = EPAPER_BUSY_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(EPAPER_BUSY_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = EPAPER_RST_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(EPAPER_RST_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = EPAPER_CS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(EPAPER_CS_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = EPAPER_DC_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(EPAPER_DC_GPIO_Port, &GPIO_InitStruct);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE8);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE9);

  /**/
  LL_GPIO_SetPinPull(BUTTON1_GPIO_Port, BUTTON1_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinPull(BUTTON2_GPIO_Port, BUTTON2_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(BUTTON1_GPIO_Port, BUTTON1_Pin, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(BUTTON2_GPIO_Port, BUTTON2_Pin, LL_GPIO_MODE_INPUT);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_8;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_9;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI4_15_IRQn, 0);
  NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */
/**
 * Function that gets settings stored in EEPROM, such as the nummber of
 */
void get_setting_from_ee(){
	uint8_t ee_user_data[EEPROM_USER_SETTING_SIZE];
	ee24fc64_multi_read_read(ee_user_data, EEPROM_USER_SETTING_SIZE, 0);
	user_settings.numb_of_frames = ee_user_data[0];
}

void store_settings_to_ee(){
	uint8_t ee_user_data[EEPROM_USER_SETTING_SIZE];
	memset(ee_user_data, 0, EEPROM_USER_SETTING_SIZE);	// Making sure the user data written starts out empty
	ee_user_data[0] = user_settings.numb_of_frames;

	uint16_t start_address = 0;
	for(int i=0;i<EEPROM_USER_SETTING_SIZE/32;i++){
		ee24fc64_multi_write(&UserRxBufferFS[(32*i)], 32, start_address);
		start_address += 32;
	}
}

void display_frame_number(uint8_t frame_number){
	uint16_t start_address = frame_number*BYTES_PER_FRAME + EEPROM_USER_SETTING_SIZE;
	for(int i=0;i<SECTION_PER_FRAME;i++){
		ee24fc64_multi_read_read(&display_buffer[i*SECTION_SIZE], SECTION_SIZE, start_address);
		start_address += SECTION_SIZE;
	}
	DISPLAY_ON; Paper_Init();
	Paper_Display(display_buffer);
	Paper_Sleep();
	DISPLAY_OFF;
}


void handle_usb_packet(){
	uint8_t frame, section;
	uint16_t start_address;
	if(UserRxBufferFS[0] == 0x02){
		ee_write_frame = UserRxBufferFS[1];
		ee_write_section = UserRxBufferFS[2];
		usb_asking_for = 1;
		usb_asking_numb_data = SECTION_SIZE;
		UserTxBufferFS[0] = 0xFE; UserTxBufferFS[1] = '\n';
		CDC_Transmit_FS(UserTxBufferFS, 2);
	}
	else if(UserRxBufferFS[0] == 0x03){
		frame = UserRxBufferFS[1];
		section = UserRxBufferFS[2];
		start_address = (frame*BYTES_PER_FRAME) + (section*SECTION_SIZE) + EEPROM_USER_SETTING_SIZE;
		memset(&UserTxBufferFS[0], 0, SECTION_SIZE);
		ee24fc64_multi_read_read(&UserTxBufferFS[0], SECTION_SIZE, start_address);
		CDC_Transmit_FS(UserTxBufferFS, SECTION_SIZE);
	} else if(UserRxBufferFS[0] == 'v'){
		CDC_Transmit_FS(version_string, sizeof(version_string)-1);
	} else if(UserRxBufferFS[0] == 0x20){
		ee_write_frame = UserRxBufferFS[1];
		handle_loop_extra_stuff = 2;
	}
}

void handle_usb_extra_data(){
	uint32_t crc; uint16_t start_address;
	if(usb_asking_for == 1){
		LL_CRC_SetInitialData(CRC, 0);
		start_address = (ee_write_frame*BYTES_PER_FRAME) + (ee_write_section*SECTION_SIZE) + EEPROM_USER_SETTING_SIZE;
		for(int i=0;i<SECTION_SIZE/32;i++){
			ee24fc64_multi_write(&UserRxBufferFS[(32*i)], 32, start_address);
			for(int f=0;f<32;f++){LL_CRC_FeedData8(CRC, UserRxBufferFS[(32*i)+f]);}
			start_address += 32;
		}
		crc = LL_CRC_ReadData32(CRC);
		crc = ~crc;
		memcpy(&UserTxBufferFS[0], &crc, 4); UserTxBufferFS[4] = '\n';
		CDC_Transmit_FS(UserTxBufferFS, 5);
	}
	usb_asking_for = 0;
	usb_asking_numb_data = 0;
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
  __disable_irq();
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
