/*
 * stm32f4xx_spi_driver.h
 *
 *  Created on: Apr 2, 2025
 *      Author: 35596
 */

#ifndef INC_STM32F4XX_SPI_DRIVER_H_
#define INC_STM32F4XX_SPI_DRIVER_H_

#include "stm32f4xx.h"

/*
 * @SPI_DeviceMode
 */
#define SPI_MODE_MASTER 1
#define SPI_MODE_SLAVE  0

/*
 * @SPI_CommunicationMode
 */
#define SPI_FULL_DUPLEX 0
#define SPI_HALF_DUPLEX 1
#define SPI_SIMPLEX     2

/*
 * @SPI_DataFormat
 */
#define SPI_DFF_8BIT  0
#define SPI_DFF_16BIT 1

/*
 * @SPI_NSS_Configuration
 */
#define SPI_SSM_ENABLE  1
#define SPI_SSM_DISABLE 0

/*
 * @SPI_FrameFormat
 */
#define SPI_LSBFIRST  0
#define SPI_MSBFIRST  1

/*
 * @SPI_Baudrate
 */
#define SPI_BAUDRATE_FPCLK_2  0  //PCLK = 8 MHz on nucleo board, divide by 2, gives 4 Mhz
#define SPI_BAUDRATE_FPCLK_4  1  //divide by 4, gives 2 Mhz
#define SPI_BAUDRATE_FPCLK_8  2
#define SPI_BAUDRATE_FPCLK_16  3
#define SPI_BAUDRATE_FPCLK_32  4
#define SPI_BAUDRATE_FPCLK_64  5
#define SPI_BAUDRATE_FPCLK_128  6
#define SPI_BAUDRATE_FPCLK_256  7

/*
 * @SPI_CPOL_Configuration
 */
#define SPI_CPOL_LOW  0
#define SPI_CPOL_HIGH  1

/*
 * @SPI_CPHA_Configuration
 */
#define SPI_CPHA_LOW  0
#define SPI_CPHA_HIGH  1

/*
 * @SPI_CR1_SSI
 */
#define SPI_SSI_EN  1
#define SPI_SSI_DI  0


/*
 * @SPI_CR1_Configuration
 */
#define SPI_CR1_CPHA 0
#define SPI_CR1_CPOL 1
#define SPI_CR1_MSTR 2
#define SPI_CR1_BR   3
#define SPI_CR1_SPE  6
#define SPI_CR1_LSBFIRST 7
#define SPI_CR1_SSI 8
#define SPI_CR1_SSM 9
#define SPI_CR1_RXONLY 10
#define SPI_CR1_DFF 11
#define SPI_CR1_BIDIOE 14
#define SPI_CR1_BIDIMODE 15

/*
 * @SPI_CR2_Configuration
 */
#define SPI_CR2_SSOE 2
#define SPI_CR2_FRF  4
#define SPI_CR2_ERRIE 5
#define SPI_CR2_RXNEIE   6
#define SPI_CR2_TXEIE  7

/*
 * @SPI_SR_Configuration
 */
#define SPI_SR_RXNE 0
#define SPI_SR_TXE  1
#define SPI_SR_CHSIDE 2
#define SPI_SR_UDR   3
#define SPI_SR_MODF  5
#define SPI_SR_OVR   6
#define SPI_SR_BSY   7
#define SPI_SR_FRE   8

/*
 * @TxRxState
 */
#define SPI_READY 0
#define SPI_BUSY_TX 1
#define SPI_BUSY_RX 2

typedef struct
{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
	uint8_t SPI_SSI;
}SPI_Confg_t;

typedef struct
{
	SPI_RegDef_t *pSPIx;
	SPI_Confg_t SPIConfig;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxRxState;
}SPI_Handle_t;

void SPI_PeriClkCtrl(SPI_RegDef_t *pvSPIx,uint8_t EnDis);
void SPI_IgnoreNSSPin(SPI_RegDef_t *pvSPIx,uint8_t EnDis);
void SPI_Init(SPI_Handle_t *pvSPI_handle);
void SPI_DeInit(SPI_RegDef_t *pvSPIx);
void SPI_MasterSendData(SPI_RegDef_t *pvSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_MasterReceiveData (SPI_RegDef_t *pvSPIx, uint8_t *pRxBuffer, uint32_t Len);
void SPI_Close(SPI_RegDef_t *pvSPIx);
void SPI_IRQPriorityConfig(uint8_t IRQPriority);
void SPI_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnDis);
void SPI_MasterReceiveDataIT(SPI_Handle_t *pvSPIHandle, uint8_t *pRxBuffer, uint32_t Len);
void SPI_MasterSendDataIT(SPI_Handle_t *pvSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
void SPI_RXNEIE_Close(SPI_Handle_t * pvSPIHandle);
void SPI_TXEIE_Close(SPI_Handle_t * pvSPIHandle);
void SPI_EV_Handling(SPI_Handle_t * pvHandle);
#endif /* INC_STM32F4XX_SPI_DRIVER_H_ */
