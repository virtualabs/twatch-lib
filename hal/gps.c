#include "hal/gps.h"

#define TAG "[hal::gps]"

typedef enum {
  GPS_IDLE,
  GPS_ON,
  GPS_WOKEN_UP,
  GPS_PING,
  GPS_READY
} gps_state_t;


/* GPS state (FSM). */
volatile gps_state_t g_gps_state;


static void twatch_gps_control(void *pvParameters)
{
  uart_event_t uart_evt;
  uint8_t buffer[256];
  for(;;)
  {

    switch(g_gps_state)
    {
      case GPS_IDLE:
        break;

      case GPS_ON:
        {
          ESP_LOGI(TAG, "Waking up GPS ...");
          /* Wake-up GPS. */
          vTaskDelay(60/portTICK_RATE_MS);
          gpio_set_level(TWATCH_GPS_WAKEUP, 0);
          vTaskDelay(200/portTICK_RATE_MS);
          gpio_set_level(TWATCH_GPS_WAKEUP, 1);

          /* GPS should be woken up. */
          ESP_LOGI(TAG, "GPS has been woken up !");
          g_gps_state = GPS_WOKEN_UP;
        }
        break;

      case GPS_WOKEN_UP:
        /* GPS is woken up, send hello. */
        ESP_LOGI(TAG, "Sending hello ...");
        twatch_uart_transmit((uint8_t *)"$PGKC462*2F\r\n", 13);
        g_gps_state = GPS_PING;
        break;

      case GPS_PING:
        {
          if(twatch_uart_wait_event(&uart_evt))
          {
            ESP_LOGI(TAG, "uart event: %d", uart_evt.type);
            if (uart_evt.type == UART_DATA)
            {
              twatch_uart_receive(buffer, uart_evt.size);
              ESP_LOGI(TAG, "received %d bytes", uart_evt.size);
              buffer[uart_evt.size] = 0;
              ESP_LOGI(TAG, "data: %s", (char *)buffer);
            }
          }
        }
        break;

      case GPS_READY:
        if(twatch_uart_wait_event(&uart_evt))
        {
          ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
          switch(uart_evt.type)
          {
            case UART_DATA:
              break;

            default:
              break;
          }
        }
        break;
    }

    vTaskDelay(10);
  }
}

esp_err_t twatch_gps_init(void)
{
  /* Initialize UART. */
  twatch_uart_init(9600);

  /* Initialize wake-up GPIO. */
  gpio_config_t gps_wake_up;

  /* Configure wake-up GPIO. */
  gps_wake_up.mode = GPIO_MODE_OUTPUT;
  gps_wake_up.pin_bit_mask = (1ULL << TWATCH_GPS_WAKEUP);
  gpio_config(&gps_wake_up);
  gpio_set_level(TWATCH_GPS_WAKEUP, 1);

  /* GPS is idling. */
  g_gps_state = GPS_IDLE;

  /* Start our GPS controller. */
  return xTaskCreate(twatch_gps_control, "twatch_gps_control", 10000, NULL, 12, NULL);
}

esp_err_t twatch_gps_on(void)
{
  esp_err_t result = ESP_FAIL;

  if (g_gps_state == GPS_IDLE)
  {
    ESP_LOGI(TAG, "switching on GPS ...");

    /* Power on GPS through LDO4. */
    result = twatch_pmu_gps_power(true);
    vTaskDelay(200/portTICK_RATE_MS);

    /* Change current state. */
    g_gps_state = GPS_ON;
  }
  else
    ESP_LOGE(TAG, "GPS is already on !");

  return result;
}

esp_err_t twatch_gps_off(void)
{
  esp_err_t result;

  /* Power off GPS through LDO4. */
  ESP_LOGI(TAG, "switching off GPS ...");
  result = twatch_pmu_gps_power(false);

  /* Mark GPS as powered off. */
  if (result == ESP_OK)
    g_gps_state = GPS_IDLE;

  /* Return result. */
  return result;
}
