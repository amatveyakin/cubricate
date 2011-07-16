#ifndef LINEAR_ALGEBRA_HPP
#define LINEAR_ALGEBRA_HPP


// TODO: check disassemble for variants ``foo (Vec2d)'' and ``foo (const Vec2d&)''
//      (and better use the second one: the underlying class may potentially be quite complex)


#include <cstddef>
#include <cmath>
#include <limits>
#include <algorithm>
#include <ostream>

#include "common/math_utils.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vectors



// TODO: delete
#define DECLARE_DERIVED(Type__) \
  private: \
    const Type__& derived () const    { return static_cast <const Type__&> (*this); } \
    Type__& derived ()                { return static_cast <Type__&> (*this); }



template <int DIMENSION, typename ElementT>
class VectorBase {
public:
  ElementT at (size_t index) const          { return m_elements [index]; }
  ElementT& at (size_t index)               { return m_elements [index]; }
  ElementT operator[] (size_t index) const  { return m_elements [index]; }
  ElementT& operator[] (size_t index)       { return m_elements [index]; }

  const ElementT* data () const             { return m_elements; }
  ElementT* data ()                         { return m_elements; }

  bool operator== (const VectorBase& a) {
    static_assert (std::numeric_limits <ElementT>::is_integer, "Comparing floating point vectors with operator== is strongly discouraged!");
    for (int i = 0; i < DIMENSION; ++i)
      if (m_elements[i] != a.m_elements[i])
        return false;
    return true;
  }

protected:
  ElementT m_elements [DIMENSION];

  template <int, typename, typename>
  friend class CommonVectorLinearOperations;
};


template <int DIMENSION, typename VectorT, typename ElementT>
class CommonVectorLinearOperations {
public:
  // in-place operators

  VectorT& operator+= (VectorT a) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().m_elements[i] += a.m_elements[i];
    return derived ();
  }

  VectorT& operator-= (VectorT a) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().m_elements[i] -= a.m_elements[i];
    return derived ();
  }

  VectorT& operator*= (ElementT q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().m_elements[i] *= q;
    return derived ();
  }

  VectorT& operator/= (ElementT q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().m_elements[i] /= q;
    return derived ();
  }

  VectorT& operator%= (ElementT q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ().m_elements[i] %= q;
    return derived ();
  }


  // unary operators

  VectorT operator- () const {
    return VectorT::zero () - derived ();
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

  friend VectorT operator* (ElementT q, VectorT a) {
    return a * q;
  }

  DECLARE_DERIVED (VectorT)
};

template <int DIMENSION, typename VectorT, typename ElementT>
class VectorLinearOperations : public CommonVectorLinearOperations <DIMENSION, VectorT, ElementT> { };

template <int DIMENSION, typename VectorT>
class VectorLinearOperations <DIMENSION, VectorT, int> : public CommonVectorLinearOperations <DIMENSION, VectorT, int> {
public:
  // TODO: implement a combined divModFloored function

  // in-place operators

  VectorT& applyDivFloored (int q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ()[i] = ::divFloored (derived ()[i], q);
    return derived ();
  }

  VectorT& applyModFloored (int q) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ()[i] = ::modFloored (derived ()[i], q);
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


template <int DIMENSION, typename VectorT, typename ElementT>
class VectorConversations {
public:
  void copyFromArray (const ElementT* elements) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ()[i] = elements[i];
  }

  template <typename OtherElementT>
  void copyFromArrayConverted (const OtherElementT* elements) {
    for (int i = 0; i < DIMENSION; ++i)
      derived ()[i] = elements[i];
  }

  template <typename OtherVectorT>
  void copyFromVectorConverted (OtherVectorT source) {
    copyFromArrayConverted (source.data ());
  }

  void copyToArray (ElementT* elements) const {
    for (int i = 0; i < DIMENSION; ++i)
      elements[i] = derived ()[i];
  }


  template <typename OtherVectorT>
  static VectorT fromVectorConverted (OtherVectorT source) {
    VectorT result;
    result.copyFromVectorConverted (source);
    return result;
  }

  DECLARE_DERIVED (VectorT)
};





template <int VECTOR_DIMENSION, typename ElementT>
class Vector {
  Vector () = delete;
};


