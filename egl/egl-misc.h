#pragma once

#include <EGL/egl.h>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace egl::misc {

std::vector<std::string> SplitBySpace(std::string str);

std::string SerializeExtensions(
    const std::vector<std::string> &extensions,
    const std::set<std::string> &exclude_extensions);

template <typename OutputType, typename InputType>
OutputType *ConvertAttributes(const InputType *attrib_list,
                              std::vector<OutputType> &out_attribs) {
  if (!out_attribs.empty() && out_attribs.back() == EGL_NONE) {
    out_attribs.pop_back();
  }
  if (attrib_list) {
    while (*attrib_list != EGL_NONE) {
      out_attribs.push_back(attrib_list[0]);
      out_attribs.push_back(attrib_list[1]);
      attrib_list += 2;
    }
  }
  if (out_attribs.empty()) {
    return nullptr;
  }
  out_attribs.push_back(EGL_NONE);
  return out_attribs.data();
}

template <typename OutputType, typename InputType>
OutputType *ApendAttributes(const InputType *attrib_list,
                            std::vector<OutputType> &out_attribs) {
  return ConvertAttributes<OutputType, InputType>(attrib_list, out_attribs);
}

template <typename Type>
std::vector<Type> DupAttributes(const Type *attrib_list) {
  std::vector<Type> attribs;
  ApendAttributes(attrib_list, attribs);
  return attribs;
}

template <typename Type>
int32_t EglAttrbCount(const Type *attrib_list) {
  if (!attrib_list) {
    return 0;
  }
  auto begin = attrib_list;
  while (*attrib_list != EGL_NONE) {
    attrib_list += 2;
  }
  return std::distance(begin, attrib_list) / 2;
}

std::vector<EGLint> ConvertAttribToInt(const EGLAttrib *attrib_list);
std::vector<EGLAttrib> ConvertIntToAttrib(const EGLint *attrib_list);

std::string StringFourcc(uint32_t fourcc);

}  // namespace egl::misc