cmake_minimum_required(VERSION 3.13)

idf_component_register(
  SRCS
  "drivers/axp202/axp20x.c"
  "hal/pmu/pmu.c"

  INCLUDE_DIRS "inc"

)
