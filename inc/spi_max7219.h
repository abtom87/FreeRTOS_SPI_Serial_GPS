/*
 * max7219.h
 *
 *  Created on: Mar 6, 2018
 *      Author: abel
 */

#ifndef SPI_MAX7219_H_
#define SPI_MAX7219_H_

#include <string.h>
#include <stdlib.h>
#include "spi_drv.h"
#include "stm32f4xx_gpio.h"

#define MAX7219_ADDR_DECODE_MODE 0x09
#define MAX7219_ADDR_INTENSITY   0x0A
#define MAX7219_ADDR_SCAN_LIM    0x0B
#define MAX7219_ADDR_SHUTDOWN    0x0C
#define MAX7219_ADDR_DISP_TEST   0x0F

#define MAX7219_ADDR_DIG_0       0x01
#define MAX7219_ADDR_DIG_1	     0x02
#define MAX7219_ADDR_DIG_2       0x03
#define MAX7219_ADDR_DIG_3       0x04
#define MAX7219_ADDR_DIG_4       0x05
#define MAX7219_ADDR_DIG_5       0x06
#define MAX7219_ADDR_DIG_6       0x07
#define MAX7219_ADDR_DIG_7       0x08

#define DIGITS_TO_DISPLAY         8

void init_MAX7219_powerup(void);
void write_to_MAX7219(uint8_t addr, uint8_t data);
void clearDisplay_MAX7219(void);
void Display_longNum_MAX7219(volatile long number);

void Display_Time_MAX7219(char *str_to_parse, char *str_to_extract,
		const uint8_t offset);

#endif /* SPI_MAX7219_H_ */
