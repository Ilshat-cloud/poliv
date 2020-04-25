
#include "stm32f1xx_hal.h"
#ifndef wh1602
#define wh1602

#define     PIN_RS            GPIO_PIN_3          // PA3
#define     PIN_RS_PORT       GPIOA               
#define     PIN_EN            GPIO_PIN_4          // PA4
#define     PIN_EN_PORT       GPIOA               
#define     PIN_D7            GPIO_PIN_5          // PA5
#define     PIN_D7_PORT       GPIOA               
#define     PIN_D6            GPIO_PIN_6          // PA6
#define     PIN_D6_PORT       GPIOA               
#define     PIN_D5            GPIO_PIN_7          // PA7
#define     PIN_D5_PORT       GPIOA               
#define     PIN_D4            GPIO_PIN_0          // PB0
#define     PIN_D4_PORT       GPIOB               


void PulseLCD(void);
void SendByte(char ByteToSend, int IsData);
void Cursor(char Row, char Col);
void ClearLCDScreen(void);
void InitializeLCD(void);
void PrintStr(char *Text);
void OffLCD(void);
#endif
