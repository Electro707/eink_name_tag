/*
 * paper.c
 *
 *  Created on: Dec 25, 2019
 *      Author: electro
 */

#include "paper.h"


const unsigned char lut[] = {
0xAA,	0x99,	0x10,	0x00,	0x00,	0x00,	0x00,	0x55,	0x99,	0x80,	0x00,	0x00,	0x00,	0x00,	0x8A,	0xA8,
0x9B,	0x00,	0x00,	0x00,	0x00,	0x8A,	0xA8,	0x9B,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x0F,	0x0F,	0x0F,	0x0F,	0x02,	0x14,	0x14,	0x14,	0x14,	0x06,	0x14,	0x14,	0x0C,
0x82,	0x08,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x00,	0x00,	0x00

/*
0xA5,	0x89,	0x10,	0x00,	0x00,	0x00,	0x00,	0xA5,	0x19,	0x80,	0x00,	0x00,	0x00,	0x00,	0xA5,	0xA9,
0x9B,	0x00,	0x00,	0x00,	0x00,	0xA5,	0xA9,	0x9B,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x0F,	0x0F,	0x0F,	0x0F,	0x02,	0x10,	0x10,	0x0A,	0x0A,	0x03,	0x08,	0x08,	0x09,
0x43,	0x07,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
0x00,	0x00,	0x00,	0x00,	0x00,	0x00
*/
};


void Paper_WriteLUT(void)
{

	Paper_SendCommand(WRITE_LUT_REGISTER);			// write LUT register
	for(int i=0;i<70;i++){			// write LUT register with 29bytes instead of 30bytes 2D13
		Paper_SendData(lut[i]);
	}
    Paper_CS_1;
}

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void Paper_Reset(void)
{
	Paper_DC_0;
	Paper_CS_1;

   Paper_RST_1;
   LL_mDelay(200);
    Paper_RST_0;
    LL_mDelay(200);
    Paper_RST_1;
    LL_mDelay(200);
}

