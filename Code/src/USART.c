// File     : USART.c
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "USART.h"

//--------------------------------------------------------------
// Инициализация USART2
//--------------------------------------------------------------
void USART2_Init(){
	 RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //Тактирование на usart — 45Мгц
	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	//--------------Tx------------------------------
	
	GPIOA->MODER |= GPIO_MODER_MODER2_1; // Режим альтернативной функции
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_2;  // Двухтактный выход или push-pull сокращено PP 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR2;  // Сброс в 0
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR2_0; // Подтяжка к плюсу питания или pull-up сокращено PU
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2; // Установка скорости порта
		
	//--------------Rx------------------------------
	GPIOA->MODER |= GPIO_MODER_MODER3_1; // Режим альтернативной функции
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR3;  // Сброс в 0
	GPIOA->AFR[0] |= 0x7700;       // Назначаем ОБОИМ выводам конкретную альтернативную функцию
	
	USART2->BRR = 0x1117; 							 // Baudrate 9600
	USART2->CR1|=USART_CR1_UE|USART_CR1_TE|USART_CR1_RE; //Вкл. uart, приема и передачи    
	USART2->CR1|=USART_CR1_RXNEIE; 			 //Разрешаем генерировать прерывание по приему
	NVIC_EnableIRQ(USART2_IRQn);   			 //Включаем прерывание, указываем вектор
}
//--------------------------------------------------------------
// Передача по USART символа
//--------------------------------------------------------------
void USART2_Send (char chr){
	while (!(USART2->SR & USART_SR_TC)){};
               USART2->DR = chr;
}
//--------------------------------------------------------------
// Передача по USART строки
//--------------------------------------------------------------
void USART2_Send_String (char* str){
	     uint8_t i = 0;
	GPIOA->BSRRL |= GPIO_BSRR_BS_5;
       while (str[i])
			 {USART2_Send (str[i++]);}
			GPIOA->BSRRH |= GPIO_BSRR_BS_5;
		 }
