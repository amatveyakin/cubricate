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

//     std::cout << cube << std::endl;
    if (cube.z () > 0)
      processLowerNeighbour (cube);


    //yep, thats must be fixed. or not.
    if (((cube.x () > 0) && (cube.x () < MAP_SIZE - 1)) &&
        ((cube.y () > 0) && (cube.y () < MAP_SIZE - 1)))
      processHorizontalNeighbours(cube);

    if (cube.z () < MAP_SIZE - 1)
      processUpperNeighbour (cube);

    // Step 2. Splitting
/*    if (simpleWorldMap.get (cube).fluidSaturation > WaterParams::SPLITTING_THRESHOLD) {
      if (tryToSplit (cube, Direction::Z_MINUS)) continue;
      if (tryToSplit (cube, Direction::X_MINUS)) continue;
      if (tryToSplit (cube, Direction::X_PLUS))  continue;
      if (tryToSplit (cube, Direction::Y_MINUS)) continue;
      if (tryToSplit (cube, Direction::Y_PLUS))  continue;
      if (tryToSplit (cube, Direction::Z_PLUS))  continue;
    }*/
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
  if (simpleWorldMap.get (cube).fluidSaturation > WaterParams::VERTICAL_SPLITTING_THRESHOLD) {
    WorldBlock emptyWaterCube = newWaterCube (0);
    simpleWorldMap.set (upperCube, emptyWaterCube);
    processVerticalWaterPair (cube, upperCube);
  }
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
      if ((simpleWorldMap.get (neigbours[i]).type == BT_WATER) ||
          (simpleWorldMap.get (neigbours[i]).type == BT_AIR)) {
        simpleWorldMap.set (neigbours[i], meanCube);
      }
    }
  }
  else {
    WorldBlock waterMeanCube = newWaterCube (totalSaturation / nWaterNeigbours);
    for (int i = 0; i < N_NEIGHBOURS; ++i) {
      if (simpleWorldMap.get (neigbours[i]).type == BT_WATER)
        simpleWorldMap.set (neigbours[i], waterMeanCube);
    }
  }
}


void WaterEngine::processVerticalWaterPair (Vec3i lowerCube, Vec3i upperCube) {
  float saturationSum = simpleWorldMap.get (lowerCube).fluidSaturation + simpleWorldMap.get (upperCube).fluidSaturation;
  if (saturationSum < WaterParams::VERTICAL_MERGING_THRESHOLD) {
    simpleWorldMap.set (lowerCube, newWaterCube (saturationSum));
    simpleWorldMap.set (upperCube, BT_AIR);
  }
  else {
    float upperCubeSaturation = saturationSum / (1 + WaterParams::NORMAL_VERTICAL_SATURATION_RATIO);
    if (upperCubeSaturation < WaterParams::MIN_SATURATION)
      upperCubeSaturation = WaterParams::MIN_SATURATION;

    float lowerCubeSaturation = upperCubeSaturation * WaterParams::NORMAL_VERTICAL_SATURATION_RATIO;
    if (lowerCubeSaturation > WaterParams::MAX_SATURATION) {
      lowerCubeSaturation = WaterParams::MAX_SATURATION;
      upperCubeSaturation = saturationSum - lowerCubeSaturation;
    }

    simpleWorldMap.set (lowerCube, newWaterCube (lowerCubeSaturation));
    simpleWorldMap.set (upperCube, newWaterCube (upperCubeSaturation));
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
