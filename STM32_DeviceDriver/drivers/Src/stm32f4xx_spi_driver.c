/*
 * stm32f4xx_spi_driver.c
 *
 *  Created on: Apr 2, 2025
 *      Author: 35596
 */

#include "stm32f4xx_spi_driver.h"


void SPI_Init(SPI_Handle_t *pvSPI_handle)
{
	//first let configure the SPI_CR1 register

	uint32_t tempreg =0;

	//Enable SPI peripheral clock
	SPI_PeriClkCtrl(pvSPI_handle->pSPIx,ENABLE);

	//set device mode, master/slave
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR );

	//set communication modes
	if(pvSPI_handle->SPIConfig.SPI_BusConfig == SPI_FULL_DUPLEX)
	{
		//BIDI mode is cleared
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
	}
	else if(pvSPI_handle->SPIConfig.SPI_BusConfig == SPI_HALF_DUPLEX)
	{
		//BIDI mode is Set
		tempreg |= (1 << SPI_CR1_BIDIMODE);
	}
	else//simplex
	{
		//BIDI mode is cleared
		//RXONLY is set
		//In simplex Readonly is provided
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
		tempreg |= (1 << SPI_CR1_RXONLY);
	}

	//set clock speed
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR );

	//set data format
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_DFF << SPI_CR1_DFF );

	//set clock polarity
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL );

	//set clock phase
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA );

	//set software slave select
	tempreg |= ( pvSPI_handle->SPIConfig.SPI_SSM << SPI_CR1_SSM );

	//SSI enable/disable when SSM is enabled
	if( pvSPI_handle->SPIConfig.SPI_SSM == SET )
	{
		tempreg |= ( pvSPI_handle->SPIConfig.SPI_SSI << SPI_CR1_SSI );
	}
	else
	{
		//when Slave select pin is managed by hardware, it managed automatically
		//it becomes low when SPE =1 and becomes high when SPE=0;
		pvSPI_handle->pSPIx->SPI_CR2 |= ( 1 << SPI_CR2_SSOE ) ;
	}

	//enable SPI peripheral
	tempreg |= ( 1<< SPI_CR1_SPE);

	pvSPI_handle->pSPIx->SPI_CR1 = tempreg;
}
void SPI_MasterSendData(SPI_RegDef_t *pvSPIx, uint8_t *pTxBuffer, uint32_t Len)
{

	while( Len )
	{
		//wait for TxE = 1
		while(!(pvSPIx->SPI_SR & ( 1 << SPI_SR_TXE )));

		//check the DFF in CR1 to send 8 or 16 bit data
		if( pvSPIx->SPI_CR1 & ( 1 << SPI_CR1_DFF) )
		{
			//DFF is 16 bit
			pvSPIx->SPI_DR = *((uint16_t*)pTxBuffer);
			Len--;
			Len--;
			(uint16_t*)pTxBuffer++;
		}
		else
		{
			//DFF is 8 bit
			pvSPIx->SPI_DR = *pTxBuffer;
			pTxBuffer++;
			Len--;
		}
	}
	//confirm last data byte sent by checking TxE = 1, BSY = 0
	//while(!(pvSPIx->SPI_SR & ( 1 << SPI_SR_TXE )));
	//while((pvSPIx->SPI_SR & ( 1 << SPI_SR_BSY )));
}

void SPI_MasterReceiveData(SPI_RegDef_t *pvSPIx, uint8_t *pRxBuffer, uint32_t Len)
{

	while( Len )
	{
		//wait for TxE = 1
		while(!(pvSPIx->SPI_SR & ( 1 << SPI_SR_RXNE )));

		//read data from Data Register
		//check the DFF in CR1 to send 8 or 16 bit data
		if( pvSPIx->SPI_CR1 & ( 1 << SPI_CR1_DFF) )
		{
			//DFF is 16 bit
			*((uint16_t*)pRxBuffer) = pvSPIx->SPI_DR;
			Len--;
			Len--;
			(uint16_t*)pRxBuffer++;
		}
		else
		{
			//DFF is 8 bit
			*pRxBuffer = pvSPIx->SPI_DR;
			pRxBuffer++;
			Len--;
		}
	}
	//confirm last data byte sent by checking TxE = 1, BSY = 0
	while(!(pvSPIx->SPI_SR & ( 1 << SPI_SR_TXE )));
	while((pvSPIx->SPI_SR & ( 1 << SPI_SR_BSY )));
}

