#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "SPI.h"
#include "USART.h"

uint8_t LIS3DSHTR_Init(uint8_t address);

void GPIO_Init();

static void vReceiverTask( void *pvParameters );
static void vSenderTask( void *pvParameters );

xQueueHandle xQueue; //объявление очереди

int main( void ){
	
	GPIO_Init();
	SPI1_Init();
	LIS3DSHTR_Init(0x0F); //возвращает значение ID регистра
	USART2_Init();
	
	
 xQueue = xQueueCreate( 3, sizeof( xCoordinates ) ); //инициализация очереди
	
  if( xQueue != NULL ){
     
     xTaskCreate( vSenderTask, "Sender", 1000, NULL, 2, NULL );
    
     /* Создание задачи, которая будет читать из очереди. Задача создается
        с приоритетом 1, что ниже приоритета задач, помещающих данные
        в очередь. */
     xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 1, NULL );

     /* Запуск шедулера, после чего созданные задачи начнут выполняться. */
     vTaskStartScheduler();
  }
  else{
     /* Очередь не может быть создана. */
  }
  for( ;; );
}
	

void GPIO_Init(){// PD12 PD13 PD14 PD15 output PP enabled 

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;   
	
	 //GPIOE->OTYPER &= ~GPIO_OTYPER_OT_3;
   GPIOD->MODER |= GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0;
   GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR12 | GPIO_OSPEEDER_OSPEEDR13 | GPIO_OSPEEDER_OSPEEDR14 | GPIO_OSPEEDER_OSPEEDR15;
   //GPIOE->PUPDR &= ~GPIO_PUPDR_PUPDR3;
}

uint8_t LIS3DSHTR_Init(uint8_t address){
	
	uint8_t data;
	SPI1_Tx(0x24,0x00);		//обнуление CTRL_REG5
	SPI1_Tx(0x20,0x00);		//обнуление CTRL_REG4
	
	SPI1_Tx(0x24,0x48);		//CTRL_REG5; Anti-aliasing filter bandwidth = 200Hz.  Full-scale selection = +/- 4g
	SPI1_Tx(0x20,0x67);		//CTRL_REG4; output data rate selection = 100Hz. Z,Y,X-axis enabled
	
	data = SPI1_Rx(address);	//чтение данных из *address*
	return data;
}




static void vSenderTask( void *pvParameters ){
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 100 / portTICK_RATE_MS; //время ожидания в состоянии Blocked
	xCoordinates Coordinates;  //структура для координат

  for( ;; ){
		
     Coordinates.X = SPI1_Rx(0x29);
		 Coordinates.Y = SPI1_Rx(0x2B);
		
		xStatus = xQueueSendToBack( xQueue, &Coordinates, xTicksToWait ); //отправка данных в очередь
		if( xStatus != pdPASS ){ /*ошибка*/  }

     taskYIELD();
  }
}

static void vReceiverTask( void *pvParameters ){

	xCoordinates xReceivedStructure; //структура для временного хранения значений координат
portBASE_TYPE xStatus;
	
char X_buf[8], Y_buf[8]; 

  for( ;; ){
    
     if( uxQueueMessagesWaiting( xQueue ) != 3 ){
        /* очередь переполнена */
     }


     xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 );

     if( xStatus == pdPASS ){/* Данные были успешно приняты из очереди.*/	
			 
			 /*перевод в char[8] численных переменных координат*/
			 sprintf(X_buf,"%d",xReceivedStructure.X);
			 sprintf(Y_buf,"%d",xReceivedStructure.Y);
			 /*---отправка в USART2----*/
			 USART2_Send_String(X_buf);
			 USART2_Send_String("\t");
			 USART2_Send_String(Y_buf);
			 USART2_Send_String ("\r\n");
			 
			 /*---вкл/выкл led на PD12 PD13 PD14 PD15 взависимости от наклона платы---*/
			 	if (xReceivedStructure.X<-15) GPIOD->BSRRL |= GPIO_BSRR_BS_12;
				else GPIOD->BSRRH |= GPIO_BSRR_BS_12;
				if (xReceivedStructure.X>15) GPIOD->BSRRL |= GPIO_BSRR_BS_14;
				else GPIOD->BSRRH |= GPIO_BSRR_BS_14;
				if (xReceivedStructure.Y<-15) GPIOD->BSRRL |= GPIO_BSRR_BS_15;
				else GPIOD->BSRRH |= GPIO_BSRR_BS_15;
				if (xReceivedStructure.Y>15) GPIOD->BSRRL |= GPIO_BSRR_BS_13;
				else GPIOD->BSRRH |= GPIO_BSRR_BS_13;
			 }
     else{
         /* Из очереди ничего не принято. Это должно означать ошибку, так как
            эта задача может быть запущена только при заполненной очереди. */
     }
  }
}
