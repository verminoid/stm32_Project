/*
 * sd.c
 *
 *  Created on: Jun 19, 2020
 *      Author: IV
 */

#include "sd.h"
//----------------------
extern volatile uint16_t Timer1;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;
sd_info_ptr sdinfo;
char str1[60]={0};
//-------------------
static void Error (void)
{
	LD_ON;
}
//--------------------
uint8_t SPIx_WriteRead(uint8_t Byte)
{
	uint8_t receivedbyte =0;
	if (HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)&Byte, (uint8_t*)&receivedbyte, 1, 1000)!=HAL_OK)
	{
		Error();
	}
	return receivedbyte;
}
//-------------------
void SPI_SendByte(uint8_t bt)
{
	SPIx_WriteRead(bt);
}
//------------------
uint8_t SPI_ReceiveByte(void)
{
	uint8_t bt=SPIx_WriteRead(0xFF);
	return bt;
}
//------------------
void SPI_Release(void)
{
	SPIx_WriteRead(0xFF);
}
//------------------
static uint8_t SD_cmd (uint8_t cmd, uint32_t arg)
{
	uint8_t n, res;
	if (cmd & 0x80)
	{
		cmd &= 0x7F;
		res = SD_cmd(CMD55,0);
		if (res>1) return  res;
	}
	//select card
	SS_SD_DESELECT();
	SPI_ReceiveByte();
	SS_SD_SELECT();
	SPI_ReceiveByte();
	SPI_SendByte(cmd); // start + command index
	SPI_SendByte((uint8_t)(arg >> 24)); // argument 31..24
	SPI_SendByte((uint8_t)(arg >> 16)); // argument 23..16
	SPI_SendByte((uint8_t)(arg >> 8)); // argument 15..8
	SPI_SendByte((uint8_t)arg); // argument 7..0
	n= 0x01; // Dummy CRC +Stop
	if (cmd == CMD0) {n = 0x95;}
	if (cmd == CMD8) {n = 0x87;}
	SPI_SendByte(n);

	n=10;
	do {
		res = SPI_ReceiveByte();
	} while ((res&0x80)&& --n);
	return res;
}
//-------------------
void SD_PowerOn(void)
{
	Timer1 =0;
	while(Timer1<2);
}
//------------------------
uint8_t sd_ini(void)
{
	uint8_t i, cmd;
	uint8_t ocr[4];
	int16_t tmr;
	uint32_t temp;
	LD_OFF;
	sdinfo.type =0;
	temp = hspi2.Init.BaudRatePrescaler;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128; // 156.25 kbps
	HAL_SPI_Init(&hspi2);
	SS_SD_DESELECT();
	for(i=0;i<10;i++) // 80 импульсов
		SPI_Release();
	hspi2.Init.BaudRatePrescaler = temp;
	HAL_SPI_Init(&hspi2);
	SS_SD_SELECT();
	if (SD_cmd(CMD0, 0)==1)// enteridle state
	{
		SPI_Release();
		if (SD_cmd(CMD8, 0x1AA) == 1) //sdv2
		{
			for (i=0;i<4;i++) ocr[i] = SPI_ReceiveByte();
			HAL_UART_Transmit(&huart1, (uint8_t*)str1, sprintf(str1, "OCR: 0x%02X 0x%02X 0x%02X 0x%02X \r\n",ocr[0],ocr[1],ocr[2],ocr[3]), 1000);
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) // work with 2.7 - 3.6v
			{
				for (tmr = 12000; tmr && SD_cmd(ACMD41, 1UL << 30); tmr--); // wait idle state
				if (tmr && SD_cmd(CMD58, 0)==0) {
					for (i=0; i<4; i++) ocr[i] = SPI_ReceiveByte();
					HAL_UART_Transmit(&huart1, (uint8_t*)str1, sprintf(str1, "OCR: 0x%02X 0x%02X 0x%02X 0x%02X \r\n",ocr[0],ocr[1],ocr[2],ocr[3]), 1000);
					sdinfo.type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // sdv2
				}
			}
		}
		else //sdv1 or MMCv3
		{
			if (SD_cmd(ACMD41, 0) <= 1)
			{
				sdinfo.type = CT_SD1; cmd = ACMD41; //SDv1
			}
			else
			{
				sdinfo.type = CT_MMC; cmd = CMD1; //MMCv3
			}
			for (tmr = 25000; tmr && SD_cmd(cmd, 0); tmr--); // wait idle state
			if (!tmr || SD_cmd(CMD16, 512) != 0) // set r/w block length 512
			sdinfo.type = 0;
		}
	}
	else
	{
		return 1;
	}
	HAL_UART_Transmit(&huart1, (uint8_t*)str1, sprintf(str1, "Type SD: 0x%02X \r\n", sdinfo.type), 1000);

	return 0;
}




