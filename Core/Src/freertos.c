/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stream_buffer.h"
#include "usart.h"
#include "keyled.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define  BUFFER_LEN     80			//流缓存区大小，字节数
#define  TRIGGER_LEVEL  20			//触发水平，字节数
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
StreamBufferHandle_t  streamBuf;	//流缓冲区句柄变量
/* USER CODE END Variables */
/* Definitions for Task_Main */
osThreadId_t Task_MainHandle;
const osThreadAttr_t Task_Main_attributes = {
  .name = "Task_Main",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void AppTask_Main(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Main */
  Task_MainHandle = osThreadNew(AppTask_Main, NULL, &Task_Main_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  streamBuf=xStreamBufferCreate(BUFFER_LEN, TRIGGER_LEVEL);   //创建流缓存区
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_AppTask_Main */
/**
  * @brief  Function implementing the Task_Main thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_AppTask_Main */
void AppTask_Main(void *argument)
{
	/* USER CODE BEGIN AppTask_Main */
	/* Infinite loop */
	printf("Stream Buffer length= %d\r\n",BUFFER_LEN);
	printf("Trigger Level= %d\r\n",TRIGGER_LEVEL);

	uint16_t requiredBytes=32;  //需要≥TRIGGER_LEVEL=20;
	printf("Required bytes= %d\r\n",requiredBytes);

	uint32_t adcArray[10];			//最多读取10个数据点求平均，40字节

	for(;;)
	{
		uint16_t actualReadBytes=xStreamBufferReceive(streamBuf, adcArray,
				requiredBytes, portMAX_DELAY);
		LED1_Toggle();
		printf("Actual read bytes= %d\r\n",actualReadBytes);

		uint8_t	 actualItems=actualReadBytes/4;   //实际的数据点个数, 每个数据点4字节
		uint32_t sum=0;
		for( uint8_t i=0; i<actualItems; i++)
			sum += adcArray[i];
		sum= sum/actualItems;  //计算平均值
		printf("Average ADC Value= %ld\r\n",sum);//显示平均值
	}
	/* USER CODE END AppTask_Main */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance != ADC3)
		return;

	uint32_t adc_value=HAL_ADC_GetValue(hadc);	//ADC转换原始数据
	BaseType_t  highTaskWoken=pdFALSE;
	if (streamBuf != NULL)
	{
		xStreamBufferSendFromISR(streamBuf, &adc_value, 4, &highTaskWoken);
		portYIELD_FROM_ISR(highTaskWoken);  		//申请进行任务切换
	}
}

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart3,(uint8_t*)&ch,1,0xFFFF);
	return ch;
}
/* USER CODE END Application */

