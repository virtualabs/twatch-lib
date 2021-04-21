#ifndef __INC_TWATCH_TOUCH_H
#define __INC_TWATCH_TOUCH_H

#if 0
  #define TOUCH_RELEASE 0
  #define TOUCH_CONTACT
  #define TOUCH_PRESS 2
#else
  #define TOUCH_RELEASE 0
  #define TOUCH_CONTACT 2
  #define TOUCH_PRESS 0
#endif

#define TOUCH_TAP_MAX_TIME  300
#define TOUCH_TAP_MAX_DIST  5
#define TOUCH_SWIPE_MIN_DIST 10
#define TOUCH_SWIPE_MIN_VELOCITY 20

/* Maximum X/Y values. */
#define TOUCH_MAX_X 240
#define TOUCH_MAX_Y 240

#include "drivers/ft6236.h"
#include "freertos/queue.h"

typedef enum {
  TOUCH_STATE_CLEAR,
  TOUCH_STATE_PRESS,
  TOUCH_STATE_RELEASE
} touch_state_t;

typedef enum {
  TOUCH_EVENT_PRESS,
  TOUCH_EVENT_RELEASE,
  TOUCH_EVENT_TAP,
  TOUCH_EVENT_SWIPE_LEFT,
  TOUCH_EVENT_SWIPE_RIGHT,
  TOUCH_EVENT_SWIPE_UP,
  TOUCH_EVENT_SWIPE_DOWN
} touch_event_type_t;


/**
 * Touch event coordinates (used for taps)
 **/

typedef struct {
  uint16_t x;
  uint16_t y;
} touch_event_coords_t;

/**
 * Touch event structure.
 **/

typedef struct {
  /* Event type. */
  touch_event_type_t type;

  /* Tap coordinates. */
  touch_event_coords_t coords;

  /* Swipe velocity. */
  float velocity;
} touch_event_t;

/* Initialize Touch HAL. */
esp_err_t twatch_touch_init(void);

/* Set touch screen as inverted. */
void twatch_touch_set_inverted(bool inverted);

/* Retrieve Touch event (if any). */
esp_err_t twatch_get_touch_event(touch_event_t *event, TickType_t ticks_to_wait);


#endif /* __INC_TWATCH_TOUCH_H */
