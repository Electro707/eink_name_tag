/*
 * paper.h
 *
 *  Created on: Dec 25, 2019
 *      Author: electro
 */

#ifndef PAPER_H_
#define PAPER_H_

#include <stdlib.h>
#include "main.h"

// Display resolution
#define Paper_WIDTH       104
#define Paper_HEIGHT      212

// EPD2IN13 commands
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F


#define Paper_CS_0     EPAPER_CS_GPIO_Port->ODR &= ~(EPAPER_CS_Pin)
#define Paper_CS_1     EPAPER_CS_GPIO_Port->ODR |= (EPAPER_CS_Pin)

#define Paper_DC_0     EPAPER_DC_GPIO_Port->ODR &= ~(EPAPER_DC_Pin)
#define Paper_DC_1     EPAPER_DC_GPIO_Port->ODR |= (EPAPER_DC_Pin)

#define Paper_RST_0     EPAPER_RST_GPIO_Port->ODR &= ~(EPAPER_RST_Pin)
#define Paper_RST_1     EPAPER_RST_GPIO_Port->ODR |= (EPAPER_RST_Pin)

#define Paper_BUSY_RD   ((LL_GPIO_ReadInputPort(EPAPER_BUSY_GPIO_Port) >> 2)&0b1)

#define SPI_Write_Byte(__DATA) SPI1->DR = __DATA; while( (SPI1->SR & (1<<7)) != 0)
//#define SPI_Read_Byte(__DATA) SPI1->DR = __DATA; while( (SPI1->SR & (1<<0)) == 0); return SPI1->DR


void Paper_Reset(void);
void Paper_SendSingleCommand(uint8_t Reg);
void Paper_SendCommand(uint8_t Reg);
void Paper_SendData(uint8_t Data);
void Paper_WaitUntilIdle(void);
void Paper_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);
void Paper_SetCursor(uint16_t Xstart, uint16_t Ystart);
void Paper_TurnOnDisplay(void);
uint8_t Paper_Init();
void Paper_Clear(void);
void Paper_Display(uint8_t *buffer);
void Paper_Sleep(void);
void Paper_SoftwareReset(void);
void Paper_WriteLUT(void);

#endif /* INC_PAPER_H_ */
