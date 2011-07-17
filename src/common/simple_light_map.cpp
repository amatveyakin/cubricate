// TODO: delete
#include <QtCore/QTime>
#include <iostream>
#include <iomanip>



#include <cmath>

#include "common/simple_light_map.hpp"

#include "client/client_world.hpp"


const float blockTransparency[] = { 0.95, 0.5, 0, 0, 0, 0, 1, 1}; //TODO move?

const int N_ITERATIONS = 16;

// #define GRAFA_KAK_V_KRUZISE

// this cool constants are spizzhened (from Andreas Kirsch), need to check them
#define SH_DELTA_FUNCTION_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_DELTA_FUNCTION_C1 0.488602512f // sqrt(3/pi) / 2

#define MAIN_DIRECTION_SOLID_ANGLE (0.4006696846f / M_PI)
#define SIDE_DIRECTION_SOLID_ANGLE (0.4234413544f / M_PI)
#define NEAREST_FACE_SOLID_ANGLE   0.6666666666f

#ifndef GRAFA_KAK_V_KRUZISE
//this cool constants are spizzhened from Andreas Kirsch
#define SH_COSLOBE_C0 0.886226925f /* sqrt(pi)/2 */
#define SH_COSLOBE_C1 1.02332671f /* sqrt(pi/3) */
#else // GRAFA_KAK_V_KRUZISE
//and this - from crytek paper
#define SH_COSLOBE_C0 0.25f
#define SH_COSLOBE_C1 0.5f
#endif // GRAFA_KAK_V_KRUZISE

const Vec3f neighbourVector[] = { Vec3f (1,  0,  0),
                                  Vec3f (0,  1,  0),
                                  Vec3f (0,  0,  1)
                                };

#define NEIGHBOUR_FACE_VECTOR_X 0.894427 //  2 / sqrt (5)
#define NEIGHBOUR_FACE_VECTOR_Y 0.447213 //  1 / sqrt (5)

const Vec3f neigbourFaceVector[] = {
  // (1, 0, 0) neighbour
  Vec3f ( NEIGHBOUR_FACE_VECTOR_X,  NEIGHBOUR_FACE_VECTOR_Y,  0                      ),
  Vec3f ( NEIGHBOUR_FACE_VECTOR_X,  0,                        NEIGHBOUR_FACE_VECTOR_Y),
  Vec3f ( NEIGHBOUR_FACE_VECTOR_X, -NEIGHBOUR_FACE_VECTOR_Y,  0                      ),
  Vec3f ( NEIGHBOUR_FACE_VECTOR_X,  0,                       -NEIGHBOUR_FACE_VECTOR_Y),
  // (0, 1, 0) neighbour
  Vec3f ( 0,                        NEIGHBOUR_FACE_VECTOR_X,  NEIGHBOUR_FACE_VECTOR_Y),
  Vec3f ( NEIGHBOUR_FACE_VECTOR_Y,  NEIGHBOUR_FACE_VECTOR_X,  0                      ),
  Vec3f ( 0,                        NEIGHBOUR_FACE_VECTOR_X, -NEIGHBOUR_FACE_VECTOR_Y),
  Vec3f (-NEIGHBOUR_FACE_VECTOR_Y,  NEIGHBOUR_FACE_VECTOR_X,  0                      ),
  // (0, 0, 1) neighbour
  Vec3f ( NEIGHBOUR_FACE_VECTOR_Y,  0,                        NEIGHBOUR_FACE_VECTOR_X),
  Vec3f ( 0,                        NEIGHBOUR_FACE_VECTOR_Y,  NEIGHBOUR_FACE_VECTOR_X),
  Vec3f (-NEIGHBOUR_FACE_VECTOR_Y,  0,                        NEIGHBOUR_FACE_VECTOR_X),
  Vec3f ( 0,                       -NEIGHBOUR_FACE_VECTOR_Y,  NEIGHBOUR_FACE_VECTOR_X)
};

#define REPROJ_VECTOR_PARAMETER 0.0

const Vec3f reprojVector[] = {
  // (1, 0, 0) neighbour
  Vec3f ( REPROJ_VECTOR_PARAMETER,  0.5,    0),
  Vec3f ( REPROJ_VECTOR_PARAMETER,  0,    0.5),
  Vec3f ( REPROJ_VECTOR_PARAMETER, -0.5,    0),
  Vec3f ( REPROJ_VECTOR_PARAMETER,  0,   -0.5),
  // (0, 1, 0) neighbour
  Vec3f ( 0,    REPROJ_VECTOR_PARAMETER,  0.5),
  Vec3f ( 0.5,  REPROJ_VECTOR_PARAMETER,  0  ),
  Vec3f ( 0,    REPROJ_VECTOR_PARAMETER, -0.5),
  Vec3f (-0.5,  REPROJ_VECTOR_PARAMETER,  0  ),
  // (0, 0, 1) neighbour
  Vec3f ( 0.5,  0,    REPROJ_VECTOR_PARAMETER),
  Vec3f ( 0,    0.5,  REPROJ_VECTOR_PARAMETER),
  Vec3f (-0.5,  0,    REPROJ_VECTOR_PARAMETER),
  Vec3f ( 0,   -0.5,  REPROJ_VECTOR_PARAMETER)
};



