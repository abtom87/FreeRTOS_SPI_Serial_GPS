/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "math.h"
#include "stdio.h"
#include "usart2_comm.h"
#include "gpio_led.h"
#include "usart6_dma_comm.h"
#include "dma2_driver.h"
#include "spi_drv.h"
#include "spi_max7219.h"

// Macro to use CCM (Core Coupled Memory) in STM32F4
#define CCM_RAM __attribute__((section(".ccmram")))

#define USE_FREERTOS_ISR_API

#define TASK1_STACK_SIZE 128
#define TASK2_STACK_SIZE 128
#define TASK3_STACK_SIZE 512

#define TASK1_PRIO 1
#define TASK2_PRIO 4
#define TASK3_PRIO 6

StackType_t Task1_Stack[TASK1_STACK_SIZE] CCM_RAM; /* Put task stack in CCM */
StackType_t Task2_Stack[TASK2_STACK_SIZE] CCM_RAM; /* Put task stack in CCM */
StackType_t Task3_Stack[TASK3_STACK_SIZE] CCM_RAM;

StaticTask_t Task1Buff CCM_RAM; /* Put TCB in CCM */
StaticTask_t Task2Buff CCM_RAM; /* Put TCB in CCM */
StaticTask_t Task3Buff CCM_RAM; /* Put TCB in CCM */

volatile uint16_t Recvd_word = 0;

#define BUF_SIZE       384
char gps_string[BUF_SIZE] = { 0 }; /*Buffer to store characters received from GPS*/

SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xBinarySemaphore_DMA;

void vTask1_toggleLED(void* p)
{
	while (1)
	{
		toggle_leds();
		USART_TX_string("Task1.\r\n");

#ifndef USE_FREERTOS_ISR_API
		if (Rx_Cplt_flag == 1)
		{
			USART_TX_byte(Recvd_word);
			Rx_Cplt_flag = 0;
		}
#endif
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

void vTask2_USARTRxHandler(void* p)
{
	while (1)
	{
		if ( xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
		{
			USART_TX_byte(Recvd_word);
		}
		vTaskDelay(200);
	}
	vTaskDelete(NULL);
}

void vTask_ISR_TimeParserDisp(void *p)
{
	while (1)
	{
		if ( xSemaphoreTake(xBinarySemaphore_DMA, portMAX_DELAY) == pdTRUE)
		{
			memcpy(gps_string, DMA_RX_Buffer, DMA_RX_BUFFER_SIZE);
			Display_Time_MAX7219(gps_string, "$GPRMC", 7);
			//USART_TX_string(gps_string);
		}
		vTaskDelay(700);
	}
}

int main(void)
{

	SystemInit();

	/*Initialize USART2 peripheral */
	init_USART2();
	init_led_gpios();
	enable_usart2_irq();

	/*Initialize DMA2 peripheral */
	init_dma2();
	enable_dma2_irq();

	/*Initialize USART6 peripheral, receives data from GPS */
	init_usart6_comm_module();
	init_usart6_gpio();
	enable_usart6_rx_irq();

	/*Initialize SPI module for MAX7219 display*/
	init_spi2_gpio();
	init_spi2_module();

	/*Send power up telegram over SPI to initialize MAX7219 Display*/
	init_MAX7219_powerup();

	xBinarySemaphore = xSemaphoreCreateBinary(); /*Create a semaphore for USART Rx Interrupt*/
	xBinarySemaphore_DMA = xSemaphoreCreateBinary(); /*Create a semaphore for USART6 DMA Interrupt*/

	/*Make sure Semaphore creation was successful*/
	if( (xBinarySemaphore != NULL)   && (xBinarySemaphore_DMA != NULL))
	{

		/*Create a task */
		/*Stack and TCB are placed in CCM of STM32F4 */
		/* The CCM block is connected directly to the core, which leads to zero wait states */

		xTaskCreateStatic(vTask1_toggleLED, "tsk_toggleLEDs", TASK1_STACK_SIZE,
				NULL,
				TASK1_PRIO, Task1_Stack, &Task1Buff);
		xTaskCreateStatic(vTask2_USARTRxHandler, "tsk_Rx_IRQ", TASK2_STACK_SIZE,
				NULL,
				TASK2_PRIO, Task2_Stack, &Task2Buff);

#ifdef USE_FREERTOS_ISR_API
		xTaskCreateStatic(vTask_ISR_TimeParserDisp, "tsk_GPS_DMA",
				TASK3_STACK_SIZE,
				NULL, TASK3_PRIO, Task3_Stack, &Task3Buff);
#endif

		USART_TX_string("Starting scheduler...\r\n");
		vTaskStartScheduler();  // should never return
	}

	while (1)
	{
	}
}

/*IRQ Handler for character received over USART2 Rx Line*/
void USART2_IRQHandler(void)
{
#ifndef USE_FREERTOS_ISR_API
	if (USART_GetITStatus(USART2, USART_IT_RXNE))
	{
		Rx_Cplt_flag =1;
		Recvd_word = USART_ReceiveData(USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);

	}
#else /*Freertos APIS are used*/

	static BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

	if (USART_GetITStatus(USART2, USART_IT_RXNE))
	{
		Recvd_word = USART_ReceiveData(USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

#endif
}

/*DMA IRQ HANDLER*/
void DMA2_Stream2_IRQHandler(void)
{

	static BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xBinarySemaphore_DMA, &xHigherPriorityTaskWoken);
	if (DMA_GetITStatus(DMA2_USART6_STREAM, DMA_IT_TCIF2) != RESET)
	{
		GPIO_ToggleBits(GPIOD, GPIO_Pin_15); /* Toggles Blue LED on board*/

		DMA_ClearITPendingBit(DMA2_USART6_STREAM, DMA_IT_TCIF2);
		DMA_ClearITPendingBit(DMA2_USART6_STREAM, DMA_IT_HTIF2);
		DMA_ClearITPendingBit(DMA2_USART6_STREAM, DMA_IT_FEIF2);
		DMA_ClearITPendingBit(DMA2_USART6_STREAM, DMA_IT_DMEIF2);
		DMA_ClearITPendingBit(DMA2_USART6_STREAM, DMA_IT_TEIF2);

	}
	if (xHigherPriorityTaskWoken != pdFALSE)
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void vApplicationTickHook(void)
{
}

/* vApplicationMallocFailedHook() will only be called if
 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
 function that will get called if a call to pvPortMalloc() fails.
 pvPortMalloc() is called internally by the kernel whenever a task, queue,
 timer or semaphore is created.  It is also called by various parts of the
 demo application.  If heap_1.c or heap_2.c are used, then the size of the
 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
 to query the size of free heap space that remains (although it does not
 provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook(void)
{
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
 to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
 task.  It is essential that code added to this hook function never attempts
 to block in any way (for example, call xQueueReceive() with a block time
 specified, or call vTaskDelay()).  If the application makes use of the
 vTaskDelete() API function (as this demo application does) then it is also
 important that vApplicationIdleHook() is permitted to return to its calling
 function, because it is the responsibility of the idle task to clean up
 memory allocated by the kernel to any task that has since been deleted. */
void vApplicationIdleHook(void)
{
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	 state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	 Note that, as the array is necessarily of type StackType_t,
	 configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 application must provide an implementation of vApplicationGetTimerTaskMemory()
 to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
		StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

