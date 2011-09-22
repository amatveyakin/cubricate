#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP


#include <limits>


template <typename NumericT>
static inline NumericT xMin (NumericT x, NumericT y) {
  return (x < y) ? x : y;
}

template <typename NumericT>
static inline NumericT xMin (NumericT x, NumericT y, NumericT z)
{
  return xMin (x, xMin (y, z));
}

template <typename NumericT>
static inline NumericT xMin (NumericT x, NumericT y, NumericT z, NumericT t)
{
  return xMin (x, xMin (y, z, t));
}


template <typename NumericT>
static inline NumericT xMax (NumericT x, NumericT y) {
  return (x < y) ? y : x;
}

template <typename NumericT>
static inline NumericT xMax (NumericT x, NumericT y, NumericT z)
{
  return xMax (x, xMax (y, z));
}

template <typename NumericT>
static inline NumericT xMax (NumericT x, NumericT y, NumericT z, NumericT t)
{
  return xMax (x, xMax (y, z, t));
}


template <typename NumericT>
static inline NumericT xBound (NumericT min_val, NumericT value, NumericT max_val) {
  return xMin (max_val, xMax (min_val, value));
}

template <typename NumericT>
static inline NumericT xAbs (NumericT x) {
  return (x >= 0) ? x : (-x);
}

template <typename NumericT>
static inline NumericT xSqr (NumericT x) {
  return x * x;
}

template <typename NumericT>
static inline NumericT xSgn (NumericT x) {
  return (x > 0) - (x < 0);
}


static_assert ((-3) / 2 == -1, "If this assertion fails, intDivFloored and intModFloored functions will not work");

// TODO: try to speed up this function, it should be a bottleneck
// WARNING: divisor must be positive
template <typename NumericT>
inline NumericT divFloored (NumericT dividend, NumericT divisor) {
  static_assert (std::numeric_limits <NumericT>::is_integer, "This function expects an integral type");
  return (dividend - (dividend < 0) * (divisor - 1)) / divisor;
}

// TODO: try to speed up this function, it should be a bottleneck
// WARNING: divisor must be positive
template <typename NumericT>
inline NumericT modFloored (NumericT dividend, NumericT divisor) {
  static_assert (std::numeric_limits <NumericT>::is_integer, "This function expects an integral type");
  return (dividend - (dividend < 0) * (divisor - 1)) % divisor;
}


template <typename NumericT>
static inline NumericT xGCD (NumericT a, NumericT b) {
  static_assert (std::numeric_limits <NumericT>::is_integer, "This function expects an integral type");
  while (b != 0) {
    NumericT c = a % b;
    a = b;
    b = c;
  }
  return a;
}

template <typename NumericT>
static inline NumericT xLCM (NumericT a, NumericT b) {
  static_assert (std::numeric_limits <NumericT>::is_integer, "This function expects an integral type");
  // The ``result = a * b / gcd (a, b)'' code may cause an integer overflow
  NumericT result = a / xGCD (a, b);
  result *= b;
  return result;
}


#endif
