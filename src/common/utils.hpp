#ifndef UTILS_HPP
#define UTILS_HPP


#include "common/c++0x_workaround.hpp"


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


#if defined CLIENT_APP && defined SERVER_APP
  #error "Both client & server compilation modes are active!"
#endif

#if !defined CLIENT_APP && !defined SERVER_APP
  #error "None of client & server compilation modes is active!"
#endif


#ifdef CLIENT_APP
  #define CLIENT_STATEMENT(code__)  code__
#else
  #define CLIENT_STATEMENT(code__)  do { } while (0)
#endif

#ifdef SERVER_APP
  #define SERVER_STATEMENT(code__)  code__
#else
  #define SERVER_STATEMENT(code__)  do { } while (0)
#endif


#endif
