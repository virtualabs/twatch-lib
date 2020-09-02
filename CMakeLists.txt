cmake_minimum_required(VERSION 3.13)

idf_component_register(
  SRCS
  "drivers/axp20x.c"
  "drivers/i2c.c"
  "hal/pmu.c"

  INCLUDE_DIRS "inc"

)
