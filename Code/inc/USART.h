//--------------------------------------------------------------
// File     : USART.h
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"

//--------------------------------------------------------------
// Глобальная функция
//--------------------------------------------------------------
void USART2_Init();
void USART2_Send (char chr);
void USART2_Send_String (char* str);