SHCoefficients SimpleLightMap::deltaFunctionSH (Vec3f direction) {
  return SHCoefficients (SH_DELTA_FUNCTION_C0,
                         direction.x() * SH_DELTA_FUNCTION_C1,
                         direction.y() * SH_DELTA_FUNCTION_C1,
                         direction.z() * SH_DELTA_FUNCTION_C1);

}

SHCoefficients SimpleLightMap::cosineLobeSH (Vec3f direction) {
  direction = L2::normalize (direction);
  return SHCoefficients (SH_COSLOBE_C0,
                         direction.x() * SH_COSLOBE_C1,
                         direction.y() * SH_COSLOBE_C1,
                         direction.z() * SH_COSLOBE_C1);
}



SimpleLightMap::SimpleLightMap (int sizeX, int sizeY, int sizeZ) :
  m_luminosity (sizeX, sizeY, sizeZ)
{
  std::fill (m_luminosity.data(), m_luminosity.data() + m_luminosity.totalElements(), Vec4f::zero());
}

SimpleLightMap::~SimpleLightMap() { }

// now i consider that secondCorner > firstCorner
void SimpleLightMap::calculateLight (Vec3i firstCorner, Vec3i secondCorner, float multiplier) {
  QTime time;
  time.start();

  firstCorner  = xMax (firstCorner  - Vec3i::replicated (N_ITERATIONS), Vec3i::replicated (0));
  secondCorner = xMin (secondCorner + Vec3i::replicated (N_ITERATIONS), Vec3i::replicated (MAP_SIZE));

  Vec3i   diagonal = secondCorner - firstCorner;
  // "1-based" array, zeroes on the border
  Array3D <SHCoefficients> changedLuminosity (diagonal.x(), diagonal.y(), diagonal.z());

  // injecting lights
  for (int x = 0; x < diagonal.x(); ++x)
    for (int y = 0; y < diagonal.y(); ++y)
      for (int z = 0; z < diagonal.z(); ++z) {
        changedLuminosity (x, y, z) = SHCoefficients (0, 0, 0, 0);  //TODO change it
        if (simpleWorldMap.get (firstCorner + Vec3i (x, y, z)).type == BT_TEST_LIGHT)
          changedLuminosity (x, y, z) += SHCoefficients (500, 0, 0, 0);  //TODO change it
        if (z + firstCorner.z() >= simpleWorldMap.highestPoint (x, y) + 1)
          changedLuminosity (x, y, z) += SHCoefficients (0.3, 0, 1, -2);  //TODO change it
      }

  // iterating. This cycles look cool and REALLY FAST :)
  // probably, set-trick (like with water) must be implemented
  for (int iter = 0; iter < N_ITERATIONS; ++iter) {
    for (int x = 0; x < diagonal.x(); ++x) {
      for (int y = 0; y < diagonal.y(); ++y) {
        int zMin = (x + y + firstCorner.x() + firstCorner.y() + firstCorner.z() + iter + 1) % 2;
        if (iter > 0)
          for (int z = zMin; z < diagonal.z(); z += 2) //removing old neighbours
            changedLuminosity (x, y, z) = Vec4f (0, 0, 0, 0);
      }
    }

    for (int x = 0; x < diagonal.x(); ++x) {
      for (int y = 0; y < diagonal.y(); ++y) {
        int zMin = (x + y + firstCorner.x() + firstCorner.y() + firstCorner.z() + iter) % 2;
        for (int z = zMin; z < diagonal.z(); z += 2) { //"chessboard"-like iterations
          for (int iNeighbour = 0; iNeighbour < 6; ++iNeighbour) {

            Vec3i currCube (x, y, z);

            SHCoefficients transferedSH  (0, 0, 0, 0);
            SHCoefficients accumulatedSH (0, 0, 0, 0);

            Vec3f currNeighbourVector (neighbourVector [iNeighbour % 3]);
            if (iNeighbour >= 3) currNeighbourVector = -currNeighbourVector;

            Vec3i currNeighbour = currCube + Vec3i::fromVectorConverted (currNeighbourVector);

            if (currNeighbour.x() < 0 || currNeighbour.y() < 0 || currNeighbour.z() < 0 ||
                currNeighbour.x() >= diagonal.x() || currNeighbour.y() >= diagonal.y() || currNeighbour.z() >= diagonal.z()) continue;

            float neighbourTransparency = blockTransparency [simpleWorldMap.get (currNeighbour + firstCorner).type];

            //SHCoefficients neighbourSH = changedLuminosity (currNeighbour);
            SHCoefficients currCubeSH = changedLuminosity (currCube);

            SHCoefficients mainDirDeltaFunctionSH = deltaFunctionSH (currNeighbourVector); // because we using propagation schema now
            SHCoefficients mainDirCosineLobeSh    = cosineLobeSH (currNeighbourVector);

            SHCoefficients fluxThroughFaceVPLEquivalent = dotProduct (currCubeSH, mainDirDeltaFunctionSH) * mainDirCosineLobeSh;

            if (neighbourTransparency > 0.) {
              transferedSH += MAIN_DIRECTION_SOLID_ANGLE * fluxThroughFaceVPLEquivalent;

              for (int iFace = 0; iFace < 4; ++iFace) {
                int faceIndex = (iNeighbour % 3) * 4 + iFace;
                Vec3f currNeighbourFaceVector = neigbourFaceVector [faceIndex];
                Vec3f currReprojVector = reprojVector [faceIndex];
                if (iNeighbour >= 3) {
                  currNeighbourFaceVector = -currNeighbourFaceVector; // propagation schema
                  currReprojVector        = -currReprojVector;
                }

                SHCoefficients sideDirDeltaFunctionSH = deltaFunctionSH (currNeighbourFaceVector);
                SHCoefficients sideDirCosineLobeSH    = cosineLobeSH    (currReprojVector);
                transferedSH += SIDE_DIRECTION_SOLID_ANGLE * dotProduct (currCubeSH, sideDirDeltaFunctionSH) * sideDirCosineLobeSH;
              }
            }

            accumulatedSH = NEAREST_FACE_SOLID_ANGLE * fluxThroughFaceVPLEquivalent * (1 - neighbourTransparency);
            m_luminosity (currCube + firstCorner) += multiplier * accumulatedSH;

            changedLuminosity (currNeighbour) += transferedSH * neighbourTransparency;

          }
        }
      }
    }
  }


  for (int x = 0; x < diagonal.x(); ++x)
    for (int y = 0; y < diagonal.y(); ++y)
      for (int z = 0; z < diagonal.z(); ++z)
        m_luminosity (x, y, z) += multiplier * changedLuminosity (Vec3i (x, y, z) + firstCorner);



//   for (int y = 7; y >= 0; --y) {
//     for (int x = 0; x < 8; ++x)
//       std::cout  <<  "("  << std::setw (7) << m_luminosity (x, y, 1).r() << ","
//                           << std::setw (7) << m_luminosity (x, y, 1).g() << ","
//                           << std::setw (7) << m_luminosity (x, y, 1).b() << ","
//                           << std::setw (7) << m_luminosity (x, y, 1).a() << ")";
//     std::cout << std::endl;
//   }

  std::cout << "light time: " << time.elapsed() << " ms" << std::endl;
}

