#ifndef WATER_ENGINE_CPP
#define WATER_ENGINE_CPP


#include <cassert>
#include <cmath>
#include <set>

#include "common/utils.hpp"
#include "common/linear_algebra.hpp"
#include "common/cube_geometry.hpp"


namespace WaterParams {
//   const float NORMAL_VERTICAL_SATURATION_RATIO = 1.05;

//   const float SPLITTING_THRESHOLD   = M_SQRT2   * sqrt (NORMAL_VERTICAL_SATURATION_RATIO);
//   const float MERGING_THRESHOLD     = M_SQRT2   / sqrt (NORMAL_VERTICAL_SATURATION_RATIO);

//   const float VERTICAL_SPLITTING_THRESHOLD   = M_SQRT2 * 1.1;
//   const float VERTICAL_MERGING_THRESHOLD     = M_SQRT2 / 1.1;

  const float MAX_SATURATION        = 1.;
  const float MIN_SATURATION        = 0.2;
}


class WaterEngine {
public:
  WaterEngine ();
  ~WaterEngine ();

  void addWaterCube (Vec3i pos) { FIX_UNUSED (pos); }
  void removeWaterCube (Vec3i pos) { FIX_UNUSED (pos); }

  void processWater ();

private:
  void processLowerNeighbour (Vec3i cube);
  void processUpperNeighbour (Vec3i cube);

  void processHorizontalNeighbours (Vec3i cube);
  void processVerticalWaterPair (Vec3i lowerCube, Vec3i upperCube);
//   void processHorizontalWaterPair (Vec3i firstCube, Vec3i secondCube);

  // Returns true if splitting succeeded
  bool tryToSplit (Vec3i cube, Direction dir);
};


#endif
