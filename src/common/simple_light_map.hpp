#ifndef SIMPLE_LIGHT_MAP_HPP
#define SIMPLE_LIGHT_MAP_HPP


#include "common/linear_algebra.hpp"
#include "common/multidim_array.hpp"
#include "client/client_world.hpp"
typedef Vec4f SHCoefficients;

class SimpleLightMap {
public:
  SimpleLightMap (int sizeX, int sizeY, int sizeZ);
  ~SimpleLightMap();

  void calculateLight (Vec3i firstCorner, Vec3i secondCorner, float multiplier);
  //void recalculateLight (Vec3i changedCube);

protected:
  //Vec4f consists of 4 SH coefficients
  Array3D <SHCoefficients>  m_luminosity;

  SHCoefficients cosineLobeSH    (Vec3f direction);
  SHCoefficients deltaFunctionSH (Vec3f direction);

};

#endif
