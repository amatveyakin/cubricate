#ifndef CLOCK_HPP
#define CLOCK_HPP


#include <SFML/System/Clock.hpp>

#include "common/time.hpp"


class Clock : private sf::Clock {
public:
  Clock() : sf::Clock() { }
  ~Clock() { }

  Time getElapsedTime() const               { return Time::fromMsec (sf::Clock::GetElapsedTime()); }
  void reset()                              { sf::Clock::Reset(); }
};


#endif
