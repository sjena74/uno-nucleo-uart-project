#include "stm32f4xx.h"

#define USART1_BAUD 9600
#define APB2_CLK 16000000UL

void USART1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOA->MODER &= ~((3U << (9*2)) | (3U << (10*2)));
    GPIOA->MODER |=  ((2U << (9*2)) | (2U << (10*2)));

    GPIOA->AFR[1] &= ~((0xFU << ((9-8)*4)) | (0xFU << ((10-8)*4)));
    GPIOA->AFR[1] |=  ((7U << ((9-8)*4)) | (7U << ((10-8)*4)));

    USART1->BRR = APB2_CLK / USART1_BAUD;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void USART1_SendByte(uint8_t byte) {
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = byte;
}

uint8_t USART1_ReceiveByte(void) {
    while (!(USART1->SR & USART_SR_RXNE)) {}
    return (uint8_t)(USART1->DR);
}

void LED_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (5*2));
    GPIOA->MODER |=  (1U << (5*2));
}

int main(void) {
    LED_Init();
    USART1_Init();

    while (1) {
        USART1_SendByte(0x01); // poll ultrasonic

        uint8_t start = USART1_ReceiveByte();
        if (start == 0xAA) {
            GPIOA->ODR |= (1 << 5); // got a real packet back
        } else {
            GPIOA->ODR &= ~(1 << 5);
        }

        for (volatile int i = 0; i < 2000000; i++) {}
    }
}