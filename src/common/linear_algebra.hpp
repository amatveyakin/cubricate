#ifndef LINEAR_ALGEBRA_HPP
#define LINEAR_ALGEBRA_HPP


// TODO: check disassemble for variants ``foo (Vec2d)'' and ``foo (const Vec2d&)''
//      (and better use the second one: the underlying class may potentially be quite complex)


#include <cstddef>
#include <cmath>
#include <limits>

#include "common/math_utils.hpp"



#define DECLARE_DERIVED(Type__) \
  private: \
    const Type__& derived () const    { return static_cast <const Type__&> (*this); } \
    Type__& derived ()                { return static_cast <Type__&> (*this); }



template <typename VectorT, typename ElementT>
class VectorIndexingOperations {
public:
  ElementT operator[] (size_t index) const {
    return derived ().coords [index];
  }

  ElementT& operator[] (size_t index) {
    return derived ().coords [index];
  }

  DECLARE_DERIVED (VectorT)
};


template <typename VectorT, typename ElementT>
class CommonVectorLinearOperations {
public:
  // in-place operators

  VectorT& operator+= (VectorT a) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] += a.coords[i];
    return derived ();
  }

  VectorT& operator-= (VectorT a) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] -= a.coords[i];
    return derived ();
  }

  VectorT& operator*= (ElementT q) {
    for (int i = 0; i <VectorT:: DIMENSION; ++i)
      derived ().coords[i] *= q;
    return derived ();
  }

  VectorT& operator/= (ElementT q) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] /= q;
    return derived ();
  }

  VectorT& operator%= (ElementT q) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] %= q;
    return derived ();
  }


  // unary operators

  VectorT operator- () const {
    return VectorT::zeroVector () - derived ();
  }


  // binary operators

  VectorT operator+ (VectorT a) const {
    VectorT result (derived ());
    result += a;
    return result;
  }

  VectorT operator- (VectorT a) const {
    VectorT result (derived ());
    result -= a;
    return result;
  }

  VectorT operator* (ElementT q) const {
    VectorT result (derived ());
    result *= q;
    return result;
  }

  VectorT operator/ (ElementT q) const {
    VectorT result (derived ());
    result /= q;
    return result;
  }

  VectorT operator% (ElementT q) const {
    VectorT result (derived ());
    result %= q;
    return result;
  }

  DECLARE_DERIVED (VectorT)
};

template <typename VectorT, typename ElementT>
class VectorLinearOperations : public CommonVectorLinearOperations <VectorT, ElementT> { };

template <typename VectorT>
class VectorLinearOperations <VectorT, int> : public CommonVectorLinearOperations <VectorT, int> {
public:
  // TODO: implement a combined divModFloored function

  // in-place operators

  VectorT& applyDivFloored (int q) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] = intDivFloored (derived ().coords[i], q);
    return derived ();
  }

  VectorT& applyModFloored (int q) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] = intModFloored (derived ().coords[i], q);
    return derived ();
  }


  // binary operators

  VectorT divFloored (int q) const {
    VectorT result (derived ());
    result.applyDivFloored (q);
    return result;
  }

  VectorT modFloored (int q) const {
    VectorT result (derived ());
    result.applyModFloored (q);
    return result;
  }

  DECLARE_DERIVED (VectorT)
};


template <typename VectorT, typename ElementT>
class VectorConversations {
public:
  void copyFromArray (const ElementT* coords__) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] = coords__[i];
  }

  template <typename OtherElementT>
  void copyFromArrayConverted (const OtherElementT* coords__) {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      derived ().coords[i] = coords__[i];
  }

  template <typename OtherVectorT>
  void copyFromVectorConverted (OtherVectorT source) {
    copyFromArrayConverted (source.coords);
  }

  void copyToArray (ElementT* coords__) const {
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      coords__[i] = derived ().coords[i];
  }


  template <typename OtherVectorT>
  static VectorT fromVectorConverted (OtherVectorT source) {
    VectorT result;
    result.copyFromVectorConverted (source);
    return result;
  }

  DECLARE_DERIVED (VectorT)
};



template <typename ElementT>
struct Vec2Base : public VectorIndexingOperations <Vec2Base <ElementT>, ElementT>,
                  public VectorLinearOperations   <Vec2Base <ElementT>, ElementT>,
                  public VectorConversations      <Vec2Base <ElementT>, ElementT> {
  static const int DIMENSION = 2;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y;
    };
    ElementType coords[DIMENSION];
  };

  Vec2Base () : x (0), y (0) { }
  Vec2Base (ElementType x__, ElementType y__) : x (x__), y (y__) { }
  Vec2Base (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__)  { x = x__;  y = y__; }

  static Vec2Base replicatedValuesVector (ElementType value)  { return Vec2Base (value, value); }     // TODO: rename (?)
  static Vec2Base zeroVector ()                               { return Vec2Base (0, 0); }
};