/******************************************************************************
function :	send command
parameter:
    Reg : Command register
******************************************************************************/
void Paper_SendCommand(uint8_t Reg)
{
    Paper_DC_0;
    Paper_CS_0;
    SPI_Write_Byte(Reg);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void Paper_SendData(uint8_t Data)
{
    Paper_DC_1;
    Paper_CS_0;
    SPI_Write_Byte(Data);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void Paper_WaitUntilIdle(void)
{
    while(Paper_BUSY_RD == 1) {      //LOW: idle, HIGH: busy
    	LL_mDelay(50);
    }
}

/******************************************************************************
function :	Setting the display window
parameter:
******************************************************************************/
void Paper_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    Paper_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    Paper_SendData((Xstart >> 3) & 0xFF);
    Paper_SendData((Xend >> 3) & 0xFF);
    Paper_CS_1;

    Paper_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    Paper_SendData(Ystart & 0xFF);
    Paper_SendData((Ystart >> 8) & 0xFF);
    Paper_SendData(Yend & 0xFF);
    Paper_SendData((Yend >> 8) & 0xFF);
    Paper_CS_1;
}

/******************************************************************************
function :	Set cursor
parameter:
******************************************************************************/
void Paper_SetCursor(uint16_t Xstart, uint16_t Ystart)
{
    Paper_SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    Paper_SendData((Xstart >> 3) & 0xFF);
    Paper_CS_1;

    Paper_SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    Paper_SendData(Ystart & 0xFF);
    Paper_SendData((Ystart >> 8) & 0xFF);
    Paper_CS_1;

    Paper_WaitUntilIdle();
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
void Paper_TurnOnDisplay(void)
{
    Paper_SendCommand(DISPLAY_UPDATE_CONTROL_2);
    Paper_SendData(0xC7);
    Paper_CS_1;

    Paper_SendCommand(MASTER_ACTIVATION);
    Paper_CS_1;

    Paper_WaitUntilIdle();
}


void Paper_SoftwareReset(void){
	Paper_SendCommand(0x12);
	Paper_CS_1;

	Paper_WaitUntilIdle();
}
/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
uint8_t Paper_Init()
{
	Paper_CS_1;

    Paper_Reset();

    Paper_SoftwareReset();

    Paper_SendCommand(0x74);  //Set Analog Block Control
    Paper_SendData(0x54);
    Paper_CS_1;

    Paper_SendCommand(0x7E);  //Set Digital Block Control
    Paper_SendData(0x3B);
    Paper_CS_1;


    Paper_SendCommand(0x2B);  // Reduce glitch under ACVCOM
    Paper_SendData(0x04);
    Paper_SendData(0x63);
    Paper_CS_1;


    Paper_SendCommand(BOOSTER_SOFT_START_CONTROL);		  // Booster Soft start Control
    Paper_SendData(0x8B);
    Paper_SendData(0x9C);
    Paper_SendData(0x96);
    Paper_SendData(0x0F);
    Paper_CS_1;

    Paper_SendCommand(DRIVER_OUTPUT_CONTROL);  // Driver Output control
    Paper_SendData(0xD3);
    Paper_SendData(0x00);
    Paper_SendData(0x00);
    Paper_CS_1;

    Paper_SendCommand(DATA_ENTRY_MODE_SETTING);  // Data Entry mode setting
    Paper_SendData(0x03);
    Paper_SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION); //Set RAM X - address Start / End position
    Paper_SendData(0x00); // RAM x address start at 0
    Paper_SendData(0x0C); //RAM x address end at 0Ch(12+1)*8->104
    Paper_SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION); //Set Ram Y- address Start / End position
    Paper_SendData(0xD3); // RAM y address start at 0D3h;
    Paper_SendData(0x00);
    Paper_SendData(0x00); // RAM y address end at 00h;
    Paper_SendData(0x00);
    Paper_CS_1;


    Paper_SendCommand(BORDER_WAVEFORM_CONTROL); // Border Waveform Control
    Paper_SendData(0x01); // HIZ
    Paper_CS_1;

    Paper_SendCommand(DISPLAY_UPDATE_CONTROL_1);
    Paper_SendData(0x00);//Normal
    Paper_CS_1;

 //       unsigned char temp1,temp2;
    Paper_SendCommand(0x18);//Temperature Sensor Control
    Paper_SendData(0x80);  //Internal temperature sensor
    Paper_CS_1;

    Paper_WriteLUT();
/*
    Paper_WriteLUT();

    Paper_SendCommand(0x04);		// set VSH,VSL value
	//Paper_SendData(0x50);		// 	    2D13  18v
	//Paper_SendData(0x41);		// 	    2D13  15v
    Paper_SendData(0x2D);		// 	    2D13  11v
	// Paper_SendData(0xbf);		//	    2D13   7.3v
	//  Paper_SendData(0xb8);		//	    2D13   6.6v
	//Paper_SendData(0xb4);		//	    2D13   6.2v
	Paper_SendData(0xb2);		//	    2D13   6v
	// Paper_SendData(0xAE);		//	    2D13   5.6v
	// Paper_SendData(0xac);		//	    2D13   5.4v
	//Paper_SendData(0xa8);		//	    2D13   5v
	//Paper_SendData(0xaa);		//	    2D13   5.2v
	//Paper_SendData(0xA4);		//	    2D13   4.6v
	// Paper_SendData(0xA2);		//	    2D13   4.4v
	// Paper_SendData(0xa0);		//	    2D13   4.2v
	// Paper_SendData(0x9C);		//	    2D13   3.8v
	//  Paper_SendData(0x96);		//	    2D9   3.8v
	// Paper_SendData(0x9A);		//	    2D13   3.6v
	Paper_SendData(0x22);		//	    2D13  -11v
	// Paper_SendData(0x32);		//	    2D13  -15v
	// Paper_SendData(0x3E);		//	    2D13  -18v
	Paper_CS_1;

	Paper_SendCommand(0x2C);           // vcom
	//Paper_SendData(0x78);           //-3V
	//Paper_SendData(0x6f);           //-2.8V
	//Paper_SendData(0x6c);           //-2.7V
	//Paper_SendData(0x68);           //-2.6V
	//Paper_SendData(0x5F);           //-2.4V
	//Paper_SendData(0x50);           //-2V
	Paper_SendData(0x3C);           //-1.5V
	Paper_CS_1;


	Paper_SendCommand(0x3A);
	Paper_SendData(0x30);
	Paper_CS_1;

	Paper_SendCommand(0x3B);
	Paper_SendData(0x0A);
	Paper_CS_1;
*/

    Paper_SendCommand(DISPLAY_UPDATE_CONTROL_2);//Display UpdateControl 2
    Paper_SendData(0xB1);	//Load Temperature and waveform setting.
//     Paper_SendData(0x80);	//Load Temperature and waveform setting.
    Paper_CS_1;

    Paper_SendCommand(MASTER_ACTIVATION); //Master Activation
    Paper_CS_1;
    Paper_WaitUntilIdle();

    return 0;
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void Paper_Clear(void)
{
    uint16_t Width, Height;
    Width = (Paper_WIDTH % 8 == 0)? (Paper_WIDTH / 8 ): (Paper_WIDTH / 8 + 1);
    Height = Paper_HEIGHT;

    Paper_SetWindows(0, 0, Paper_WIDTH, Paper_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        Paper_SetCursor(0, j);

        Paper_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            Paper_SendData(0XFF);
        }
        Paper_CS_1;
    }
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void Paper_Display(uint8_t *buffer)
{
    uint16_t Width, Height;
    Width = (Paper_WIDTH % 8 == 0)? (Paper_WIDTH / 8 ): (Paper_WIDTH / 8 + 1);
    Height = Paper_HEIGHT;

    Paper_SetWindows(0, 0, Paper_WIDTH, Paper_HEIGHT);
    for (uint16_t j = 0; j < Height; j++) {
        Paper_SetCursor(0, j);

        Paper_SendCommand(WRITE_RAM);
        for (uint16_t i = 0; i < Width; i++) {
            Paper_SendData(buffer[i + (j * Width)]);
        }
        Paper_CS_1;
    }
    Paper_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void Paper_Sleep(void)
{
    Paper_SendCommand(DEEP_SLEEP_MODE);
    Paper_SendData(0x01);
    //Paper_WaitUntilIdle();
}
