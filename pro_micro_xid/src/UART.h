#pragma once

#include <stdint.h>
#include <stdbool.h>

void uart_init(uint32_t baud);
bool uart_rx_available(void);
uint8_t uart_rx_read(void);
void uart_tx_write(uint8_t b);
void uart_tx_write_buf(const uint8_t* data, uint8_t len);
