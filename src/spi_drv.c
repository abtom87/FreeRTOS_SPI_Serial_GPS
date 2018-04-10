/*
 * spi_drv.c
 *
 *  Created on: Feb 28, 2018
 *      Author: abel
 */

#include "spi_drv.h"

void init_spi_module(void)
{

	SPI_InitTypeDef SPI_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	//SPI_StructInit(&SPI_InitStruct);
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	//SPI_InitStruct.SPI_CRCPolynomial = 0x00;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;

	SPI_Init(SPI1, &SPI_InitStruct);
	SPI_Cmd(SPI1, ENABLE);
}

void init_spi2_module(void)
{

	SPI_InitTypeDef SPI2_InitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//SPI_StructInit(&SPI2_InitStruct);
	SPI2_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI2_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI2_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	//SPI_InitStruct.SPI_CRCPolynomial = 0x00;
	SPI2_InitStruct.SPI_DataSize = SPI_DataSize_16b;
	SPI2_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI2_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI2_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI2_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;

	SPI_Init(SPI2, &SPI2_InitStruct);
	SPI_Cmd(SPI2, ENABLE);
}
void init_spi2_gpio(void)
{
	GPIO_InitTypeDef SPI2_GPIOStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/*
	 * PB13 - SCK
	 * PB14 - MISO
	 * PB15 - MOSI
	 * */
	SPI2_GPIOStruct.GPIO_Mode = GPIO_Mode_AF;
	SPI2_GPIOStruct.GPIO_OType = GPIO_OType_PP;
	SPI2_GPIOStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	SPI2_GPIOStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	SPI2_GPIOStruct.GPIO_Speed = GPIO_Medium_Speed;

	GPIO_Init(GPIOB, &SPI2_GPIOStruct);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

	/*NSS Pin Config PB12 */
	GPIO_InitTypeDef SPI_SSPIN_Struct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	SPI_SSPIN_Struct.GPIO_Mode = GPIO_Mode_OUT;
	SPI_SSPIN_Struct.GPIO_OType = GPIO_OType_PP;
	SPI_SSPIN_Struct.GPIO_Pin = GPIO_Pin_12;
	SPI_SSPIN_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	SPI_SSPIN_Struct.GPIO_Speed = GPIO_High_Speed;

	GPIO_Init(GPIOB, &SPI_SSPIN_Struct);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);

}

void init_spi_gpio(void)
{
	GPIO_InitTypeDef SPI_GPIOStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/*
	 * PA5 - SCK
	 * PA6 - MISO
	 * PA7 - MOSI
	 * */
	SPI_GPIOStruct.GPIO_Mode = GPIO_Mode_AF;
	SPI_GPIOStruct.GPIO_OType = GPIO_OType_PP;
	SPI_GPIOStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	SPI_GPIOStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	SPI_GPIOStruct.GPIO_Speed = GPIO_High_Speed;

	GPIO_Init(GPIOA, &SPI_GPIOStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	/*NSS Pin Config */
	GPIO_InitTypeDef SPI_SSPIN_Struct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	SPI_SSPIN_Struct.GPIO_Mode = GPIO_Mode_OUT;
	SPI_SSPIN_Struct.GPIO_OType = GPIO_OType_PP;
	SPI_SSPIN_Struct.GPIO_Pin = GPIO_Pin_4;
	SPI_SSPIN_Struct.GPIO_PuPd = GPIO_PuPd_UP;
	SPI_SSPIN_Struct.GPIO_Speed = GPIO_High_Speed;

	GPIO_Init(GPIOA, &SPI_SSPIN_Struct);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

void SPI_send_data(uint16_t byte)
{
	SPI_I2S_SendData(SPI2, (uint16_t) byte);
	while (((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)) != SET))
		;
	while (((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY)) == SET))
		;
}

