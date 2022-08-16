#include "drivers/uart.h"

static QueueHandle_t g_uart_queue;

esp_err_t twatch_uart_init(int baudrate)
{
  uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &g_uart_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    /* Configure UART0. */
    uart_set_pin(
      EX_UART_NUM,
      TWATCH_UART_TX,
      TWATCH_UART_RX,
      -1,
      -1
    );

  /* Success. */
  return ESP_OK;
}

esp_err_t twatch_uart_transmit(uint8_t *p_buffer, int len)
{
  return uart_write_bytes(EX_UART_NUM, (const char*)p_buffer, len);
}

esp_err_t twatch_uart_receive(uint8_t *p_buffer, int len)
{
  return uart_read_bytes(EX_UART_NUM, p_buffer, len, portMAX_DELAY);
}

bool twatch_uart_wait_event(uart_event_t *p_uart_event)
{
  return xQueueReceive(g_uart_queue, (void * )p_uart_event, (portTickType)portMAX_DELAY);
}