template <typename ElementT>
class Vector <2, ElementT> : public VectorBase              <2, ElementT>,
                             public VectorLinearOperations  <2, Vector <2, ElementT>, ElementT>,
                             public VectorConversations     <2, Vector <2, ElementT>, ElementT>
{
private:
  typedef VectorBase <2, ElementT> Parent;

public:
  Vector ()                                 { }
  Vector (ElementT x__, ElementT y__)       { setCoordinates (x__, y__); }
  Vector (ElementT* coords__)               { fromArray (coords__); }

  ElementT x () const                       { return Parent::at (0); }
  ElementT& x ()                            { return Parent::at (0); }
  ElementT y () const                       { return Parent::at (1); }
  ElementT& y ()                            { return Parent::at (1); }

  void setCoordinates (ElementT x__, ElementT y__)  { x () = x__;  y () = y__; }

  static Vector zero ()                     { return Vector (0, 0); }
  static Vector replicated (ElementT value) { return Vector (value, value); }
  static Vector e1 ()                       { return Vector (1, 0); }
  static Vector e2 ()                       { return Vector (0, 1); }
  static Vector e_i (int i)                 { Vector result = Vector::zero ();  result[i] = 1;  return result; }
};


template <typename ElementT>
class Vector <3, ElementT> : public VectorBase              <3, ElementT>,
                             public VectorLinearOperations  <3, Vector <3, ElementT>, ElementT>,
                             public VectorConversations     <3, Vector <3, ElementT>, ElementT>
{
private:
  typedef VectorBase <3, ElementT> Parent;

public:
  Vector ()                                           { }
  Vector (ElementT x__, ElementT y__, ElementT z__)   { setCoordinates (x__, y__, z__); }
  Vector (ElementT* coords__)                         { fromArray (coords__); }

  ElementT x () const                       { return Parent::at (0); }
  ElementT& x ()                            { return Parent::at (0); }
  ElementT y () const                       { return Parent::at (1); }
  ElementT& y ()                            { return Parent::at (1); }
  ElementT z () const                       { return Parent::at (2); }
  ElementT& z ()                            { return Parent::at (2); }

  ElementT r () const                       { return Parent::at (0); }
  ElementT& r ()                            { return Parent::at (0); }
  ElementT g () const                       { return Parent::at (1); }
  ElementT& g ()                            { return Parent::at (1); }
  ElementT b () const                       { return Parent::at (2); }
  ElementT& b ()                            { return Parent::at (2); }

  void setCoordinates (ElementT x__, ElementT y__, ElementT z__)  { x () = x__;  y () = y__;  z () = z__; }

  // TODO: generate other subsets and permutations
  Vector <2, ElementT> xy () const          { return Vector <2, ElementT> (x (), y ()); }

  static Vector zero ()                     { return Vector (0, 0, 0); }
  static Vector replicated (ElementT value) { return Vector (value, value, value); }
  static Vector e1 ()                       { return Vector (1, 0, 0); }
  static Vector e2 ()                       { return Vector (0, 1, 0); }
  static Vector e3 ()                       { return Vector (0, 0, 1); }
  static Vector e_i (int i)                 { Vector result = Vector::zero ();  result[i] = 1;  return result; }
};


template <typename ElementT>
class Vector <4, ElementT> : public VectorBase              <4, ElementT>,
                             public VectorLinearOperations  <4, Vector <4, ElementT>, ElementT>,
                             public VectorConversations     <4, Vector <4, ElementT>, ElementT>
{
private:
  typedef VectorBase <4, ElementT> Parent;

public:
  Vector ()                                                         { }
  Vector (ElementT x__, ElementT y__, ElementT z__, ElementT w__)   { setCoordinates (x__, y__, z__, w__); }
  Vector (ElementT* coords__)                                       { fromArray (coords__); }

  ElementT x () const                       { return Parent::at (0); }
  ElementT& x ()                            { return Parent::at (0); }
  ElementT y () const                       { return Parent::at (1); }
  ElementT& y ()                            { return Parent::at (1); }
  ElementT z () const                       { return Parent::at (2); }
  ElementT& z ()                            { return Parent::at (2); }
  ElementT w () const                       { return Parent::at (3); }
  ElementT& w ()                            { return Parent::at (3); }

  ElementT r () const                       { return Parent::at (0); }
  ElementT& r ()                            { return Parent::at (0); }
  ElementT g () const                       { return Parent::at (1); }
  ElementT& g ()                            { return Parent::at (1); }
  ElementT b () const                       { return Parent::at (2); }
  ElementT& b ()                            { return Parent::at (2); }
  ElementT a () const                       { return Parent::at (3); }
  ElementT& a ()                            { return Parent::at (3); }

  void setCoordinates (ElementT x__, ElementT y__, ElementT z__, ElementT w__)  { x () = x__;  y () = y__;  z () = z__;  w () = w__; }

  static Vector zero ()                     { return Vector (0, 0, 0, 0); }
  static Vector replicated (ElementT value) { return Vector (value, value, value, value); }
  static Vector e1 ()                       { return Vector (1, 0, 0, 0); }
  static Vector e2 ()                       { return Vector (0, 1, 0, 0); }
  static Vector e3 ()                       { return Vector (0, 0, 1, 0); }
  static Vector e4 ()                       { return Vector (0, 0, 0, 1); }
  static Vector e_i (int i)                 { Vector result = Vector::zero ();  result[i] = 1;  return result; }
};





