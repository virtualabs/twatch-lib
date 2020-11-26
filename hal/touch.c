#include "esp_log.h"
#include "hal/touch.h"
#include "math.h"

#define TOUCH_TAG "[hal::touch]"

ft6236_touchpoint_t first, last;
volatile touch_state_t touch_state;
float velocity;
int16_t dx,dy;
double distance;
int duration, touch_start_ms, touch_stop_ms;

QueueHandle_t _touch_queue;

unsigned long IRAM_ATTR millis()
{
    return (unsigned long) (esp_timer_get_time() / 1000ULL);
}


/**
 * @brief Send touch report event to internal message queue.
 * @param event: pointer to a touch_event_t structure
 **/

void _touch_report_event(touch_event_t *event)
{
  xQueueSend(_touch_queue, event, 0);
}


/**
 * @brief Process touch data
 * @param touch: touch data coming from the FT6236 chip
 **/

void _process_touch_data(ft6236_touch_t *touch)
{
  touch_event_t event;
  
  switch(touch_state)
  {
    case TOUCH_STATE_CLEAR:
    {
      /* Is it a press ? */
      if (touch->touches[0].event == TOUCH_PRESS)
      {
        /* Notify touch press. */
        event.type = TOUCH_EVENT_PRESS;
        event.coords.x = first.x;
        event.coords.y = first.y;
        event.velocity = 0.0;
        _touch_report_event(&event);

        /* Save first point. */
        first.x = touch->touches[0].x;
        first.y = touch->touches[0].y;
        touch_start_ms = millis();
        touch_state = TOUCH_STATE_PRESS;

      }
    }
    break;

    case TOUCH_STATE_PRESS:
    {
      /* Is it released ? */
      if (touch->touches[0].event == TOUCH_RELEASE)
      {
        /* Notify release. */
        event.type = TOUCH_EVENT_RELEASE;
        event.coords.x = last.x;
        event.coords.y = last.y;
        event.velocity = 0.0;
        _touch_report_event(&event);


        /* If touch lasts less than 500ms, then it is a tap ! */
        touch_stop_ms = millis();

        /* Compute distance. */
        dx = last.x-first.x;
        dy = last.y-first.y;
        distance=sqrt(dx*dx+dy*dy);
        velocity = (distance*100)/(touch_stop_ms - touch_start_ms);

        if (((touch_stop_ms - touch_start_ms) < TOUCH_TAP_MAX_TIME) && (distance < TOUCH_TAP_MAX_DIST))
        {
          ESP_LOGD(TOUCH_TAG, "[!] TAP @ %d,%d", first.x, first.y);
          event.type = TOUCH_EVENT_TAP;
          event.coords.x = first.x;
          event.coords.y = first.y;
          event.velocity = 0.0;
          _touch_report_event(&event);
        }
        else
        {
          /* Check if we have a swipe. */
          if (distance >= TOUCH_SWIPE_MIN_DIST)
          {
            /* Determine direction. */
            if (abs(dx) > abs(dy))
            {
              if (dx > 0)
              {
                ESP_LOGD(TOUCH_TAG, "SWIPE RIGHT, velocity: %f", velocity);
                event.type = TOUCH_EVENT_SWIPE_RIGHT;
                event.coords.x = 0;
                event.coords.y = 0;
                event.velocity = velocity;
                _touch_report_event(&event);
              }
              else
              {
                ESP_LOGD(TOUCH_TAG, "SWIPE LEFT, velocity: %f", velocity);
                event.type = TOUCH_EVENT_SWIPE_LEFT;
                event.coords.x = 0;
                event.coords.y = 0;
                event.velocity = velocity;
                _touch_report_event(&event);
              }
            }
            else if (abs(dy) > abs(dx))
            {
              if (dy > 0)
              {
                ESP_LOGD(TOUCH_TAG, "SWIPE DOWN, velocity: %f", velocity);
                event.type = TOUCH_EVENT_SWIPE_DOWN;
                event.coords.x = 0;
                event.coords.y = 0;
                event.velocity = velocity;
                _touch_report_event(&event);
              }
              else
              {
                ESP_LOGD(TOUCH_TAG, "SWIPE UP, velocity: %f", velocity);
                event.type = TOUCH_EVENT_SWIPE_UP;
                event.coords.x = 0;
                event.coords.y = 0;
                event.velocity = velocity;
                _touch_report_event(&event);
              }
            }
          }
        }
        touch_state = TOUCH_STATE_CLEAR;
      }
      else
      {
        last.x = touch->touches[0].x;
        last.y = touch->touches[0].y;
      }
    }
    break;

    default:
      break;
  }
}


/**
 * @brief Task monitoring the FT6236 chip, and send data to _process_touch_data()
 **/

void ft6x36_monitor(void *parameter)
{
  ft6236_touch_t touch;

  while(true){
  	if(ft6x36_read_touch_data(&touch)){
      _process_touch_data(&touch);
  	}
  }
  vTaskDelete( NULL );
}


/**
 * @brief Initialize touch abstraction layer
 * @retval Always ESP_OK
 **/

esp_err_t twatch_touch_init(void)
{
  /* Create event queue. */
  _touch_queue  = xQueueCreate(10, sizeof(touch_event_t));

  /* Initialize touch state. */
  touch_state = TOUCH_STATE_CLEAR;

  /* Initialize our FT6236. */
  ft6x36_init(FT6236_I2C_SLAVE_ADDR);

  /* Create a task to monitor the touch screen and process events. */
  xTaskCreate(ft6x36_monitor, "ft6x36_monitor", 10000, NULL, 1, NULL);

  return ESP_OK;
}


/**
 * @brief Get touch event
 * @param event: pointer to a touch_event_t structure to fill
 * @param ticks_to_wait: number of ticks to wait
 * @retval ESP_OK if an event has been retrieved, ESP_FAIL otherwise
 **/

esp_err_t twatch_get_touch_event(touch_event_t *event, TickType_t ticks_to_wait)
{
  if (xQueueReceive(_touch_queue, event, ticks_to_wait))
  {
    return ESP_OK;
  }
  else
    return ESP_FAIL;
}
