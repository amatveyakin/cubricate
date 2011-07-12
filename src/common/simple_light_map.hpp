#ifndef SIMPLE_LIGHT_MAP_HPP
#define SIMPLE_LIGHT_MAP_HPP


#include "common/linear_algebra.hpp"
#include "common/multidim_array.hpp"


class SimpleLightMap {
public:
  SimpleLightMap (int sizeX, int sizeY, int sizeZ);
  ~SimpleLightMap();

protected:
  // The first 3 components are total (r, b, g) values from various light sources.
  // The 4-th component is the sunlight intensity.
  Array3D <Vec4uc>  m_luminosity;
};


#endif
