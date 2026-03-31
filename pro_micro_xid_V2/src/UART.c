#include "UART.h"
#include <avr/io.h>

void uart_init(uint32_t baud) {
    const uint16_t ubrr = (uint16_t)((F_CPU / (8UL * baud)) - 1UL);

    UCSR1A = (1 << U2X1);
    UBRR1H = (uint8_t)(ubrr >> 8);
    UBRR1L = (uint8_t)(ubrr & 0xFF);

    UCSR1B = (1 << RXEN1) | (1 << TXEN1);
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

bool uart_rx_available(void) {
    return (UCSR1A & (1 << RXC1)) != 0;
}

uint8_t uart_rx_read(void) {
    return UDR1;
}

void uart_tx_write(uint8_t b) {
    while ((UCSR1A & (1 << UDRE1)) == 0) {
    }
    UDR1 = b;
}

void uart_tx_write_buf(const uint8_t* data, uint8_t len) {
    for (uint8_t i = 0; i < len; ++i) {
        uart_tx_write(data[i]);
    }
}
