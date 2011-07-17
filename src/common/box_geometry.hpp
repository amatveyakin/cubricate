#ifndef BOX_GEOMETRY_HPP
#define BOX_GEOMETRY_HPP


#include "linear_algebra.hpp"


template <int DIMENSION, typename ElementT>
struct Box {
  typedef Vector <DIMENSION, ElementT> VectorType;

  Box() = default;
  Box (VectorType corner1Value, VectorType corner2Value) : corner1 (corner1Value), corner2 (corner2Value) { }

  // All corner1 coordinates must be not larger than respactive corner2 coordinates
  VectorType corner1, corner2;
};


template <int DIMENSION, typename ElementT>
Box <DIMENSION, ElementT> enclosingBox (Box <DIMENSION, ElementT> box1, Box <DIMENSION, ElementT> box2) {
  return Box <DIMENSION, ElementT> (xMin (box1.corner1, box2.corner1), xMax (box1.corner2, box2.corner2));
}

template <int DIMENSION, typename ElementT>
Box <DIMENSION, ElementT> intersectBox (Box <DIMENSION, ElementT> box1, Box <DIMENSION, ElementT> box2) {
  return Box <DIMENSION, ElementT> (xMax (box1.corner1, box2.corner1), xMin (box1.corner2, box2.corner2));
}


typedef Box <3, int> Box3i;


#endif
