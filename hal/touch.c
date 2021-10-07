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
RTC_DATA_ATTR static bool b_inverted=false;

/* b_touched: used by ISR to notify touch HAL some touch data need to be retrieved. */
volatile bool b_touched = false;
volatile ft6236_touch_t touch_data;

/*
 b_swipe_sent: true if we already sent a swipe event.

 This boolean is used to avoid sending multiple swipe events during the same swipe
 movement. It is reset on TOUCH_RELEASE.
*/
volatile bool b_swipe_sent = false;

QueueHandle_t _touch_queue;

unsigned long IRAM_ATTR millis()
{
    return (unsigned long) (esp_timer_get_time() / 1000ULL);
}

/**
 * @brief IRQ handler ISR
 **/

void _touch_irq_handler(void)
{
  b_touched = true;
}


/**
 * @brief Send touch report event to internal message queue.
 * @param event: pointer to a touch_event_t structure
 **/

void _touch_report_event(touch_event_t *event)
{
  /* Invert coordinates if required. */
  if (b_inverted)
  {
    event->coords.x = (TOUCH_MAX_X - event->coords.x);
    event->coords.y = (TOUCH_MAX_Y - event->coords.y);
  }

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
      if ((touch->touches[0].event == TOUCH_PRESS) || (touch->touches[0].event == TOUCH_CONTACT))
      {
        /* Save first point. */
        first.x = touch->touches[0].x;
        first.y = touch->touches[0].y;
        touch_start_ms = millis();
        touch_state = TOUCH_STATE_PRESS;

        /* Notify touch press. */
        event.type = TOUCH_EVENT_PRESS;
        event.coords.x = first.x;
        event.coords.y = first.y;
        event.velocity = 0.0;
        _touch_report_event(&event);
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

        if (((touch_stop_ms - touch_start_ms) < TOUCH_TAP_MAX_TIME) && (distance < TOUCH_TAP_MAX_DIST))
        {
          ESP_LOGD(TOUCH_TAG, "[!] TAP @ %d,%d", first.x, first.y);
          event.type = TOUCH_EVENT_TAP;
          event.coords.x = first.x;
          event.coords.y = first.y;
          event.velocity = 0.0;
          _touch_report_event(&event);
        }

        /* Rearm b_swipe_sent. */
        b_swipe_sent = false;

        touch_state = TOUCH_STATE_CLEAR;
      }
      else
      {
        last.x = touch->touches[0].x;
        last.y = touch->touches[0].y;

        /* If touch lasts less than 500ms, then it is a tap ! */
        touch_stop_ms = millis();

        /* Compute distance. */
        dx = last.x-first.x;
        dy = last.y-first.y;
        distance=sqrt(dx*dx+dy*dy);
        if ((touch_stop_ms - touch_start_ms) > 0)
        {
          velocity = (distance*100)/(touch_stop_ms - touch_start_ms);
        }
        else
          velocity = 0.0;

        /* Check if we have a swipe. */
        if ((velocity >= TOUCH_SWIPE_MIN_VELOCITY) && !b_swipe_sent)
        {
          /* Determine direction. */
          if (abs(dx) > abs(dy))
          {
            if (dx > 0)
            {
              ESP_LOGD(TOUCH_TAG, "SWIPE RIGHT, velocity: %f", velocity);
              event.type = (b_inverted?TOUCH_EVENT_SWIPE_LEFT:TOUCH_EVENT_SWIPE_RIGHT);
              event.coords.x = first.x;
              event.coords.y = first.y;
              event.velocity = velocity;
              _touch_report_event(&event);

              /* Mark swipe event as sent. */
              b_swipe_sent = true;
            }
            else
            {
              ESP_LOGD(TOUCH_TAG, "SWIPE LEFT, velocity: %f", velocity);
              event.type = (b_inverted?TOUCH_EVENT_SWIPE_RIGHT:TOUCH_EVENT_SWIPE_LEFT);
              event.coords.x = first.x;
              event.coords.y = first.y;
              event.velocity = velocity;
              _touch_report_event(&event);

              /* Mark swipe event as sent. */
              b_swipe_sent = true;
            }
          }
          else if (abs(dy) > abs(dx))
          {
            if (dy > 0)
            {
              ESP_LOGD(TOUCH_TAG, "SWIPE DOWN, velocity: %f", velocity);
              event.type = (b_inverted?TOUCH_EVENT_SWIPE_UP:TOUCH_EVENT_SWIPE_DOWN);
              event.coords.x = first.x;
              event.coords.y = first.y;
              event.velocity = velocity;
              _touch_report_event(&event);

              /* Mark swipe event as sent. */
              b_swipe_sent = true;
            }
            else
            {
              ESP_LOGD(TOUCH_TAG, "SWIPE UP, velocity: %f", velocity);
              event.type = (b_inverted?TOUCH_EVENT_SWIPE_DOWN:TOUCH_EVENT_SWIPE_UP);
              event.coords.x = first.x;
              event.coords.y = first.y;
              event.velocity = velocity;
              _touch_report_event(&event);

              /* Mark swipe event as sent. */
              b_swipe_sent = true;
            }
          }
        }

        /* Notify touch press. */
        event.type = TOUCH_EVENT_PRESS;
        event.coords.x = last.x;
        event.coords.y = last.y;
        event.velocity = 0.0;
        _touch_report_event(&event);
      }
    }
    break;

    default:
      break;
  }
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

  twatch_pmu_reset_touchscreen();

  /* Initialize our FT6236. */
  ft6x36_init(FT6236_I2C_SLAVE_ADDR, (FT6X36_IRQ_HANDLER)_touch_irq_handler);

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
  /* Handle IRQ if any. */
  if (b_touched)
  {
    /* Read touch data. */
    ft6x36_read((ft6236_touch_t *)&touch_data);

    _process_touch_data((ft6236_touch_t *)&touch_data);

    /* Reset b_touched. */
    b_touched = false;
  }

  /* Do we have some event to process ? */
  if (xQueueReceive(_touch_queue, event, ticks_to_wait))
  {
    return ESP_OK;
  }
  else
    return ESP_FAIL;
}

/**
 * @brief Set touch screen as inverted (or not).
 * @param inverted: true if touch screen is inverted, false otherwise
 **/

void twatch_touch_set_inverted(bool inverted)
{
  b_inverted = inverted;
}

bool twatch_touch_is_inverted(void)
{
  return b_inverted;
}
