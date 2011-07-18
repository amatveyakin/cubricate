#ifndef SIMPLE_LIGHT_MAP_HPP
#define SIMPLE_LIGHT_MAP_HPP


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include "common/linear_algebra.hpp"
#include "common/multidim_array.hpp"

typedef Vec4f SHCoefficients;


class SimpleLightMap {
public:
  SimpleLightMap (int sizeX, int sizeY, int sizeZ);
  ~SimpleLightMap();

  void clear();

  void loadSubLightMapToTexture (GLuint texture, Vec3i firstCorner, Vec3i secondCorner);
  void loadSubLightMapToTexture (GLuint texture, Vec3i modifiedCube);
  void calculateLight (Vec3i firstCorner, Vec3i secondCorner, float multiplier);
  void calculateLight (Vec3i modifiedCube, float multiplier);
  void lightThatCubePlease (Vec3i cube);
  void  addStillLight  (Vec3i position, SHCoefficients lightSH, float multiplier);
  //void  addLightSource (Vec3i position, SHCoefficients lightSH, float multiplier);
  //void recalculateLight (Vec3i changedCube);

  void calculateSunlight (Vec3i changedCube, float multiplier);

  void loadVisibilityMapToTexture (GLuint texture);


protected:
  //Vec4f consists of 4 SH coefficients
  Array3D <SHCoefficients>  m_luminosity;
  Array3D <SHCoefficients>  m_sunVisibility;
  int                       m_nRays;
  Vec3d*                    m_rays;

  void generateRandomRays (int nRays);

  SHCoefficients cosineLobeSH    (Vec3f direction);
  SHCoefficients deltaFunctionSH (Vec3f direction);

};


#endif
