#ifndef LINEAR_ALGEBRA_HPP
#define LINEAR_ALGEBRA_HPP


// TODO: check disassemble for variants ``foo (Vec2d)'' and ``foo (const Vec2d&)''
//      (and better use the second one: the underlying class may potentially be quite complex)


#include <cstddef>
#include <cmath>
#include <limits>

#include "common/math_utils.hpp"



template <int DIMENSION, typename ElementT>
class Vector {
public:
  Vector () = delete;
};



#define DECLARE_DERIVED(VECTOR_DIMENSION__, ElementT__) \
  private: \
    const Vector <VECTOR_DIMENSION__, ElementT__>& derived () const    { return static_cast <const Vector <VECTOR_DIMENSION__, ElementT__>&> (*this); } \
    Vector <VECTOR_DIMENSION__, ElementT__>& derived ()                { return static_cast <Vector <VECTOR_DIMENSION__, ElementT__>&> (*this); }



template <int DIMENSION, typename ElementT>
class VectorIndexingOperations {
public:
  ElementT operator[] (size_t index) const {
    return derived ().coords [index];
  }

  ElementT& operator[] (size_t index) {
    return derived ().coords [index];
  }

  DECLARE_DERIVED (DIMENSION, ElementT)
};


template <int DIMENSION, typename ElementT>
class CommonVectorLinearOperations {
private:
  typedef Vector <DIMENSION, ElementT> VectorType;

public:
  // in-place operators

  VectorType& operator+= (VectorType a) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] += a.coords[i];
    return derived ();
  }

  VectorType& operator-= (VectorType a) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] -= a.coords[i];
    return derived ();
  }

  VectorType& operator*= (ElementT q) {
    for (int i = 0; i <VectorType:: DIMENSION; ++i)
      derived ().coords[i] *= q;
    return derived ();
  }

  VectorType& operator/= (ElementT q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] /= q;
    return derived ();
  }

  VectorType& operator%= (ElementT q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] %= q;
    return derived ();
  }


  // unary operators

  VectorType operator- () const {
    return VectorType::zeroVector () - derived ();
  }


  // binary operators

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

  VectorType operator* (ElementT q) const {
    VectorType result (derived ());
    result *= q;
    return result;
  }

  VectorType operator/ (ElementT q) const {
    VectorType result (derived ());
    result /= q;
    return result;
  }

  VectorType operator% (ElementT q) const {
    VectorType result (derived ());
    result %= q;
    return result;
  }

  DECLARE_DERIVED (DIMENSION, ElementT)
};

template <int DIMENSION, typename ElementT>
class VectorLinearOperations : public CommonVectorLinearOperations <DIMENSION, ElementT> { };

template <int DIMENSION>
class VectorLinearOperations <DIMENSION, int> : public CommonVectorLinearOperations <DIMENSION, int> {
private:
  typedef Vector <DIMENSION, int> VectorType;

public:
  // TODO: implement a combined divModFloored function

  // in-place operators

  VectorType& applyDivFloored (int q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] = intDivFloored (derived ().coords[i], q);
    return derived ();
  }

  VectorType& applyModFloored (int q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] = intModFloored (derived ().coords[i], q);
    return derived ();
  }


  // binary operators

  VectorType divFloored (int q) const {
    VectorType result (derived ());
    result.applyDivFloored (q);
    return result;
  }

  VectorType modFloored (int q) const {
    VectorType result (derived ());
    result.applyModFloored (q);
    return result;
  }

  DECLARE_DERIVED (DIMENSION, int)
};


template <int DIMENSION, typename ElementT>
class VectorConversations {
private:
  typedef Vector <DIMENSION, ElementT> VectorType;

public:
  void copyFromArray (const ElementT* coords__) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] = coords__[i];
  }

  template <typename OtherElementT>
  void copyFromArrayConverted (const OtherElementT* coords__) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().coords[i] = coords__[i];
  }

  template <typename OtherVectorT>
  void copyFromVectorConverted (OtherVectorT source) {
    copyFromArrayConverted (source.coords);
  }

  void copyToArray (ElementT* coords__) const {
    for (int i = 0; i < DIMENSION; ++i)
      coords__[i] = derived ().coords[i];
  }


  template <typename OtherVectorT>
  static VectorType fromVectorConverted (OtherVectorT source) {
    VectorType result;
    result.copyFromVectorConverted (source);
    return result;
  }

  DECLARE_DERIVED (DIMENSION, ElementT)
};



