#include "egl-misc.h"

#include <log/log.h>
#include <system/graphics-base.h>

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "gbm.h"

namespace {

template <typename AttribType>
std::string StringifyAttributes(const AttribType *attrib_list) {
  std::ostringstream oss;
  oss.setf(std::ios::uppercase);
  if (attrib_list) {
    for (auto attirbs = attrib_list; *attirbs != EGL_NONE; attirbs += 2) {
      oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << attirbs[0]
          << " = 0x" << attirbs[1] << ", ";
    }
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << EGL_NONE;
  }
  return oss.str();
};

template <typename AttribType>
AttribType *CheckAndFilterSpecialAttributes(
    AttribType *attributes,
    const std::map<AttribType, AttribType> &special_attribs) {
  auto attrib_count = egl::misc::EglAttrbCount(attributes);
  auto attrib_end = attributes + (attrib_count * 2 + 1);
  constexpr int32_t kStep = 2;
  for (auto pos = (attrib_count - 1) * kStep; pos >= 0; pos -= kStep) {
    auto curr_attrib = attributes + pos;
    if (auto it = special_attribs.find(*curr_attrib);
        it != special_attribs.end()) {
      auto value = *(curr_attrib + 1);
      if (value != EGL_DONT_CARE && it->second != EGL_DONT_CARE &&
          it->second != value) {
        return curr_attrib;
      }
      // remove special attribs
      std::move(curr_attrib + kStep, attrib_end, curr_attrib);
      attrib_end -= kStep;
    }
  }
  return nullptr;
}

}  // namespace

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

std::string StringifyAttributes(const EGLAttrib *attrib_list) {
  return ::StringifyAttributes<EGLAttrib>(attrib_list);
}

std::string StringifyAttributes(const EGLint *attrib_list) {
  return ::StringifyAttributes<EGLint>(attrib_list);
}

EGLint *CheckAndFilterSpecialAttributes(
    EGLint *attributes, const std::map<EGLint, EGLint> &special_attribs) {
  return ::CheckAndFilterSpecialAttributes<EGLint>(attributes, special_attribs);
}

EGLAttrib *CheckAndFilterSpecialAttributes(
    EGLAttrib *attributes,
    const std::map<EGLAttrib, EGLAttrib> &special_attribs) {
  return ::CheckAndFilterSpecialAttributes<EGLAttrib>(attributes,
                                                      special_attribs);
}

}  // namespace egl::misc