template <typename ElementT>
struct Vec3Base : public VectorIndexingOperations <Vec3Base <ElementT>, ElementT>,
                  public VectorLinearOperations   <Vec3Base <ElementT>, ElementT>,
                  public VectorConversations      <Vec3Base <ElementT>, ElementT> {
  static const int DIMENSION = 3;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z;
    };
    ElementType coords[DIMENSION];
  };

  Vec3Base () : x (0), y (0), z (0) { }
  Vec3Base (ElementType x__, ElementType y__, ElementType z__) : x (x__), y (y__), z (z__) { }
  Vec3Base (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__, ElementType z__)  { x = x__;  y = y__;  z = z__; }

  static Vec3Base replicatedValuesVector (ElementType value)  { return Vec3Base (value, value, value); }     // TODO: rename (?)
  static Vec3Base zeroVector ()                               { return Vec3Base (0, 0, 0); }

  // TODO: generate other subsets and permutations
  Vec2Base <ElementT> xy () const   { return Vec2Base <ElementT> (x, y); }
};

template <typename ElementT>
struct Vec4Base : public VectorIndexingOperations <Vec4Base <ElementT>, ElementT>,
                  public VectorLinearOperations   <Vec4Base <ElementT>, ElementT>,
                  public VectorConversations      <Vec4Base <ElementT>, ElementT> {
  static const int DIMENSION = 4;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z, w;
    };
    ElementType coords[DIMENSION];
  };

  Vec4Base () : x (0), y (0), z (0), w (0) { }
  Vec4Base (ElementType x__, ElementType y__, ElementType z__, ElementType w__) : x (x__), y (y__), z (z__), w (w__) { }
  Vec4Base (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__, ElementType z__, ElementType w__)  { x = x__;  y = y__;  z = z__;  w = w__; }

  static Vec4Base replicatedValuesVector (ElementType value)  { return Vec4Base (value, value, value, value); }     // TODO: rename (?)
  static Vec4Base zeroVector ()                               { return Vec4Base (0, 0, 0, 0); }

  Vec3Base <ElementT> xyz () const  { return Vec3Base <ElementT> (x, y, z); }
};



// TODO: delete
#define XY_LIST(vec__)    (vec__).x, (vec__).y
#define XYZ_LIST(vec__)   (vec__).x, (vec__).y, (vec__).z
#define XYZW_LIST(vec__)  (vec__).x, (vec__).y, (vec__).z, (vec__).w



template <typename VectorT>
VectorT floor (VectorT a) {
  static_assert (!std::numeric_limits <typename VectorT::ElementType>::is_integer, "Are you sure your want to apply floor function to an integer type?");
  for (int i = 0; i < VectorT::DIMENSION; ++i)
    a.coords[i] = floor (a.coords[i]);
  return a;
}


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

/*
template <typename VectorT>
typename VectorT::ElementType euclideanNormSqr (VectorT a) {
  return scalarProduct (a, a);
}

template <typename VectorT>
typename VectorT::ElementType euclideanNorm (VectorT a) {
  return std::sqrt (euclideanNormSqr (a));
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
  return std::sqrt (euclideanDistanceSqr (a, b));
}

template <typename VectorT>
VectorT euclideanNormalize (VectorT a) {    // TODO: rename (?)
  return a / euclideanNorm (a);
}
*/

namespace L1 {
  template <typename VectorT>
  typename VectorT::ElementType norm (VectorT a) {
    typename VectorT::ElementType sum = 0;
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      sum += xAbs (a.coords[i]);
    return sum;
  }

  template <typename VectorT>
  typename VectorT::ElementType distance (VectorT a, VectorT b) {
    typename VectorT::ElementType sum = 0;
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      sum += xAbs (a.coords[i] - b.coords[i]);
    return sum;
  }

  template <typename VectorT>
  VectorT normalize (VectorT a) {
    return a / norm (a);
  }
}

namespace L2 {
  template <typename VectorT>
  typename VectorT::ElementType normSqr (VectorT a) {
    return scalarProduct (a, a);
  }

  template <typename VectorT>
  typename VectorT::ElementType norm (VectorT a) {
    return std::sqrt (normSqr (a));
  }

  template <typename VectorT>
  typename VectorT::ElementType distanceSqr (VectorT a, VectorT b) {
  //   return euclideanNormSqr (a - b);
    typename VectorT::ElementType sum = 0;
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      sum += xSqr (a.coords[i] - b.coords[i]);
    return sum;
  }

  template <typename VectorT>
  typename VectorT::ElementType distance (VectorT a, VectorT b) {
    return std::sqrt (distanceSqr (a, b));
  }

  template <typename VectorT>
  VectorT normalize (VectorT a) {
    return a / norm (a);
  }
}

namespace Linf {
  template <typename VectorT>
  typename VectorT::ElementType norm (VectorT a) {
    typename VectorT::ElementType max = 0;
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      max = xMax (max, xAbs (a.coords[i]));
    return max;
  }

  template <typename VectorT>
  typename VectorT::ElementType distance (VectorT a, VectorT b) {
    typename VectorT::ElementType max = 0;
    for (int i = 0; i < VectorT::DIMENSION; ++i)
      max = xMax (max, xAbs (a.coords[i] - b.coords[i]));
    return max;
  }

  template <typename VectorT>
  VectorT normalize (VectorT a) {
    return a / norm (a);
  }
}



template <typename VectorT, typename ElementT>
struct LexicographicCompareVectors {
  static_assert (std::numeric_limits <ElementT>::is_integer, "Using floating-point values as keys for a set or a map is almost certainly a bad idea.");
  bool operator() (VectorT a, VectorT b) const {
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