template <typename ElementT>
class Vector <2, ElementT> : public VectorIndexingOperations <2, ElementT>,
                             public VectorLinearOperations   <2, ElementT>,
                             public VectorConversations      <2, ElementT> {
public:
  static const int DIMENSION = 2;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y;
    };
    ElementType coords[DIMENSION];
  };

  Vector () : x (0), y (0) { }
  Vector (ElementType x__, ElementType y__) : x (x__), y (y__) { }
  Vector (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__)  { x = x__;  y = y__; }

  static Vector replicatedValuesVector (ElementType value)  { return Vector (value, value); }     // TODO: rename (?)
  static Vector zeroVector ()                               { return Vector (0, 0); }
};

template <typename ElementT>
class Vector <3, ElementT> : public VectorIndexingOperations <3, ElementT>,
                             public VectorLinearOperations   <3, ElementT>,
                             public VectorConversations      <3, ElementT> {
public:
  static const int DIMENSION = 3;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z;
    };
    ElementType coords[DIMENSION];
  };

  Vector () : x (0), y (0), z (0) { }
  Vector (ElementType x__, ElementType y__, ElementType z__) : x (x__), y (y__), z (z__) { }
  Vector (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__, ElementType z__)  { x = x__;  y = y__;  z = z__; }

  static Vector replicatedValuesVector (ElementType value)  { return Vector (value, value, value); }     // TODO: rename (?)
  static Vector zeroVector ()                               { return Vector (0, 0, 0); }

  // TODO: generate other subsets and permutations
  Vector <2, ElementT> xy () const   { return Vector <2, ElementT> (x, y); }
};

template <typename ElementT>
class Vector <4, ElementT> : public VectorIndexingOperations <4, ElementT>,
                             public VectorLinearOperations   <4, ElementT>,
                             public VectorConversations      <4, ElementT> {
public:
  static const int DIMENSION = 4;
  typedef ElementT ElementType;

  union {
    struct {
      ElementType x, y, z, w;
    };
    ElementType coords[DIMENSION];
  };

  Vector () : x (0), y (0), z (0), w (0) { }
  Vector (ElementType x__, ElementType y__, ElementType z__, ElementType w__) : x (x__), y (y__), z (z__), w (w__) { }
  Vector (ElementType* coords__) { fromArray (coords__); }

  void setCoordinates (ElementType x__, ElementType y__, ElementType z__, ElementType w__)  { x = x__;  y = y__;  z = z__;  w = w__; }

  static Vector replicatedValuesVector (ElementType value)  { return Vector (value, value, value, value); }     // TODO: rename (?)
  static Vector zeroVector ()                               { return Vector (0, 0, 0, 0); }
};



// TODO: delete
#define XY_LIST(vec__)    (vec__).x, (vec__).y
#define XYZ_LIST(vec__)   (vec__).x, (vec__).y, (vec__).z
#define XYZW_LIST(vec__)  (vec__).x, (vec__).y, (vec__).z, (vec__).w



template <int DIMENSION, typename ElementT>
Vector <DIMENSION, ElementT> floor (Vector <DIMENSION, ElementT> a) {
  static_assert (!std::numeric_limits <ElementT>::is_integer, "Are you sure your want to apply floor function to an integer type?");
  for (int i = 0; i < DIMENSION; ++i)
    a.coords[i] = floor (a.coords[i]);
  return a;
}


template <int DIMENSION, typename ElementT>
ElementT scalarProduct (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  ElementT sum = 0;
  for (int i = 0; i < DIMENSION; ++i)
    sum += a.coords[i] * b.coords[i];
  return sum;
}

