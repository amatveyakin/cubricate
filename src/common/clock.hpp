#ifndef CLOCK_HPP
#define CLOCK_HPP


#include <SFML/System/Clock.hpp>

#include "common/time.hpp"


class Clock : private sf::Clock {
public:
  Clock() : sf::Clock() { }
  ~Clock() { }

  Time getElapsedTime() const               { return Time::fromUsec (sf::Clock::getElapsedTime ().asMicroseconds ()); }
  void reset()                              { sf::Clock::restart (); }
};


#endif
