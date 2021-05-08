// File     : SPI.c
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "SPI.h"

//--------------------------------------------------------------
// Local Functions
//--------------------------------------------------------------
void CS_set(uint8_t);

//--------------------------------------------------------------
// Инициализация SPI1
//--------------------------------------------------------------
void SPI1_Init(void){

   RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;		//Тактирование на SPI
   RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   //Тактирование на порт A
   RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;   //Тактирование на порт E
   
   //---------------- MOSI MISO SCK  PA7 PA6 PA5-----------------
   GPIOA->AFR[0] |= 0x55500000;				//AF5
   GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
   GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7;
   GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 |GPIO_OTYPER_OT_7);
   GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR5 |GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);
   
   //---------------- CS PE3 -----------------
   GPIOE->OTYPER &= ~GPIO_OTYPER_OT_3;
   GPIOE->MODER |= GPIO_MODER_MODER3_0;
   GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
   GPIOE->PUPDR &= ~GPIO_PUPDR_PUPDR3;


   SPI1->CR1 &= ~SPI_CR1_BIDIMODE;	 //2-line
   SPI1->CR1 &= ~SPI_CR1_CRCEN;      //Disable CRC
   SPI1->CR1 &= ~SPI_CR1_DFF;      	 // Frame 8bit
   SPI1->CR1 |= SPI_CR1_SSI|SPI_CR1_SSM;
   SPI1->CR1 |= SPI_CR1_CPHA;
   SPI1->CR1 |= SPI_CR1_CPOL;
   SPI1->CR1 &= ~SPI_CR1_BR;
   SPI1->CR1 |= SPI_CR1_BR_0|SPI_CR1_BR_1|SPI_CR1_BR_2; //256
   SPI1->CR1 |= SPI_CR1_MSTR;
   SPI1->CR1 |= SPI_CR1_SPE;
 }

//--------------------------------------------------------------
// Передача по SPI
//--------------------------------------------------------------
uint8_t SPI1_Tx(uint8_t address, uint8_t data){
	
	CS_set(0);
	
	while(!(SPI1->SR & SPI_SR_TXE)){};
	SPI1->DR = address;
	while(!(SPI1->SR & SPI_SR_TXE)){};
  SPI1->DR = data;
	while(!(SPI1->SR & SPI_SR_RXNE)){};
	(void)SPI1->DR;
	while(!(SPI1->SR & SPI_SR_TXE))
  while((SPI1->SR & SPI_SR_BSY)){};
		
	CS_set(1);	
	return data;
}

//--------------------------------------------------------------
// Приём по SPI
//--------------------------------------------------------------
uint8_t SPI1_Rx(uint8_t address){
	
	volatile uint8_t data = 0x00;
	
	address |= 0x80;
	CS_set(0);
	
	while(!(SPI1->SR & SPI_SR_TXE)){};
	SPI1->DR = address;									//Запись первого передаваемого элемента данных в регистр SPI_DR (это очистит флаг TXE).
	while(!(SPI1->SR & SPI_SR_TXE)){};	//Ожидание, когда установится флаг TXE=1
																			//затем запись второго элемента даных для передачи(у меня нет данных здесь)
	while(!(SPI1->SR & SPI_SR_RXNE)){};	//После этого ожидание RXNE=1 и...

	(void)SPI1->DR;											//чтение SPI_DR
	SPI1->DR = 0x00;									// без этой строки не работает.причина неизвестна
  while(!(SPI1->SR & SPI_SR_RXNE)){};	//Ожидание RXNE=1, 
  while((SPI1->SR & SPI_SR_BSY)){};		// без этой строки работает не стабильно.причина неизвестна
	data=SPI1->DR;											//и затем чтение последнего принятого элемента данных.

	CS_set(1);	
	return data;
}

//--------------------------------------------------------------
// Local Functions
//--------------------------------------------------------------
void CS_set(uint8_t state){
	state ? (GPIOE->BSRRL |= GPIO_BSRR_BS_3) : (GPIOE->BSRRH |= GPIO_BSRR_BS_3) ;
}