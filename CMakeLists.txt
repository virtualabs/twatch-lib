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

  "img/img.c"
  "ui/ui.c"
  "ui/widget.c"
  "ui/image.c"
  "ui/button.c"
  "ui/label.c"
  "ui/container.c"
  "ui/progress.c"
  "ui/listbox.c"
  "font/font16.c"

  INCLUDE_DIRS "inc"

)