void SimpleLightMap::calculateLight (Vec3i modifiedCube, float multiplier) {
  calculateLight (xMax (modifiedCube, Vec3i::replicated (0)),
                  xMin (modifiedCube + Vec3i::replicated (1), Vec3i::replicated (MAP_SIZE - 1)),
                  multiplier);
}


void SimpleLightMap::loadSubLightMapToTexture (GLuint texture, Vec3i modifiedCube) {
  loadSubLightMapToTexture (texture,
                            xMax (modifiedCube - Vec3i::replicated (N_ITERATIONS), Vec3i::replicated (0)),
                            xMin (modifiedCube + Vec3i::replicated (N_ITERATIONS + 1), Vec3i::replicated (MAP_SIZE - 1)));
}

void SimpleLightMap::loadSubLightMapToTexture (GLuint texture, Vec3i firstCorner, Vec3i secondCorner) {
  Vec4f* data = new Vec4f [(secondCorner.x() - firstCorner.x()) *
                           (secondCorner.y() - firstCorner.y()) *
                           (secondCorner.z() - firstCorner.z())   ];
  int offset = 0;
  for (int z = firstCorner.z(); z < secondCorner.z(); ++z) {
   for (int y = firstCorner.y(); y < secondCorner.y(); ++y) {
     for (int x = firstCorner.x(); x < secondCorner.x(); ++x) {
        data [offset] = m_luminosity (x, y, z);
        offset++;
      }
    }
  }
  glBindTexture (GL_TEXTURE_3D, texture);
  glTexSubImage3D (GL_TEXTURE_3D, 0, firstCorner.x(), firstCorner.y(), firstCorner.z(),
                   secondCorner.x() - firstCorner.x(), secondCorner.y() - firstCorner.y(), secondCorner.z() - firstCorner.z(),
                   GL_RGBA, GL_FLOAT, data);
  glBindTexture (GL_TEXTURE_3D, 0);

  delete[] data;
}

void SimpleLightMap::lightThatCubePlease (Vec3i cube)
{
  m_luminosity (cube) = Vec4f (10, 0, 0, 0);
}


