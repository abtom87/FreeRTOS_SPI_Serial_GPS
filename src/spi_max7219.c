/*
 * spi_max7219.c
 *
 *  Created on: Mar 6, 2018
 *      Author: abel
 */
#include "spi_max7219.h"

#define SUMMER_TIME

void init_MAX7219_powerup()
{
	write_to_MAX7219(MAX7219_ADDR_DECODE_MODE, 0xFF);
	write_to_MAX7219(MAX7219_ADDR_INTENSITY, 0x0A);
	write_to_MAX7219(MAX7219_ADDR_SCAN_LIM, 0x07);
	write_to_MAX7219(MAX7219_ADDR_SHUTDOWN, 0x01);
	write_to_MAX7219(MAX7219_ADDR_DISP_TEST, 0x00);

	write_to_MAX7219(MAX7219_ADDR_DIG_0, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_1, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_2, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_3, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_4, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_5, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_6, 0x00);
	write_to_MAX7219(MAX7219_ADDR_DIG_7, 0x00);
	clearDisplay_MAX7219();
}

void write_to_MAX7219(uint8_t addr, uint8_t data)
{

	uint16_t Addr, Data, Data_to_send;
	Addr = (uint16_t) addr;
	Data = (uint16_t) data;

	Data_to_send = ( ( (Addr << 8) & 0xFF00) | Data );

	SET_CS_LOW();
	SPI_send_data(Data_to_send);
	SET_CS_HIGH();
}

void clearDisplay_MAX7219()
{
	uint8_t i = DIGITS_TO_DISPLAY;
	while (i)
	{
		write_to_MAX7219(i, 0x0F);
		i--;
	}

}
void Display_longNum_MAX7219(volatile long number)
{
	uint8_t i = 1;

	while (number)
	{
		if (i == 3 || i == 5)
			write_to_MAX7219(i, 0x80 + (uint8_t) (number % 10));
		else
			write_to_MAX7219(i, (uint8_t) (number % 10));
		number /= 10;
		i++;
	}

}
void Display_Time_MAX7219(char *str_to_parse, char *str_to_extract,
		const uint8_t offset)
{
	unsigned long time_to_disp = 0;
	int k = 0;
	unsigned char *substr;
	unsigned char Time_buffer[10] = { 0 };
	char gps_buffer[1024] = { 0 };

	for (int i = 0; i < strlen(str_to_parse); i++)
		gps_buffer[i] = str_to_parse[i];

	substr = strstr(gps_buffer, str_to_extract);
	k = offset;
	while (k < 13)
	{
		Time_buffer[k - offset] = (unsigned char) (substr[k]);
		k++;
	}


#ifdef SUMMER_TIME /* write summer/ winter time   */
	if ( (Time_buffer[1] == 0x38) )
	{
		Time_buffer[1] = 0x30; /* If HH is 18 , change 1 to 2 and 8 to 0 */
		Time_buffer[0] += 1;
	}
	else
		if( (Time_buffer[1] == 0x39)  )
		{
			Time_buffer[1] = 0x31; /* If HH is 19 , change 1 to 2 and 9 to 1 */
			Time_buffer[0] +=1;
		}
		else
		Time_buffer[1] += 2;
#else

	if ( (Time_buffer[1] == 0x39) &&  (Time_buffer[0] == 0x31) )
	{
		Time_buffer[1] = 0x30;  /* If HH is 19 , change 1 to 2 and 9 to 0 */
		Time_buffer[0] += 1;
	}
	else
		Time_buffer[1] += 1;
#endif

	time_to_disp = strtol(Time_buffer, (char **) NULL, 10);

	if (time_to_disp >= 244905)
		clearDisplay_MAX7219();

	Display_longNum_MAX7219(time_to_disp);
}