// TODO: delete
#define XY_LIST(vec__)    (vec__).x (), (vec__).y ()
#define XYZ_LIST(vec__)   (vec__).x (), (vec__).y (), (vec__).z ()
#define XYZW_LIST(vec__)  (vec__).x (), (vec__).y (), (vec__).z (), (vec__).w ()



template <int DIMENSION, typename ElementT>
Vector <DIMENSION, ElementT> xMin (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  Vector <DIMENSION, ElementT> result;
  for (int i = 0; i < DIMENSION; ++i)
    result[i] = xMin (a[i], b[i]);
  return result;
}

template <int DIMENSION, typename ElementT>
Vector <DIMENSION, ElementT> xMax (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  Vector <DIMENSION, ElementT> result;
  for (int i = 0; i < DIMENSION; ++i)
    result[i] = xMax (a[i], b[i]);
  return result;
}


template <int DIMENSION, typename ElementT>
Vector <DIMENSION, ElementT> floor (Vector <DIMENSION, ElementT> a) {
  static_assert (!std::numeric_limits <ElementT>::is_integer,
                 "Are you sure your want to apply floor function to an integer type?");
  Vector <DIMENSION, ElementT> result;
  for (int i = 0; i < DIMENSION; ++i)
    result[i] = floor (a[i]);
  return result;
}


template <int DIMENSION, typename ElementT>
ElementT dotProduct (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  ElementT sum = 0;
  for (int i = 0; i < DIMENSION; ++i)
    sum += a[i] * b[i];
  return sum;
}

template <int DIMENSION, typename ElementT>
ElementT positiveDotProduct (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
  return xMax (dotProduct (a, b), ElementT (0));
}
// template <int DIMENSION, typename ElementT>
// Vector <2, ElementT> crossProduct (Vector <2, ElementT> a) {
//   return Vector <2, ElementT> (-a.y, a.x);
// }

template <typename ElementT>
Vector <3, ElementT> crossProduct (Vector <3, ElementT> a, Vector <3, ElementT> b) {
  return Vector <3, ElementT> (a.y() * b.z() - a.z() * b.y(),  a.z() * b.x() - a.x() * b.z(),  a.x() * b.y() - a.y() * b.x());
}


namespace L1 {
  template <int DIMENSION, typename ElementT>
  ElementT norm (Vector <DIMENSION, ElementT> a) {
    ElementT sum = 0;
    for (int i = 0; i < DIMENSION; ++i)
      sum += xAbs (a[i]);
    return sum;
  }

  template <int DIMENSION, typename ElementT>
  ElementT distance (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
    ElementT sum = 0;
    for (int i = 0; i < DIMENSION; ++i)
      sum += xAbs (a[i] - b[i]);
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
    return dotProduct (a, a);
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
      sum += xSqr (a[i] - b[i]);
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
      max = xMax (max, xAbs (a[i]));
    return max;
  }

  template <int DIMENSION, typename ElementT>
  ElementT distance (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) {
    ElementT max = 0;
    for (int i = 0; i < DIMENSION; ++i)
      max = xMax (max, xAbs (a[i] - b[i]));
    return max;
  }

  template <int DIMENSION, typename ElementT>
  Vector <DIMENSION, ElementT> normalize (Vector <DIMENSION, ElementT> a) {
    return a / norm (a);
  }
}



template <int DIMENSION, typename ElementT>
std::ostream& operator<< (std::ostream& os, Vector <DIMENSION, ElementT> a) {
  os << "(" << a.x () << ", " << a.y () << ", " << a.z () << ")";
  return os;
}



template <int DIMENSION, typename ElementT>
struct LexicographicCompareVectors {
  static_assert (std::numeric_limits <ElementT>::is_integer,
                 "Using floating-point values as keys for a set or a map is almost certainly a bad idea.");
  bool operator() (Vector <DIMENSION, ElementT> a, Vector <DIMENSION, ElementT> b) const {
//     for (int i = 0; i < DIMENSION; ++i) {
    for (int i = DIMENSION - 1; i >= 0; --i) {
      if (a[i] < b[i])
        return true;
      else if (a[i] > b[i])
        return false;
    }
    return false;
  }
};



