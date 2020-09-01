
/*----------------------------
        nRF24L01 Module
        Source
----------------------------*/

#include "nRF24L01.h"


uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE2};
uint8_t RX_ADDRESS0[TX_ADR_WIDTH] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE2};
uint8_t RX_ADDRESS1[TX_ADR_WIDTH] = {0xC1, 0xC2, 0xC2, 0xC2, 0xC2};
//uint8_t RX_ADDRESS1[TX_ADR_WIDTH] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC1};
volatile uint8_t RX_BUF[TX_PLOAD_WIDTH] = {0};
volatile uint8_t rx_flag = 0;
//----------------------------
extern SPI_HandleTypeDef hspi1;
//-----------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros)
{
  micros *= (SystemCoreClock / 1000000) / 9;
  /* Wait till done */
  while (micros--) ;
}

//--------------------------------------------------
//Function
//------------------------------------------------
uint8_t NRF24_ReadReg(uint8_t addr)
{
  uint8_t dt=0, cmd;
  CS_ON;
  HAL_SPI_TransmitReceive(&hspi1,&addr,&dt,1,1000);
  if (addr!=STATUS) // ���� ����� ����� ������ �������� ������ �� ���������� ��� ���������
  {
    cmd=0xFF;
    HAL_SPI_TransmitReceive(&hspi1,&cmd,&dt,1,1000);
  }
  CS_OFF;
  return dt;
}
//-------------------
void NRF24_WriteReg(uint8_t addr, uint8_t dt)
{
  addr |= W_REGISTER; // �������� ��� ������ � �����
  CS_ON;
  HAL_SPI_Transmit(&hspi1,&addr, 1, 1000); // send address
  HAL_SPI_Transmit(&hspi1,&dt,1,1000);// send  data
  CS_OFF;
}
//--------------------
void NRF24_ToggleFeatures(void)
{
  uint8_t dt[1]= {ACTIVATE};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  dt[0] = 0x73;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  CS_OFF;
}
//--------------------
void NRF24_Read_Buf(uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{
  CS_ON;
  HAL_SPI_Transmit(&hspi1, &addr, 1, 1000); // address
  HAL_SPI_Receive(&hspi1, pBuf, bytes, 1000); // data to buffer
  CS_OFF;
}
//--------------------
void NRF24_Write_Buf(uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{
  addr |= W_REGISTER; // ��� ������ � �����
  CS_ON;
  HAL_SPI_Transmit(&hspi1, &addr, 1, 1000); // address + write
  DelayMicro(1);
  HAL_SPI_Transmit(&hspi1, pBuf, bytes,1000); // data 
  CS_OFF;
}
//----------------------
void NRF24_FlushRX(void)
{
  uint8_t dt[1] = {FLUSH_RX};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  CS_OFF;
}
//--------------------
void NRF24_FlushTX(void)
{
  uint8_t dt[1] = {FLUSH_TX};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  CS_OFF;
}
//---------------------- ��������� ������
void NRF24_RX_Mode(void)
{
  uint8_t regval=0x00;
  regval = NRF24_ReadReg(CONFIG);
  regval |= (1<<PWR_UP)|(1<<PRIM_RX);
  NRF24_WriteReg(CONFIG,regval);
  CE_SET;
  DelayMicro(150);
  //Flush buffers
  NRF24_FlushRX();
  NRF24_FlushTX();
}
//--------------------
void NRF24_TX_Mode(void)
{
  NRF24_Write_Buf(TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);
  CE_RESET;
  //Flush
  NRF24_FlushRX();
  NRF24_FlushTX();
}
//--------------------
//   Transmit
//--------------------
void NRF24_Transmit(uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{
  CE_RESET;
  CS_ON;
  HAL_SPI_Transmit(&hspi1, &addr, 1, 1000); //address
  DelayMicro(1);
  HAL_SPI_Transmit(&hspi1,pBuf,bytes,1000); //data
  CS_OFF;
  CE_SET;
}
//--------------------
uint8_t NRF24_Send(uint8_t *pBuf)
{
  uint8_t status=0x00, regval=0x00;
  NRF24_TX_Mode();
  regval = NRF24_ReadReg(CONFIG);
  // wake up
  regval |= (1<<PWR_UP);
  regval &= ~(1<<PRIM_RX);
  NRF24_WriteReg(CONFIG,regval);
  DelayMicro(150); 
  //Send
  NRF24_Transmit(WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);
  CE_SET;
  DelayMicro(15); // 10us High pulse
  CE_RESET;
  while(!(NRF24_ReadReg(STATUS)&0x70)) {} //!!!!!!
  status = NRF24_ReadReg(STATUS);
  if(status&TX_DS) // tx_ds == 0x20
  {
  //  LED_TGL;
    NRF24_WriteReg(STATUS, 0x20);
  }
  else 
    if (status&MAX_RT)
    {
      NRF24_WriteReg(STATUS, 0x10);
      NRF24_FlushTX();
    }
  regval = NRF24_ReadReg(OBSERVE_TX);
    // Reciever
  NRF24_RX_Mode();
  return regval;
}
//--------------------
/*
void NRF24_Receive(void)
{
  uint8_t status=0x01;
  uint16_t dt=0;
  while(!(NRF24_ReadReg(STATUS)&0x70)){} // !!! IRQ PIN
  status = NRF24_ReadReg(STATUS);
  DelayMicro(10);
  status = NRF24_ReadReg(STATUS);
  if (status & 0x40)
  {
    NRF24_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);
    dt = *(int16_t*)RX_BUF;// 
    // 
    NRF24_WriteReg(STATUS, 0x40);
  }
}*/
//--------------------
void NRF24_init(void)
{
  CE_RESET;
  HAL_Delay(5);
  NRF24_WriteReg(CONFIG, 0x0a); //Pwr_up, CRC 1bit, Prim_rx (transmitter)
  HAL_Delay(5); // time for start
  NRF24_WriteReg(EN_AA, 0x03); //Enable pipe 0+1 !!!!
  NRF24_WriteReg(EN_RXADDR, 0x03); // pipe 0+1
  NRF24_WriteReg(SETUP_AW, 0x11); // ����� ������, �������� �� ����!!!!
  NRF24_WriteReg(SETUP_RETR, 0x5F); // 1500us, 15 retrans
  NRF24_ToggleFeatures();
  NRF24_WriteReg(FEATURE,0);
  NRF24_WriteReg(DYNPD,0);
  NRF24_WriteReg(STATUS,0x70); // reset flags IRQ
  NRF24_WriteReg(RF_CH, 4); // CHANNEL !!!
  NRF24_WriteReg(RF_SETUP,0x06); //Power 0dBm, 1Mbps!!!
  NRF24_Write_Buf(TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
  NRF24_Write_Buf(RX_ADDR_P0, RX_ADDRESS0, TX_ADR_WIDTH);
  NRF24_Write_Buf(RX_ADDR_P1, RX_ADDRESS1, TX_ADR_WIDTH);  
  NRF24_WriteReg(RX_PW_P0, TX_PLOAD_WIDTH); //payload on pipe 0
  NRF24_WriteReg(RX_PW_P1, TX_PLOAD_WIDTH); //payload on pipe 1
  NRF24_RX_Mode();
 // LED_ON;
}
//-------------------------------------------
//IRQ
//------------------------
/*
void IRQ_Callback(void)
{
  uint8_t status=0x01;
  uint16_t dt=0;
  uint8_t pipe;
  LED_TGL;
  status = NRF24_ReadReg(STATUS);
  if(status & RX_DR)
  {
    NRF24_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);
    NRF24_WriteReg(STATUS, RX_DR);
    rx_flag = 1;
  }
}*/
