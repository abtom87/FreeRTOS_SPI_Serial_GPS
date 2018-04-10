/*
 * spi_drv.h
 *
 *  Created on: Feb 28, 2018
 *      Author: abel
 */

#ifndef SPI_DRV_H_
#define SPI_DRV_H_

#include "stm32f4xx_spi.h"

#define SET_CS_LOW() GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define SET_CS_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_12)

void init_spi_module(void);
void init_spi_gpio(void);
void init_spi2_module(void);
void init_spi2_gpio(void);

void SPI_send_data(uint16_t byte);

#endif /* SPI_DRV_H_ */
