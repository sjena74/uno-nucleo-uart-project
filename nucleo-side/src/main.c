#include "stm32f4xx.h"

#define USART1_BAUD 9600
#define APB2_CLK 16000000UL

#define START_BYTE 0xAA

// Sensor report types (UNO -> Nucleo)
#define TYPE_ULTRASONIC 0x01
#define TYPE_PIR        0x02
#define TYPE_PHOTO      0x03

// Command types (Nucleo -> UNO)
#define CMD_RELAY    0x10
#define CMD_LED      0x11
#define CMD_SERVO    0x12
#define CMD_LCD_TEXT 0x13

#define DIST_THRESHOLD_CM 10
#define LIGHT_THRESHOLD   500

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

uint8_t computeChecksum(uint8_t type, uint8_t len, uint8_t* data) {
    uint8_t sum = type ^ len;
    for (int i = 0; i < len; i++) sum ^= data[i];
    return sum;
}

// Sends a poll byte, waits for and parses a full response packet
// Returns 1 on valid packet, 0 on checksum failure
uint8_t pollSensor(uint8_t pollType, uint8_t* outData, uint8_t* outLen) {
    USART1_SendByte(pollType);

    uint8_t start = USART1_ReceiveByte();
    if (start != START_BYTE) return 0;

    uint8_t type = USART1_ReceiveByte();
    uint8_t len  = USART1_ReceiveByte();

    uint8_t data[16];
    for (int i = 0; i < len; i++) {
        data[i] = USART1_ReceiveByte();
    }

    uint8_t checksum = USART1_ReceiveByte();
    if (checksum != computeChecksum(type, len, data)) return 0;

    for (int i = 0; i < len; i++) outData[i] = data[i];
    *outLen = len;
    return 1;
}

void sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t checksum = computeChecksum(type, len, data);
    USART1_SendByte(START_BYTE);
    USART1_SendByte(type);
    USART1_SendByte(len);
    for (int i = 0; i < len; i++) USART1_SendByte(data[i]);
    USART1_SendByte(checksum);
}

void delayMs(volatile uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 4000; i++) {}
}

int main(void) {
    LED_Init();
    USART1_Init();

    while (1) {
        uint8_t data[16];
        uint8_t len;

        // --- Ultrasonic -> relay + LCD ---
        if (pollSensor(TYPE_ULTRASONIC, data, &len)) {
            uint16_t distance = (data[0] << 8) | data[1];

            if (distance < DIST_THRESHOLD_CM) {
                uint8_t relayOn[1] = {1};
                sendCommand(CMD_RELAY, relayOn, 1);

                uint8_t msg[] = "OBJECT NEAR    ";
                sendCommand(CMD_LCD_TEXT, msg, 15);

                GPIOA->ODR |= (1 << 5); // onboard LED mirrors relay state for debug
            } else {
                uint8_t relayOff[1] = {0};
                sendCommand(CMD_RELAY, relayOff, 1);

                uint8_t msg[] = "ALL CLEAR      ";
                sendCommand(CMD_LCD_TEXT, msg, 15);

                GPIOA->ODR &= ~(1 << 5);
            }
        }
        delayMs(50);

        // --- PIR -> servo ---
        if (pollSensor(TYPE_PIR, data, &len)) {
            uint8_t motion = data[0];
            uint8_t angle[1] = { motion ? 90 : 0 };
            sendCommand(CMD_SERVO, angle, 1);
        }
        delayMs(50);

        // --- Photoresistor -> LED ---
        if (pollSensor(TYPE_PHOTO, data, &len)) {
            uint16_t light = (data[0] << 8) | data[1];
            uint8_t brightness[1] = { (light > LIGHT_THRESHOLD) ? 255 : 0 };
            sendCommand(CMD_LED, brightness, 1);
        }

        delayMs(200);
    }
}