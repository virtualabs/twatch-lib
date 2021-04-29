#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "hal/pmu.h"

#define AXP_CHECK(x) if(x != AXP_PASS) return ESP_FAIL

/* User button handling. */
volatile bool b_axpxx_irq_triggered = false;
volatile int userbtn_int_count = 0;
portMUX_TYPE userbtn_mux = portMUX_INITIALIZER_UNLOCKED;


/**
 * _axpxx_interrupt_handler()
 *
 * Internal interrupt handler for AXP202 IRQ.
 **/

void IRAM_ATTR _axpxx_interrupt_handler(void *parameter)
{
  /* Mark AXP202 IRQ as triggered. */
  b_axpxx_irq_triggered = true;
}

/**
 * twatch_pmu_init()
 *
 * Initializes the watch Power Management Unit (AXP202) and its associated
 * I2C channel.
 **/

esp_err_t twatch_pmu_init(void)
{
  gpio_config_t irq_conf;

  /* Initialize AXP202 IRQ pin as input pin. */
  rtc_gpio_deinit(GPIO_NUM_35);
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;
  irq_conf.pin_bit_mask = (1ULL << 35);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_down_en = 0;
  irq_conf.pull_up_en = 1;
  gpio_config(&irq_conf);


  /* Install our user button interrupt handler. */
  if (gpio_install_isr_service(0) != ESP_OK)
    printf("[isr] Error while installing service\r\n");
  gpio_isr_handler_add(GPIO_NUM_35, _axpxx_interrupt_handler, NULL);

  /* Initialize I2C master communication. */
  axpxx_i2c_init();

  /* Initialize AXP202. */
  if (axpxx_probe_chip() == AXP_PASS)
  {
    /* Enable PEK short press IRQ. */
    axpxx_enableIRQ((1 << AXP202_IRQ_POKSH), true);
    axpxx_clearIRQ();

    /* Success. */
    return ESP_OK;
  }
  else
  {
    /* AXP202 not detected, we have an error. */
    return ESP_FAIL;
  }
}


/**
 * twatch_pmu_audio_power()
 *
 * Enable or disable power for T-Watch audio circuit (audio DAC)
 **/

esp_err_t twatch_pmu_audio_power(bool enable)
{
  if (axpxx_setLDO3Mode(1) == AXP_PASS)
  {
    if (axpxx_setPowerOutPut(AXP202_LDO3, enable) == AXP_PASS)
      return ESP_OK;
    else
      return ESP_FAIL;
  }
  else
    return ESP_FAIL;
}


/**
 * twatch_pmu_screen_power()
 *
 * Enable or disable power for T-Watch TFT screen.
 **/

esp_err_t twatch_pmu_screen_power(bool enable)
{
  if (axpxx_setPowerOutPut(AXP202_LDO2, (enable?1:0)) == AXP_PASS)
    return ESP_OK;
  else
    return ESP_FAIL;
}


/**
 * twatch_pmu_power()
 *
 * Enable/disable power for whole watch.
 **/

esp_err_t twatch_pmu_power(bool enable)
{
  if (enable)
  {
    /**
     * Initialize AXP202, based on official TTGO Arduino lib.
     * By default, audio circuit is off.
     **/

    AXP_CHECK(axpxx_setShutdownTime(AXP_POWER_OFF_TIME_4S));
    AXP_CHECK(axpxx_setChgLEDMode(AXP20X_LED_OFF));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_EXTEN, false));
    AXP_CHECK(axpxx_setChargeControlCur(300));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO2, AXP202_ON));
  }
  else
  {
    /* Shut power down, based on official TTGO Arduino lib. */
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_EXTEN, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO4, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_DCDC2, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO3, false));
    AXP_CHECK(axpxx_setPowerOutPut(AXP202_LDO2, false));
  }

  /* Success. */
  return ESP_OK;
}

/**
 * twatch_pmu_read_irq()
 * 
 * Fetch IRQ data from AXP202.
 * 
 **/

void twatch_pmu_read_irq(void)
{
  if (b_axpxx_irq_triggered)
  {
    /* Read IRQ. */
    if (axpxx_readIRQ() == AXP_PASS)
    {
      /* Check if user button has been pressed. */
      if (axpxx_isPEKShortPressIRQ())
      {
        userbtn_int_count = 1;
      }
    }

    /* Clear IRQ. */
    axpxx_clearIRQ();
    b_axpxx_irq_triggered = false;
  }
}

/**
 * twatch_pmu_is_userbtn_pressed(void)
 *
 * Determines if the user button has been pressed.
 *
 * @return true if user button has been pressed, false otherwise.
 **/

bool twatch_pmu_is_userbtn_pressed(void)
{
  bool result = false;

  /* Fetch IRQ data if there is any. */
  twatch_pmu_read_irq();

  /* If user button has been (short) pressed, return true. */
  if (userbtn_int_count > 0)
  {
    result = true;
    
    /* Reset interrupt counter. */
    userbtn_int_count = 0;    
  }

  return result;
}


/**
 * twatch_pmu_deepsleep()
 *
 * Enable deep sleep. Configure GPIO 35 (AXP202 IRQ) to wake up
 * the chip.
 **/

void twatch_pmu_deepsleep(void)
{
  /* Set GPIO 35 as wakeup signal. */
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);

  /* Go into deep sleep mode. */
  esp_deep_sleep_start();
}
