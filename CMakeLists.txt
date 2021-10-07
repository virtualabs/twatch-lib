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
  "drivers/pcf8563.c"
  "drivers/ir/esp32_rmt_common.c"
  "drivers/ir/rmtlib_raw.c"
  
  "hal/hal.c"
  "hal/audio.c"
  "hal/pmu.c"
  "hal/touch.c"
  "hal/vibrate.c"
  "hal/screen.c"
  "hal/rtc.c"

  "img/img.c"
  "ui/ui.c"
  "ui/modal.c"
  "ui/widget.c"
  "ui/image.c"
  "ui/button.c"
  "ui/label.c"
  "ui/container.c"
  "ui/progress.c"
  "ui/scrollbar.c"
  "ui/listbox.c"
  "ui/frame.c"
  "ui/slider.c"
  "ui/switch.c"
  "font/font16.c"

  INCLUDE_DIRS "inc"

)
