#ifndef LINEAR_ALGEBRA
#define LINEAR_ALGEBRA


// TODO: check disassemble for variants ``foo (Vec2d)'' and ``foo (const Vec2d&)''
//      (and better use the second one: the underlying class may potentially be quite complex)


#include <cmath>
#include <limits>
#include "math_utils.hpp"


template <typename VectorT, typename ElementT>
class LinearVectorOperations {
public:
  typedef ElementT ElementType;
  typedef VectorT  VectorType;

  VectorType& operator+= (VectorType a) {
    for (int i = 0; i < VectorType::DIMENSION; ++i)
      derived ().coords[i] += a.coords[i];
    return derived ();
  }

  VectorType& operator-= (VectorType a) {
    for (int i = 0; i < VectorType::DIMENSION; ++i)
      derived ().coords[i] -= a.coords[i];
    return derived ();
  }

  VectorType& operator*= (ElementType q) {
    for (int i = 0; i <VectorType:: DIMENSION; ++i)
      derived ().coords[i] *= q;
    return derived ();
  }

  VectorType& operator/= (ElementType q) {
    for (int i = 0; i < VectorType::DIMENSION; ++i)
      derived ().coords[i] /= q;
    return derived ();
  }

  VectorType operator+ (VectorType a) const {
    VectorType result (derived ());
    result += a;
    return result;
  }

  VectorType operator- (VectorType a) const {
    VectorType result (derived ());
    result -= a;
    return result;
  }

  VectorType operator* (ElementType q) const {
    VectorType result (derived ());
    result *= q;
    return result;
  }

  VectorType operator/ (ElementType q) const {
    VectorType result (derived ());
    result /= q;
    return result;
  }

private:
  const VectorType& derived () const    { return static_cast< const VectorType& > (*this); }
  VectorType& derived ()                { return static_cast< VectorType& > (*this); }
};

template <typename VectorT, typename ElementT>
class VectorConversations {
public:
  typedef ElementT ElementType;
  typedef VectorT  VectorType;

  void fromArray (const ElementType* coords__) {
    for (int i = 0; i < VectorType::DIMENSION; ++i)
      derived ().coords[i] = coords__[i];
  }

  void toArray (ElementType* coords__) const {
    for (int i = 0; i < VectorType::DIMENSION; ++i)
      coords__[i] = derived ().coords[i];
  }

private:
  const VectorType& derived () const    { return static_cast< const VectorType& > (*this); }
  VectorType& derived ()                { return static_cast< VectorType& > (*this); }
};



template <typename ElementT>
struct Vec2Base : public LinearVectorOperations <Vec2Base <ElementT>, ElementT>,
                  public VectorConversations    <Vec2Base <ElementT>, ElementT> {
  static const int DIMENSION = 2;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y;
    };
    ElementType coords[DIMENSION];
  };

  Vec2Base() : x (0), y (0) { }
  Vec2Base (ElementType x__, ElementType y__) : x (x__), y (y__) { }
  Vec2Base (ElementType* coords__) { fromArray (coords__); }
};

template <typename ElementT>
struct Vec3Base : public LinearVectorOperations <Vec3Base <ElementT>, ElementT>,
                  public VectorConversations    <Vec3Base <ElementT>, ElementT> {
  static const int DIMENSION = 3;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z;
    };
    ElementType coords[DIMENSION];
  };

  Vec3Base() : x (0), y (0), z (0) { }
  Vec3Base (ElementType x__, ElementType y__, ElementType z__) : x (x__), y (y__), z (z__) { }
  Vec3Base (ElementType* coords__) { fromArray (coords__); }
};

template <typename ElementT>
struct Vec4Base : public LinearVectorOperations <Vec4Base <ElementT>, ElementT>,
                  public VectorConversations    <Vec4Base <ElementT>, ElementT> {
  static const int DIMENSION = 4;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z, w;
    };
    ElementType coords[DIMENSION];
  };

  Vec4Base() : x (0), y (0), z (0), w (0) { }
  Vec4Base (ElementType x__, ElementType y__, ElementType z__, ElementType w__) : x (x__), y (y__), z (z__), w (w__) { }
  Vec4Base (ElementType* coords__) { fromArray (coords__); }
};



template <typename VectorT>
typename VectorT::ElementType scalarProduct (VectorT a, VectorT b) {
  typename VectorT::ElementType sum = 0;
  for (int i = 0; i < VectorT::DIMENSION; ++i)
    sum += a.coords[i] * b.coords[i];
  return sum;
}

// template <typename ElementT>
// Vec2Base <ElementT> crossProduct (Vec2Base <ElementT> a) {
//   return Vec2Base <ElementT> (-a.y, a.x);
// }

template <typename ElementT>
Vec3Base <ElementT> crossProduct (Vec3Base <ElementT> a, Vec3Base <ElementT> b) {
  return Vec3Base <ElementT> (a.y*b.z - a.z*b.y,  a.z*b.x - a.x*b.z,  a.x*b.y - a.y*b.x);
}

template <typename VectorT>
typename VectorT::ElementType euclideanNormSqr (VectorT a) {
  return scalarProduct (a, a);
}

template <typename VectorT>
typename VectorT::ElementType euclideanNorm (VectorT a) {
  return std::sqrt (euclideanNorm (a));
}

template <typename VectorT>
typename VectorT::ElementType euclideanDistanceSqr (VectorT a, VectorT b) {
//   return euclideanNormSqr (a - b);
  typename VectorT::ElementType sum = 0;
  for (int i = 0; i < VectorT::DIMENSION; ++i)
    sum += xSqr (a.coords[i] - b.coords[i]);
  return sum;
}

template <typename VectorT>
typename VectorT::ElementType euclideanDistance (VectorT a, VectorT b) {
  return std::sqrt (euclideanDistance (a, b));
}


template <typename VectorT, typename ElementT>
struct LexicographicCompareVectors {
  static_assert (std::numeric_limits <ElementT>::is_integer, "Using floating-point values as keys for a set or a map is almost certainly a bad idea.");
  bool operator() (VectorT a, VectorT b) {
    for (int i = 0; i < VectorT::DIMENSION; ++i) {
      if (a.coords[i] < b.coords[i])
        return true;
      else if (a.coords[i] > b.coords[i])
        return false;
    }
    return false;
  }
};


typedef Vec2Base <int>    Vec2i;
typedef Vec3Base <int>    Vec3i;
typedef Vec4Base <int>    Vec4i;

typedef Vec2Base <float>  Vec2f;
typedef Vec3Base <float>  Vec3f;
typedef Vec4Base <float>  Vec4f;

typedef Vec2Base <double> Vec2d;
typedef Vec3Base <double> Vec3d;
typedef Vec4Base <double> Vec4d;


struct LexicographicCompareVec2i : LexicographicCompareVectors <Vec2i, int> { };
struct LexicographicCompareVec3i : LexicographicCompareVectors <Vec3i, int> { };
struct LexicographicCompareVec4i : LexicographicCompareVectors <Vec4i, int> { };


#endif
