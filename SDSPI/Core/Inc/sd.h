/*
 * sd.h
 *
 *  Created on: Jun 19, 2020
 *      Author: IV
 */

#ifndef INC_SD_H_
#define INC_SD_H_
//------------------------
#include "main.h"
#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <stdint.h>
//---------------------------
#define SS_SD_SELECT() HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define SS_SD_DESELECT() HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)
#define LD_ON HAL_GPIO_WritePin(LD5_GPIO_Port,LD5_Pin,GPIO_PIN_RESET)
#define LD_OFF HAL_GPIO_WritePin(LD5_GPIO_Port,LD5_Pin,GPIO_PIN_SET)
//---------------------------
/* Card type flags*/
#define CT_MMC 0x01 // MMC ver 3
#define CT_SD1 0x02 // SD ver1
#define CT_SD2 0x04 // SD ver2
#define CT_SDC (CT_SD1|CT_SD2) // SD
#define CT_BLOCK 0x08 // Block addressing
//------------------------
/* Definition for MMC/SDC command*/
#define CMD0 (0x40+0) // go_idle_state
#define CMD1 (0x40+1) // SEND_op_cond (MMC)
#define ACMD41 (0xC0+41) // send_op_cond (SDC)
#define CMD8 (0x40+8) // send_if_cond
#define CMD9 (0x40+9) // send_csd
#define CMD16 (0x40+16) // set_blocklen
#define CMD17 (0x40+17) // REad_single_block
#define CMD24 (0x40+24) // Write_block
#define CMD55 (0x40+55) // APP_CMD
#define CMD58 (0x40+58) // read_ocr
//-----------------------
typedef struct sd_info {
	volatile uint8_t type; // тип карты
} sd_info_ptr;

// Prototype-------------------
void SD_PowerOn(void);
uint8_t sd_ini(void);
void SPI_Release(void);

//--------------------------
#endif /* INC_SD_H_ */
