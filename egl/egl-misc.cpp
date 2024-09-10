#include "egl-misc.h"

#include <log/log.h>
#include <system/graphics-base.h>

#include <algorithm>
#include <iterator>
#include <sstream>

#include "gbm.h"

namespace egl::misc {

std::vector<std::string> SplitBySpace(std::string str) {
  std::vector<std::string> words;
  std::istringstream iss{std::move(str)};
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(), std::back_inserter(words));
  return words;
}

int32_t GetHalFromFromGbmFormat(int32_t gbm_format) {
  switch (gbm_format) {
    // case GBM_FORMAT_R8:
    // case GBM_FORMAT_R16:
    // case GBM_FORMAT_GR88:
    // case GBM_FORMAT_GR1616:
    // case GBM_FORMAT_ARGB1555:
    case GBM_FORMAT_RGB565:
      return HAL_PIXEL_FORMAT_RGB_565;
    case GBM_FORMAT_XRGB8888:
      return HAL_PIXEL_FORMAT_RGB_888;
    case GBM_FORMAT_ARGB8888:
      return HAL_PIXEL_FORMAT_BGRA_8888;
    case GBM_FORMAT_ABGR8888:
      return HAL_PIXEL_FORMAT_RGBA_8888;
    case GBM_FORMAT_XBGR8888:
      return HAL_PIXEL_FORMAT_RGBX_8888;
    // case GBM_FORMAT_XBGR16161616:
    case GBM_FORMAT_XBGR16161616F:
    case GBM_FORMAT_ABGR16161616F:
      return HAL_PIXEL_FORMAT_RGBA_FP16;
    // case GBM_FORMAT_XRGB2101010:
    // case GBM_FORMAT_ARGB2101010:
    case GBM_FORMAT_XBGR2101010:
    case GBM_FORMAT_ABGR2101010:
      return HAL_PIXEL_FORMAT_RGBA_1010102;
    default:
      ALOGW("unsupported gbm buffer format 0x%08X", gbm_format);
  }
  return EGL_DONT_CARE;
}

int32_t GetGbmFormatFromHalFormat(int32_t hal_format) {
  switch (hal_format) {
    case HAL_PIXEL_FORMAT_RGB_565:
      return GBM_FORMAT_RGB565;
    case HAL_PIXEL_FORMAT_BGRA_8888:
      return GBM_FORMAT_ARGB8888;
    case HAL_PIXEL_FORMAT_RGBA_8888:
      return GBM_FORMAT_ABGR8888;
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
      /*
       * HACK: Hardcode this to RGBX_8888 as per cros_gralloc hack.
       * TODO: Remove this once https://issuetracker.google.com/32077885 is
       * fixed.
       */
    case HAL_PIXEL_FORMAT_RGBX_8888:
      return GBM_FORMAT_XBGR8888;
    case HAL_PIXEL_FORMAT_RGBA_FP16:
      return GBM_FORMAT_ABGR16161616F;
    case HAL_PIXEL_FORMAT_RGBA_1010102:
      return GBM_FORMAT_ABGR2101010;
    default:
      break;
  }
  return EGL_DONT_CARE;
}

std::string SerializeExtensions(
    const std::vector<std::string> &extensions,
    const std::set<std::string> &exclude_extensions) {
  std::ostringstream oss;
  std::copy_if(extensions.cbegin(), extensions.cend(),
               std::ostream_iterator<std::string>(oss, " "),
               [&exclude_extensions](const std::string &ext) {
                 return exclude_extensions.find(ext) ==
                        exclude_extensions.end();
               });
  return oss.str();
}

std::vector<EGLint> ConvertAttribToInt(const EGLAttrib *attrib_list) {
  std::vector<EGLint> attribs;
  ConvertAttributes(attrib_list, attribs);
  return attribs;
}

std::vector<EGLAttrib> ConvertIntToAttrib(const EGLint *attrib_list) {
  std::vector<EGLAttrib> attribs;
  ConvertAttributes(attrib_list, attribs);
  return attribs;
}

}  // namespace egl::misc