void SPI_MasterReceiveDataIT(SPI_Handle_t *pvSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
	pvSPIHandle->TxRxState = SPI_BUSY_RX;
	pvSPIHandle->pRxBuffer = pRxBuffer;
	pvSPIHandle->RxLen = Len;
	//enable TXEIE and ERRIE interrupt
	pvSPIHandle->pSPIx->SPI_CR2 |= ( 1 << SPI_CR2_RXNEIE );
	pvSPIHandle->pSPIx->SPI_CR2 |= ( 1 << SPI_CR2_ERRIE );
}
void SPI_MasterSendDataIT(SPI_Handle_t *pvSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
	pvSPIHandle->TxRxState = SPI_BUSY_TX;
	pvSPIHandle->pTxBuffer = pTxBuffer;
	pvSPIHandle->TxLen = Len;
	//enable TXEIE and ERRIE interrupt
	pvSPIHandle->pSPIx->SPI_CR2 |= ( 1 << SPI_CR2_TXEIE );
	pvSPIHandle->pSPIx->SPI_CR2 |= ( 1 << SPI_CR2_ERRIE );

}
void SPI_EV_Handling(SPI_Handle_t * pvHandle)
{
	uint8_t temp1,temp2,temp3;

	//check TXE event
	temp1 = (pvHandle->pSPIx->SPI_SR & ( 1 << SPI_SR_TXE ));
	temp2 = (pvHandle->pSPIx->SPI_CR2 & ( 1 << SPI_CR2_TXEIE ));

	temp3 = (pvHandle->pSPIx->SPI_CR1 & ( 1 << SPI_CR1_DFF ));
	if( temp1 && temp2 )
	{
		if(temp3)
		{
			pvHandle->pSPIx->SPI_DR = *((uint16_t*)pvHandle->pTxBuffer) ;
			(uint16_t*)pvHandle->pTxBuffer++;
			pvHandle->TxLen--;
			pvHandle->TxLen--;
		}
		else
		{
			pvHandle->pSPIx->SPI_DR = *pvHandle->pTxBuffer;
			pvHandle->pTxBuffer++;
			pvHandle->TxLen--;
		}
	}

	//check RXNE event
	temp1 = (pvHandle->pSPIx->SPI_SR & ( 1 << SPI_SR_RXNE ));
	temp2 = (pvHandle->pSPIx->SPI_CR2 & ( 1 << SPI_CR2_RXNEIE ));

	temp3 = (pvHandle->pSPIx->SPI_CR1 & ( 1 << SPI_CR1_DFF ));
	if( temp1 && temp2 )
	{
		if(temp3)
		{
			*((uint16_t*)pvHandle->pRxBuffer) = pvHandle->pSPIx->SPI_DR ;
			(uint16_t*)pvHandle->pRxBuffer++;
			pvHandle->RxLen--;
			pvHandle->RxLen--;
		}
		else
		{
			*pvHandle->pRxBuffer = pvHandle->pSPIx->SPI_DR ;
			pvHandle->pRxBuffer++;
			pvHandle->RxLen--;
		}
	}

	// check ERRIE
	temp1 = (pvHandle->pSPIx->SPI_SR & ( 1 << SPI_SR_OVR ));
	temp2 = (pvHandle->pSPIx->SPI_CR2 & ( 1 << SPI_CR2_ERRIE ));
	if( temp1 && temp2 )
	{
		//let the update the application
		while(1);
	}

	//close the TXE SPI
	if( pvHandle->TxLen == 0 )
	{
		SPI_TXEIE_Close(pvHandle);
	}

	//close the RXNE SPI
	if( pvHandle->RxLen == 0 )
	{
		SPI_RXNEIE_Close(pvHandle);
	}
}
void SPI_Close(SPI_RegDef_t * pvSPIx)
{
	pvSPIx->SPI_CR1 &= ~( 1 << SPI_CR1_SPE );
}
void SPI_TXEIE_Close(SPI_Handle_t * pvSPIHandle)
{
	pvSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_TXEIE );
	pvSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_ERRIE );
	pvSPIHandle->pSPIx->SPI_CR1 &= ~( 1 << SPI_CR1_SPE );
	pvSPIHandle->TxLen = 0;
	pvSPIHandle->pTxBuffer = '\0';
	pvSPIHandle->TxRxState = SPI_READY;
}
void SPI_RXNEIE_Close(SPI_Handle_t * pvSPIHandle)
{
	pvSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_RXNEIE );
	pvSPIHandle->pSPIx->SPI_CR2 &= ~( 1 << SPI_CR2_ERRIE );
	pvSPIHandle->pSPIx->SPI_CR1 &= ~( 1 << SPI_CR1_SPE );
	pvSPIHandle->RxLen = 0;
	pvSPIHandle->pRxBuffer = '\0';
	pvSPIHandle->TxRxState = SPI_READY;
}
void SPI_PeriClkCtrl(SPI_RegDef_t * pvSPIx,uint8_t EnDis)
{
	if(EnDis == ENABLE)
	{
		if( pvSPIx == SPI1 )
		{
			SPI1_PCLK_EN();
		}
		else if( pvSPIx == SPI2 )
		{
			SPI2_PCLK_EN();
		}
		else if( pvSPIx == SPI3 )
		{
			SPI3_PCLK_EN();
		}
	}
	else
	{
		if( pvSPIx == SPI1 )
		{
			SPI1_PCLK_DI();
		}
		else if( pvSPIx == SPI2 )
		{
			SPI2_PCLK_DI();
		}
		else if( pvSPIx == SPI3 )
		{
			SPI3_PCLK_DI();
		}
	}
}

