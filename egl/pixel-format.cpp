#include "pixel-format.h"

#include <system/graphics-base.h>

namespace egl {

HalPixelFormat::HalPixelFormat(int32_t format) : format_(format) {
  switch (format_) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
      red_size_ = 8;
      green_size_ = 8;
      blue_size_ = 8;
      alpha_size_ = 8;
      break;
    case HAL_PIXEL_FORMAT_RGBX_8888:
      red_size_ = 8;
      green_size_ = 8;
      blue_size_ = 8;
      alpha_size_ = 0;
      break;
    case HAL_PIXEL_FORMAT_RGB_565:
      red_size_ = 5;
      green_size_ = 6;
      blue_size_ = 5;
      alpha_size_ = 0;
      break;
    case HAL_PIXEL_FORMAT_RGBA_1010102:
      red_size_ = 10;
      green_size_ = 10;
      blue_size_ = 10;
      alpha_size_ = 2;
      break;
    default:
      break;
  }
}

bool HalPixelFormat::BuildFormat(int32_t red_size, int32_t green_size,
                                 int32_t blue_size, int32_t alpha_size) {
  int32_t format = -1;
  if ((red_size == 8) && (green_size == 8) && (blue_size == 8) &&
      (alpha_size == 8)) {
    format = HAL_PIXEL_FORMAT_RGBA_8888;
  } else if ((red_size == 8) && (green_size == 8) && (blue_size == 8) &&
             (alpha_size == 0)) {
    format = HAL_PIXEL_FORMAT_RGBX_8888;
  } else if ((red_size == 5) && (green_size == 6) && (blue_size == 5) &&
             (alpha_size == 0)) {
    format = HAL_PIXEL_FORMAT_RGB_565;
  } else if ((red_size == 10) && (green_size == 10) && (blue_size == 10) &&
             (alpha_size == 2)) {
    format = HAL_PIXEL_FORMAT_RGBA_1010102;
    // } else if ((red_size == 5) && (green_size == 5) && (blue_size == 5) &&
    //            (alpha_size == 1)) {
    //   format = HAL_PIXEL_FORMAT_RGBA_5551;
    // } else if ((red_size == 4) && (green_size == 4) && (blue_size == 4) &&
    //            (alpha_size == 4)) {
    //   format = HAL_PIXEL_FORMAT_RGBA_4444;
  } else {
    return false;
  }

  format_ = format;
  red_size_ = red_size;
  green_size_ = green_size;
  blue_size_ = blue_size;
  alpha_size_ = alpha_size;
  return true;
}

}  // namespace egl