#ifndef __INC_DRIVERS_UART_H
#define __INC_DRIVERS_UART_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#define TWATCH_UART_TX   GPIO_NUM_26
#define TWATCH_UART_RX   GPIO_NUM_36

#define EX_UART_NUM      UART_NUM_1
#define BUF_SIZE         (1024)
#define RD_BUF_SIZE      (BUF_SIZE)

esp_err_t twatch_uart_init(int baudrate);
esp_err_t twatch_uart_transmit(uint8_t *p_buffer, int len);
esp_err_t twatch_uart_receive(uint8_t *p_buffer, int len);
bool twatch_uart_wait_event(uart_event_t *p_uart_event);

#endif /* __INC_DRIVERS_UART_H */