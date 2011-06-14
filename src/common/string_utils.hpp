#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <sstream>
#include <stdexcept>

template <typename T>
inline std::string stringify (T x) {
  std::ostringstream tmp_stream;
  if (!(tmp_stream << x))
    throw std::invalid_argument ("Conversation to string failed");
  return tmp_stream.str ();
}

#endif
