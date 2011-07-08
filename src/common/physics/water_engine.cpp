// TODO: delete
#include <iostream>


#include "common/physics/water_engine.hpp"

#include "client/client_world.hpp"


static inline WorldBlock newWaterCube (float saturation) {
  WorldBlock result;
  result.type = BT_WATER;
  result.fluidSaturation = saturation;
  return result;
}


WaterEngine::WaterEngine() {
  nextCubeIter = waterCubes.end ();
}

WaterEngine::~WaterEngine() { }


void WaterEngine::processWater() {
//   std::cout << "nWaterCubes = " << waterCubes.size () << std::endl;
  simpleWorldMap.lockRepaint ();
  for (auto curCubeIter = waterCubes.begin (); curCubeIter != waterCubes.end (); curCubeIter = nextCubeIter) {
    // WARNING: nextCubeIter may be changed from other member functions!
    nextCubeIter = curCubeIter;
    nextCubeIter++;
    Vec3i cube = *curCubeIter;

    if (cube.z () > 0)
      processLowerNeighbour (cube);

    //yep, thats must be fixed. or not.
    if (((cube.x () > 0) && (cube.x () < MAP_SIZE - 1)) &&
        ((cube.y () > 0) && (cube.y () < MAP_SIZE - 1)))
      processHorizontalNeighbours (cube);

    if (cube.z () < MAP_SIZE - 1)
      processUpperNeighbour (cube);
  }
  nextCubeIter = waterCubes.end ();
  simpleWorldMap.unlockRepaint ();
}


void WaterEngine::processLowerNeighbour (Vec3i cube) {
  Vec3i lowerCube = cube - Vec3i::e3 ();
  if (simpleWorldMap.get (lowerCube).type == BT_WATER)
    processVerticalWaterPair (lowerCube, cube);
  else if (simpleWorldMap.get (lowerCube).type == BT_AIR)
    simpleWorldMap.swapCubes (lowerCube, cube);
}

void WaterEngine::processUpperNeighbour (Vec3i cube) {
  Vec3i upperCube = cube + Vec3i::e3 ();
  if (simpleWorldMap.get (upperCube).type == BT_WATER)
    processVerticalWaterPair (cube, upperCube);
}

void WaterEngine::processHorizontalNeighbours (Vec3i cube) {
  const int N_NEIGHBOURS = 5;
  Vec3i neigbours[N_NEIGHBOURS];
  neigbours[0] = cube;
  neigbours[1] = cube + Vec3i::e1 ();
  neigbours[2] = cube - Vec3i::e1 ();
  neigbours[3] = cube + Vec3i::e2 ();
  neigbours[4] = cube - Vec3i::e2 ();

  int nValidNeigbours = 0;
  int nWaterNeigbours = 0;
  float totalSaturation = 0;
  for (int i = 0; i < N_NEIGHBOURS; ++i) {
    if (simpleWorldMap.get (neigbours[i]).type == BT_WATER) {
      nValidNeigbours++;
      nWaterNeigbours++;
      totalSaturation += simpleWorldMap.get (neigbours[i]).fluidSaturation;
    }
    if (simpleWorldMap.get (neigbours[i]).type == BT_AIR) {
      nValidNeigbours++;
    }
  }
  float meanSaturation = totalSaturation / nValidNeigbours;
  if (meanSaturation > WaterParams::MIN_SATURATION) {
    WorldBlock meanCube = newWaterCube (meanSaturation);
    for (int i = 0; i < N_NEIGHBOURS; ++i) {
      if (   (simpleWorldMap.get (neigbours[i]).type == BT_WATER)
          || (simpleWorldMap.get (neigbours[i]).type == BT_AIR))
        simpleWorldMap.set (neigbours[i], meanCube);
    }
  }
  else {
    int nFloodedAirNeighbours = 1;
    meanSaturation = totalSaturation / (nWaterNeigbours + nFloodedAirNeighbours);
    while (meanSaturation > WaterParams::MIN_SATURATION) {
      nFloodedAirNeighbours++;
      meanSaturation = totalSaturation / (nWaterNeigbours + nFloodedAirNeighbours);
    }
    nFloodedAirNeighbours--;
    WorldBlock meanCube = newWaterCube (meanSaturation);
    for (int i = 0; i < N_NEIGHBOURS; ++i) {
      if (simpleWorldMap.get (neigbours[i]).type == BT_WATER) {
        simpleWorldMap.set (neigbours[i], meanCube);
      }
      else if (   simpleWorldMap.get (neigbours[i]).type == BT_AIR
               && nFloodedAirNeighbours > 0) {
        simpleWorldMap.set (neigbours[i], meanCube);
        nFloodedAirNeighbours--;
      }
    }
  }
}


void WaterEngine::processVerticalWaterPair (Vec3i lowerCube, Vec3i upperCube) {
  float saturationSum = simpleWorldMap.get (lowerCube).fluidSaturation + simpleWorldMap.get (upperCube).fluidSaturation;
  if (saturationSum < WaterParams::MAX_SATURATION) {
    simpleWorldMap.set (lowerCube, newWaterCube (saturationSum));
    simpleWorldMap.set (upperCube, BT_AIR);
  }
  else {
    simpleWorldMap.set (lowerCube, newWaterCube (WaterParams::MAX_SATURATION));
    simpleWorldMap.set (upperCube, newWaterCube (saturationSum - WaterParams::MAX_SATURATION));
  }
}

// void WaterEngine::processHorizontalWaterPair (Vec3i firstCube, Vec3i secondCube) {
//   float saturationSum = simpleWorldMap.get (firstCube).fluidSaturation + simpleWorldMap.get (secondCube).fluidSaturation;
//   if (saturationSum < WaterParams::MERGING_THRESHOLD) {
//     simpleWorldMap.set (firstCube, newWaterCube (saturationSum));
//     simpleWorldMap.set (secondCube, BT_AIR);
//   }
//   else {
//     WorldBlock homogenizedCube = newWaterCube (saturationSum / 2.);
//     simpleWorldMap.set (firstCube, homogenizedCube);
//     simpleWorldMap.set (secondCube, homogenizedCube);
//   }
// }


// bool WaterEngine::tryToSplit (Vec3i splittingSource, Direction dir) {
//   Vec3i splittingDest = getAdjacentCube (splittingSource, dir);
//   if (cubeIsValid (splittingDest) && simpleWorldMap.get (splittingDest).type == BT_AIR) {
//     WorldBlock splittingResult = newWaterCube (simpleWorldMap.get (splittingSource).fluidSaturation / 2.);
//     simpleWorldMap.set (splittingSource, splittingResult);
//     simpleWorldMap.set (splittingDest, splittingResult);
//     return true;
//   }
//   return false;
// }