// template <int DIMENSION, typename ElementT>
// Vector <2, ElementT> crossProduct (Vector <2, ElementT> a) {
//   return Vector <2, ElementT> (-a.y, a.x);
// }

template <typename ElementT>
Vector <3, ElementT> crossProduct (Vector <3, ElementT> a, Vector <3, ElementT> b) {
  return Vector <3, ElementT> (a.y*b.z - a.z*b.y,  a.z*b.x - a.x*b.z,  a.x*b.y - a.y*b.x);
}


namespace L1 {
  template <int DIMENSION, typename ElementT>
  ElementT norm (Vector <DIMENSION, ElementT> a) {
    ElementT sum = 0;
    for (int i = 0; i < DIMENSION; ++i)
      sum += xAbs (a.coords[i]);
    return sum;
  }

  template <int DIMENSION, typename ElementT>
  ElementT distance (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
    ElementT sum = 0;
    for (int i = 0; i < DIMENSION; ++i)
      sum += xAbs (a.coords[i] - b.coords[i]);
    return sum;
  }

  template <int DIMENSION, typename ElementT>
  Vector <DIMENSION, ElementT> normalize (Vector <DIMENSION, ElementT> a) {
    return a / norm (a);
  }
}

namespace L2 {
  template <int DIMENSION, typename ElementT>
  ElementT normSqr (Vector <DIMENSION, ElementT> a) {
    return scalarProduct (a, a);
  }

  template <int DIMENSION, typename ElementT>
  ElementT norm (Vector <DIMENSION, ElementT> a) {
    return std::sqrt (normSqr (a));
  }

  template <int DIMENSION, typename ElementT>
  ElementT distanceSqr (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  //   return euclideanNormSqr (a - b);
    ElementT sum = 0;
    for (int i = 0; i < DIMENSION; ++i)
      sum += xSqr (a.coords[i] - b.coords[i]);
    return sum;
  }

  template <int DIMENSION, typename ElementT>
  ElementT distance (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
    return std::sqrt (distanceSqr (a, b));
  }

  template <int DIMENSION, typename ElementT>
  Vector <DIMENSION, ElementT> normalize (Vector <DIMENSION, ElementT> a) {
    return a / norm (a);
  }
}

namespace Linf {
  template <int DIMENSION, typename ElementT>
  ElementT norm (Vector <DIMENSION, ElementT> a) {
    ElementT max = 0;
    for (int i = 0; i < DIMENSION; ++i)
      max = xMax (max, xAbs (a.coords[i]));
    return max;
  }

  template <int DIMENSION, typename ElementT>
  ElementT distance (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
    ElementT max = 0;
    for (int i = 0; i < DIMENSION; ++i)
      max = xMax (max, xAbs (a.coords[i] - b.coords[i]));
    return max;
  }

  template <int DIMENSION, typename ElementT>
  Vector <DIMENSION, ElementT> normalize (Vector <DIMENSION, ElementT> a) {
    return a / norm (a);
  }
}



template <int DIMENSION, typename ElementT>
struct LexicographicCompareVectors {
  static_assert (std::numeric_limits <ElementT>::is_integer, "Using floating-point values as keys for a set or a map is almost certainly a bad idea.");
  bool operator() (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) const {
    for (int i = 0; i < DIMENSION; ++i) {
      if (a.coords[i] < b.coords[i])
        return true;
      else if (a.coords[i] > b.coords[i])
        return false;
    }
    return false;
  }
};



typedef Vector <2, int>     Vec2i;
typedef Vector <3, int>     Vec3i;
typedef Vector <4, int>     Vec4i;

typedef Vector <2, float>   Vec2f;
typedef Vector <3, float>   Vec3f;
typedef Vector <4, float>   Vec4f;

typedef Vector <2, double>  Vec2d;
typedef Vector <3, double>  Vec3d;
typedef Vector <4, double>  Vec4d;


struct LexicographicCompareVec2i : LexicographicCompareVectors <2, int> { };
struct LexicographicCompareVec3i : LexicographicCompareVectors <3, int> { };
struct LexicographicCompareVec4i : LexicographicCompareVectors <4, int> { };


#endif
