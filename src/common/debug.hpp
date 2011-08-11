// WARNING: In contrast to assertions these functions are enabled in RelWithDebInfo build mode.
//          (but of course are disabled in Release)


#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

#include "common/clock.hpp"


/// BEGIN_TIME_MEASUREMENT / END_TIME_MEASUREMENT macro pair
/// measures time elapsed while the code \a code_ was executed.
/// If exceeds \a warnTime_ that time preceeded with \a description_ is printed.
/// \a warnTime_ should be given in milliseconds.
/// Set \a warnTime_ to 0 to print execution time anyway.

#ifdef ENABLE_DEBUG_PRINTING

#define BEGIN_TIME_MEASUREMENT                                                                      \
  {                                                                                                 \
    Clock clock;                                                                                    \
    {

#define END_TIME_MEASUREMENT(warnTime_, description_)                                               \
    }                                                                                               \
    Time timeElapsed = clock.getElapsedTime();                                                      \
    if (timeElapsed >= Time::fromMsec (warnTime_))                                                  \
      std::cerr << description_ << ": " << timeElapsed.msec() << " ms" << std::endl;                \
  }

#else // !ENABLE_DEBUG_PRINTING

#define BEGIN_TIME_MEASUREMENT {
#define END_TIME_MEASUREMENT(warnTime_, description_) }

#endif // !ENABLE_DEBUG_PRINTING

#endif
