# FreeRTOS_SPI_Serial_GPS

HW used is STM32F411VE Eval board, 2 USART modules are used in this example

FreeRTOS port for CM4 is used to run three tasks in total. One task prints "Task1" and toggles on-board LEDs,
Task 2 handles an interrupt on a USART2 Rx line, i.e if a user enters a character via keyboard ,
Task3, handles a DMA interrupt, GPS module sends data constantly to USART6 module. In this task, the received data is parsed and 
transmitted to a LED display, this communication is done via SPI.
 

