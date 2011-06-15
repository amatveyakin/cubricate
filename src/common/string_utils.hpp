#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP


#include <string>
#include <sstream>
#include <stdexcept>


template <typename T>
inline std::string toStr (T x) {
  std::ostringstream strStream;
  if (!(strStream << x))
    throw std::invalid_argument ("Conversation to string failed");
  return strStream.str ();
}

template <typename T>
inline std::string intToStr (T x, int width) {
  std::ostringstream strStream;
  strStream.fill ('0');
  strStream.width (width);
  if (!(strStream << x))
    throw std::invalid_argument ("Conversation to string failed");
  return strStream.str ();
}


#endif
