cmake_minimum_required(VERSION 3.13)

idf_component_register(
  SRCS
  "drivers/axp20x.c"
  "drivers/ft6236.c"
  "drivers/i2c.c"
  "hal/audio.c"
  "hal/pmu.c"
  "hal/touch.c"

  INCLUDE_DIRS "inc"

)
