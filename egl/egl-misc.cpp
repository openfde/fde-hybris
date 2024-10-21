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

std::string StringFourcc(uint32_t fourcc) {
  char str[5] = {};
  sprintf(str, "%c%c%c%c", (fourcc & 0x7f), ((fourcc >> 8) & 0x7f),
          ((fourcc >> 16) & 0x7f), ((fourcc >> 24) & 0x7f));
  return str;
}

}  // namespace egl::misc