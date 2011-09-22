#ifndef CPP11_FEATURE_TESTS
#define CPP11_FEATURE_TESTS


// TODO: It's ugly, but I'm too lazy to write fair tests. Hope, they'll all be redundant soon.
#ifdef WINDOWS
  #define CPP11_NULLPTR                 1
  #define CPP11_ENUM_ENCHANTMENTS       0
  #define CPP11_CLASS_STATIC_ASSERTS    0
#else // !WINDOWS
  #if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)  // GCC >= 4.6
    #define CPP11_NULLPTR               1
  #else // GCC < 4.6
    #define CPP11_NULLPTR               0
  #endif // GCC < 4.6
  #define CPP11_ENUM_ENCHANTMENTS       1
  #define CPP11_CLASS_STATIC_ASSERTS    1
#endif // !WINDOWS


#endif
