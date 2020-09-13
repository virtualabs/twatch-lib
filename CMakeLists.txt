cmake_minimum_required(VERSION 3.13)

idf_component_register(
  SRCS
  "drivers/axp20x.c"
  "drivers/ft6236.c"
  "drivers/i2c.c"
  "drivers/bma423/bma.c"
  "drivers/bma423/bma4.c"
  "drivers/bma423/bma423.c"
  "drivers/st7789.c"
  "hal/audio.c"
  "hal/pmu.c"
  "hal/touch.c"
  "hal/vibrate.c"

  INCLUDE_DIRS "inc"

)