typedef Vector <2, unsigned char> Vec2uc;
typedef Vector <3, unsigned char> Vec3uc;
typedef Vector <4, unsigned char> Vec4uc;

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




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrices


// A column-major order matrix
template <int N_ROWS, int N_COLS, typename ElementT>
class MatrixBase {
private:
  static const int N_ELEMENTS = N_ROWS * N_COLS;

public:
  const ElementT* elements () const             { return m_elements; }
  ElementT* elements ()                         { return m_elements; }

  ElementT operator() (int row, int col) const  { return m_elements [col * N_ROWS + row]; }
  ElementT& operator() (int row, int col)       { return m_elements [col * N_ROWS + row]; }
  ElementT at (int row, int col) const          { return m_elements [col * N_ROWS + row]; }
  ElementT& at (int row, int col)               { return m_elements [col * N_ROWS + row]; }

protected:
  ElementT m_elements [N_ELEMENTS];

  template <int, typename, typename>
  friend class CommonVectorLinearOperations;
};



// Rectangle matrix
template <int N_ROWS, int N_COLS, typename ElementT>
class Matrix : public MatrixBase <N_ROWS, N_COLS, ElementT>,
               public VectorLinearOperations <N_ROWS * N_COLS, Matrix <N_ROWS, N_COLS, ElementT>, ElementT>
{
public:
  static Matrix zeroMatrix () {
    Matrix result;
    std::fill (result.m_elements, result.m_elements + N_ELEMENTS, 0);
    return result;
  }

private:
  static const int N_ELEMENTS = N_ROWS * N_COLS;
};

// Square matrix
template <int SIZE, typename ElementT>
class Matrix <SIZE, SIZE, ElementT> : public MatrixBase <SIZE, SIZE, ElementT>,
                                      public VectorLinearOperations <SIZE * SIZE, Matrix <SIZE, SIZE, ElementT>, ElementT>
{
public:
  static Matrix zeroMatrix () {
    Matrix result;
    std::fill (result.m_elements, result.m_elements + N_ELEMENTS, 0);
    return result;
  }

  static Matrix identityMatrix () {
    Matrix result = Matrix::zeroMatrix ();
    for (int i = 0; i < SIZE; ++i)
      result (i, i) = 1;
    return result;
  }

  static Matrix translationMatrix (Vector <SIZE - 1, ElementT> shift) {
    Matrix result = Matrix::identityMatrix ();
    for (int i = 0; i < SIZE - 1; ++i)
      result (i, SIZE - 1) = shift[i];
    return result;
  }

private:
  static const int N_ELEMENTS = SIZE * SIZE;
};


// TODO: rename (?)
template <int DIMENSION_IN, int DIMENSION_OUT, typename ElementT>
Vector <DIMENSION_OUT, ElementT> applyTransformation (Matrix <DIMENSION_OUT, DIMENSION_IN, ElementT> matrix,
                                                      Vector <DIMENSION_IN, ElementT> vector)
{
  Vector <DIMENSION_OUT, ElementT> result;
  for (int row = 0; row < DIMENSION_OUT; ++row) {
    ElementT sum = 0;
    for (int col = 0; col < DIMENSION_IN; ++col)
      sum += matrix (row, col) * vector (col);
    result [row] = sum;
  }
}

// TODO: rename (?)
template <int LHS_ROWS, int COMMON_SIZE, int RHS_COLS, typename ElementT>
Matrix <LHS_ROWS, RHS_COLS, ElementT> multMatrices (Matrix <LHS_ROWS, COMMON_SIZE, ElementT> lhs,
                                                    Matrix <COMMON_SIZE, RHS_COLS, ElementT> rhs)
{
  Matrix <LHS_ROWS, RHS_COLS, ElementT> result;
  for (int row = 0; row < LHS_ROWS; ++row) {
    for (int col = 0; col < RHS_COLS; ++col) {
      ElementT sum = 0;
      for (int i = 0; i < COMMON_SIZE; ++i)
        sum += lhs (row, i) * rhs (i, col);
      result (row, col) = sum;
    }
  }
}



// TODO: add more typedef's
typedef Matrix <3, 3, float>  Mat3x3f;
typedef Matrix <4, 4, float>  Mat4x4f;

typedef Matrix <3, 3, double> Mat3x3d;
typedef Matrix <4, 4, double> Mat4x4d;



#endif