void SPI_IgnoreNSSPin(SPI_RegDef_t *pvSPIx,uint8_t EnDis)
{
	if( EnDis == SET )
	{
		pvSPIx->SPI_CR1 |= ( 1 << SPI_CR1_SSI );
	}
	else
	{
		pvSPIx->SPI_CR1 &= ~( 1 << SPI_CR1_SSI );
	}
}

void SPI_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnDis)
{
	 if(EnDis)
	 {
		 if(IRQNumber < 32)
		 {
			 *NVIC_ISER0 |= (1<<IRQNumber);
		 }
		 else if(IRQNumber >= 32 && IRQNumber < 64)
		 {
			 *NVIC_ISER1 |= (1<<(IRQNumber % 32));
		 }
		 else if(IRQNumber >=64 && IRQNumber < 96)
		 {
			 *NVIC_ISER2 |= (1<<(IRQNumber % 64));
		 }
		 else if(IRQNumber >=96 && IRQNumber < 128)
		 {
			 *NVIC_ISER3 |= (1<<(IRQNumber % 96));
		 }
	 }
	 else
	 {
		 if(IRQNumber < 32)
		 {
			 *NVIC_ICER0 |= (1<<IRQNumber);
		 }
		 else if(IRQNumber >= 32 && IRQNumber < 64)
		 {
			 *NVIC_ICER1 |= (1<<(IRQNumber % 32));
		 }
		 else if(IRQNumber >=64 && IRQNumber < 96)
		 {
			 *NVIC_ICER2 |= (1<<(IRQNumber % 64));
		 }
		 else if(IRQNumber >=96 && IRQNumber < 128)
		 {
			 *NVIC_ICER3 |= (1<<(IRQNumber % 96));
		 }
	 }
}
void SPI_IRQPriorityConfig(uint8_t IRQPriority)
{
	 uint8_t pri_reg_number=0;
	 uint8_t pri_reg_section =0;
	 uint8_t shift_amount = 0;
	 pri_reg_number = IRQPriority / 4;
	 pri_reg_section = IRQPriority % 4;
	 shift_amount = (8 * pri_reg_section)+ (8-CM4_INT_PRI_BIT_IMPLEMENTATION);
	 *(NVIC_PRIO_BASEADDRS + (4 *pri_reg_number)) |= (IRQPriority << shift_amount );
}
