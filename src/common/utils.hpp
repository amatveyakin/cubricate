#ifndef UTILS_HPP
#define UTILS_HPP


#include <string>
#include <sstream>
#include <stdexcept>
#include "c++0x_workaround.hpp"


#define FIX_UNUSED(x)  ((void)(x))


template <typename T>
static inline void FREE (T*& pointer) {
  delete pointer;
  pointer = nullptr;
}

template <typename T>
static inline void FREE_ARRAY (T*& pointer) {
  delete[] pointer;
  pointer = nullptr;
}


template <typename T>
static inline T sqr (T x) {
  return x * x;
}

template <typename T>
static inline T sgn (T x) {
  return (x > 0) - (x < 0);
}


template <typename T>
inline std::string stringify (T x) {
  std::ostringstream tmp_stream;
  if (!(tmp_stream << x))
    throw std::invalid_argument ("Conversation to string failed");
  return tmp_stream.str();
}


#endif
