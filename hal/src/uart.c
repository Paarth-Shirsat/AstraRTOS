#include "uart.h"
#include "rcc.h"
#include "gpio.h"

//Starts only UART2
void uart_init(void){
    rcc_enable_gpio(GPIOA_EN);
    rcc_enable_uart(USART2_EN);

    GPIOA_MODER &= ~((3 << (2*2)) | (3 << (3*2))); //Clear Pins 2 and 3
    GPIOA_MODER |= ((2 << (2*2)) | (2 << (3*2))); //Sets to AF mode

    GPIOA_AFRL &= ~((0xF << (2*4)) | (0xF << (3*4))); //Clear 
    GPIOA_AFRL |= ((7 << (2*4)) | (7 << (3*4))); //Selecting AF to USART2

    USART2_BRR = 0x0683; //9600 BaudRate (Condition: Peripheral Clock = 16MHz)

    //8N1
    USART2_CR1 &= ~(1 << 12);
    USART2_CR2 &= ~(3 << 12);
    USART2_CR1 &= ~(1 << 10);

    //USART Enable, TX Enable, RX Enable
    USART2_CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}