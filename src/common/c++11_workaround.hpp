#ifndef CPP11_WORKAROUND_HPP
#define CPP11_WORKAROUND_HPP

#include "common/c++11_feature_tests.hpp"

#if !CPP11_NULLPTR
const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr
#endif // !CPP11_NULLPTR

#endif
