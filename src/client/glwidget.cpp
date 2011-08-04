#include <cmath>
#include <cassert>
#include <iostream>   // TODO: delete
#include <fstream>    // TODO: delete
#include <iomanip>    // TODO: delete

// TODO: delete when QGLWidget will be deleted
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <QImage>
#include <QGLWidget>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"
#include "common/math_utils.hpp"
#include "common/string_utils.hpp"
#include "common/linear_algebra.hpp"
#include "common/cube_geometry.hpp"

#include "client/GLTools/GLShaderLoader.h"
#include "client/visible_cube_set.hpp"
#include "client/client_world.hpp"
#include "client/glwidget.hpp"

#ifndef GL_TIME_ELAPSED
#define GL_TIME_ELAPSED 0x88BF
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#endif

const int N_MAX_BLOCKS_DRAWN = N_MAP_BLOCKS;

const double FPS_MEASURE_INTERVAL         = 1.; // sec
const double PHYSICS_PROCESSING_INTERVAL  = 0.2; // sec


// 0 means success
int loadGameMap () {
  std::ifstream heightMap ("resources/height_map" + toStr (TREE_HEIGHT) + ".txt");
  if (!heightMap.is_open ()) {
    std::cout << "Unable to open height map!\n";
    return 1;
  }

  simpleWorldMap.lockRepaint ();
  for (int x = 0; x < MAP_SIZE; ++x) {
    for (int y = 0; y < MAP_SIZE; ++y) {
      int height;
      heightMap >> height;
//       if (height > MAP_SIZE / 2) {
// //         cubeArray.addCube (x, y, height - 1, 66);
//         cubeOctree.set (x, y, height - 1, 1);
//         height--;
//       }
      for (int z = 0; z < height - 1; ++z) {
//         cubeArray.addCube (x, y, z, 2);
        simpleWorldMap.set (x, y, z, BT_DIRT);
      }
      if (height >= 1) {
        if (height > 5 * MAP_SIZE / 8)
          simpleWorldMap.set (x, y, height - 1, BT_SNOWY_DIRT);
        else
          simpleWorldMap.set (x, y, height - 1, BT_GRASS);
      }
      for (int z = height; z < MAP_SIZE / 2; ++z) {
        simpleWorldMap.set (x, y, z, BT_WATER);
      }
    }
  }
  simpleWorldMap.unlockRepaint ();

//   int MAX_NODE_VALUE = 256;
//   const TreeDataT* nodes = cubeOctree.nodes();
//   int nNodeValues[MAX_NODE_VALUE];
//   std::fill (nNodeValues, nNodeValues + MAX_NODE_VALUE, 0);
//   for (int i = 0; i < cubeOctree.nNodes(); ++i) {
//     int nodeValue = nodes[i * NODE_STRUCT_SIZE];
//     assert (nodeValue >= 0);
//     assert (nodeValue < MAX_NODE_VALUE);
//     nNodeValues [nodeValue]++;
//   }
//   std::cout << std::endl;
//
//   std::cout << "Cube type frequency:" << std::endl;
//   for (int i = 0; i < MAX_NODE_VALUE; ++i)
//     if (nNodeValues[i] != 0)
//       std::cout << i << ": " << nNodeValues[i] << std::endl;
//   std::cout << std::endl;

  std::cout << "nOctreeNodes = " << cubeOctree.nNodes () << std::endl;
  std::cout << std::endl;

  return 0;
}

/*
// 0 means success
int loadGameMap () {
  std::ifstream map ("resources/World1.schematic");
  if (!map.is_open ()) {
    std::cout << "Unable to open map!\n";
    return 1;
  }
  for (int z = 0; z < 128; ++z) {
    for (int y = 0; y < 80; ++y) {
      for (int x = 0; x < 80; ++x) {
        char curBlock;
        map >> curBlock;
        map >> curBlock;
        if (x < MAP_SIZE && y < MAP_SIZE && z < MAP_SIZE)
          cubeArray.addCube (x, y, z, curBlock);
      }
    }
  }
  std::cout << "nCubes = " << cubeArray.nCubes () << std::endl;
  return 0;
}
*/



void GLWidget::lockCubes () {
//   glBindTexture (GL_TEXTURE_BUFFER, m_octTreeTexture);
  glBindBuffer (GL_TEXTURE_BUFFER, m_octTreeBuffer);
  TreeDataT* buffer = (TreeDataT *) glMapBufferRange (GL_TEXTURE_BUFFER, 0, cubeOctree.nNodes() * sizeof (TreeNodeT), GL_MAP_WRITE_BIT);
  cubeOctree.setPointer (buffer);
}

void GLWidget::unlockCubes () {
  glUnmapBuffer (GL_TEXTURE_BUFFER);
  glBindBuffer (GL_TEXTURE_BUFFER, 0);
}

void GLWidget::explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius) {
  Vec3i corner1 = Vec3i (explosionX, explosionY, explosionZ) - Vec3i::replicated (explosionRadius);
  Vec3i corner2 = Vec3i (explosionX, explosionY, explosionZ) + Vec3i::replicated (explosionRadius + 1);
  simpleLightMap.calculateLight (corner1, corner2, -1.);
  simpleWorldMap.lockRepaint ();
  for  (int x = std::max (explosionX - explosionRadius, 0); x <= std::min (explosionX + explosionRadius, MAP_SIZE - 1); ++x)
    for  (int y = std::max (explosionY - explosionRadius, 0); y <= std::min (explosionY + explosionRadius, MAP_SIZE - 1); ++y)
      for  (int z = std::max (explosionZ - explosionRadius, 0); z <= std::min (explosionZ + explosionRadius, MAP_SIZE - 1); ++z) {
        if  (xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius)) {
          simpleWorldMap.set (x, y, z, BT_AIR);
        }
        else if  (   simpleWorldMap.get (x, y, z).type != BT_AIR
                  && xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius + 1)) {
//           simpleWorldMap.set (x, y, z, 239);
          simpleWorldMap.set (x, y, z, BT_BRICKS);
        }
      }
  simpleWorldMap.unlockRepaint ();
  simpleLightMap.calculateLight (corner1, corner2, 1.);
  simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, corner1, corner2);
}

void GLWidget::summonMeteorite (int meteoriteX, int meteoriteY) {
  const int METEORITE_RADIUS = 10;
  int meteoriteZ = MAP_SIZE - 1;
  while  (meteoriteZ > 0 && simpleWorldMap.get (meteoriteX, meteoriteY, meteoriteZ).type == BT_AIR)
    meteoriteZ--;
  explosion (meteoriteX, meteoriteY, meteoriteZ, METEORITE_RADIUS);
  Vec3i lightPos = Vec3i (meteoriteX, meteoriteY, meteoriteZ - METEORITE_RADIUS + 1);
  simpleLightMap.calculateLight(lightPos, -1);
  simpleWorldMap.set (lightPos, WorldBlock(BT_TEST_LIGHT));
  simpleLightMap.calculateLight(lightPos,  1);
  simpleLightMap.loadSubLightMapToTexture(m_lightMapTexture, lightPos);
}

//           up
//                  front
//          +------+                      6+------+7
//         /|     /|                      /|     /|
//        +------+ |                    4+------+5|
//  left  | |    | |  right              | |    | |
//        | +----|-+                     |2+----|-+3
//        |/     |/                      |/     |/
//        +------+                      0+------+1
//    back
//           down

// vertex indices
//                             {  4,   5,   7,   6,
//                                1,   3,   7,   5,
//                                2,   0,   4,   6,
//                                0,   1,   5,   4,
//                                3,   2,   6,   7,
//                                0,   2,   3,   1  }

// vertex coordinates
// 0:   -1., -1., -1.
// 1:    1., -1., -1.
// 2:   -1.,  1., -1.
// 3:    1.,  1., -1.
// 4:   -1., -1.,  1.
// 5:    1., -1.,  1.
// 6:   -1.,  1.,  1.
// 7:    1.,  1.,  1.

void GLWidget::initBuffers () {

  GLfloat proxySurfaceVertices[] = { 1, 1, 1,   -1, 1, 1,  -1, -1, 1,   1, -1, 1};
  GLfloat proxySurfaceDirections[] = { 1, 1, 1, -1, 1, 1,  -1, -1, 1,   1, -1, 1};

  glGenVertexArrays(1, &m_raytracingVAO);
  glBindVertexArray(m_raytracingVAO);
  glGenBuffers(1, &m_raytracingVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_raytracingVBO);
  glBufferData    (GL_ARRAY_BUFFER, sizeof (proxySurfaceVertices) + sizeof (proxySurfaceDirections), nullptr, GL_STATIC_DRAW);
  int offset = 0;
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (proxySurfaceVertices)  , proxySurfaceVertices);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (proxySurfaceVertices);
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (proxySurfaceDirections), proxySurfaceDirections);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  glEnableVertexAttribArray (0);
  glEnableVertexAttribArray (1);
  glBindVertexArray (0);


  glGenFramebuffers (1, &m_raytracingFBO);
  glBindFramebuffer (GL_DRAW_FRAMEBUFFER, m_raytracingFBO);

  glGenTextures(1, &m_raytracingFirstPassResult);
  glBindTexture(GL_TEXTURE_2D, m_raytracingFirstPassResult);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREEN_WIDTH / RAY_PACKET_WIDTH, SCREEN_HEIGHT / RAY_PACKET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_raytracingFirstPassResult, 0);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

void GLWidget::initTextures () {
  const int N_TEXTURES        = N_BLOCK_TYPES;
  const int TEXTURE_SIZE      = 16;

  const int N_NORMAL_MAPS     = 2;
  const int NORMAL_MAP_SIZE   = 16;

  const int N_DECALS          = 2;
  const int DECAL_SIZE        = 16;

  QImage imageTarget ("resources/images/target.png");
  assert (!imageTarget.isNull ());
  assert (imageTarget.format () == QImage::Format_ARGB32);

  QImage textureTarget = QGLWidget::convertToGLFormat (imageTarget);
  assert (!textureTarget.isNull ());

  glGenTextures (1, &m_UITexture);
  //glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, m_UITexture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, textureTarget.width (), textureTarget.height (), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureTarget.bits ());
  glBindTexture (GL_TEXTURE_2D, 0);

//   Octree octSubcube (2, TREE_HEIGHT);
//   octSubcube.set (0, 0, 0, WorldBlock (BT_BRICKS), false);
//   octSubcube.set (1, 1, 1, WorldBlock (BT_DIRT), false);
//   octSubcube.set (2, 2, 2, WorldBlock (BT_GRASS), false);
//   octSubcube.set (3, 3, 3, WorldBlock (BT_MIRROR), false);

  Octree octSubcube (5, TREE_HEIGHT);
  int subobjectSize = octSubcube.size();
  Vec3d subobjectCenter = Vec3d::replicated ((subobjectSize - 1.) / 2.);
  double radius = 0.4;
  for (int x = 0; x < subobjectSize; ++x)
    for (int y = 0; y < subobjectSize; ++y)
      for (int z = 0; z < subobjectSize; ++z)
        if (L2::norm (Vec3d (x, y, z) - subobjectCenter) / double (subobjectSize) < radius)
          octSubcube.set (x, y, z, WorldBlock (BT_SNOWY_DIRT), false);
  octSubcube.computeNeighbours();

  //here we go! EPIC TEXTURE BUFFERS!
  glGenBuffers (1, &m_octTreeBuffer);
  glBindBuffer (GL_TEXTURE_BUFFER, m_octTreeBuffer);
  glBufferData (GL_TEXTURE_BUFFER, (cubeOctree.nNodes() + octSubcube.nNodes()) * sizeof (TreeNodeT), 0, GL_STATIC_DRAW);  // TODO: STATIC or DYNAMIC?
  glBufferSubData (GL_TEXTURE_BUFFER, 0,                                        cubeOctree.nNodes() * sizeof (TreeNodeT), cubeOctree.nodes());
  glBufferSubData (GL_TEXTURE_BUFFER, cubeOctree.nNodes() * sizeof (TreeNodeT), octSubcube.nNodes() * sizeof (TreeNodeT), octSubcube.nodes());

  glBindBuffer (GL_TEXTURE_BUFFER, 0);
  glGenTextures(1, &m_octTreeTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_BUFFER, m_octTreeTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, m_octTreeBuffer);
  glBindTexture(GL_TEXTURE_BUFFER, 0);

  const char *szCubeFaces[6] = { "right", "left", "front", "back", "up", "down" };
  const float angles[6] = {-90, 90, 0, 180, 0, 0};
//   GLenum  cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
//                       GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
//                       GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
//                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
//                       GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
//                       GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


  glGenTextures(1, &m_cubeTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeTexture);

  // Set up texture maps
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 6 * N_TEXTURES, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  // Load Cube Map images
  for (int i = 0; i < N_TEXTURES; ++i)
    for(int j = 0; j < 6; ++j) {
      // Load this texture map
      //pBytes = gltReadTGABits(("resources/textures" + std::string(szCubeFaces[i])).c_str(), &iWidth, &iHeight, &iComponents, &eFormat);
      char textureFileName[256];
      sprintf (textureFileName, "resources/textures/blocks/%d/%s.tga", i, szCubeFaces[j]);
      //QImage rawTexture (("resources/textures/cubemaps/" + std::string(szCubeFaces[i])).c_str());
      QImage rawTexture (textureFileName);
      if (rawTexture.isNull ()) {
        std::cout << "Cannot open texture file ``" << textureFileName << "''!" << std::endl;
        exit (1);
      }
      QMatrix rotateMatrix;
      rotateMatrix.rotate (angles[j]);
      rawTexture =  rawTexture.transformed(rotateMatrix);
      QImage texture = QGLWidget::convertToGLFormat (rawTexture);
      assert (!texture.isNull ());
      glTexSubImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 6 * i + j, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
    }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);



  glGenTextures(1, &m_cubeNormalMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeNormalMap);

  // Set up texture maps
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, NORMAL_MAP_SIZE, NORMAL_MAP_SIZE, 6 * N_NORMAL_MAPS, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  // Load Cube Map images
  for (int i = 0; i < N_NORMAL_MAPS; ++i)
    for(int j = 0; j < 6; ++j) {
      char textureFileName[256];
      sprintf (textureFileName, "resources/textures/normals/%d/%s.tga", i, szCubeFaces[j]);
      //QImage rawTexture (("resources/textures/cubemaps/" + std::string(szCubeFaces[i])).c_str());
      QImage rawTexture (textureFileName);
      if (rawTexture.isNull ()) {
        std::cout << "Cannot open texture file ``" << textureFileName << "''!" << std::endl;
        exit (1);
      }
      QMatrix rotateMatrix;
      rotateMatrix.rotate (angles[j]);
      rawTexture =  rawTexture.transformed(rotateMatrix);
      QImage texture = QGLWidget::convertToGLFormat (rawTexture);
      assert (!texture.isNull ());
      glTexSubImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 6 * i + j, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
    }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);


  glGenTextures(1, &m_cubeDecal);
  glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeDecal);

  // Set up texture maps
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, DECAL_SIZE, DECAL_SIZE, 6 * N_DECALS, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  // Load Cube Map images
  for (int i = 0; i < N_DECALS; ++i)
    for(int j = 0; j < 6; ++j) {
      char textureFileName[256];
      sprintf (textureFileName, "resources/textures/decals/%d/%s.tga", i, szCubeFaces[j]);
      //QImage rawTexture (("resources/textures/cubemaps/" + std::string(szCubeFaces[i])).c_str());
      QImage rawTexture (textureFileName);
      if (rawTexture.isNull ()) {
        std::cout << "Cannot open texture file ``" << textureFileName << "''!" << std::endl;
        exit (1);
      }
      QMatrix rotateMatrix;
      rotateMatrix.rotate (angles[j]);
      rawTexture =  rawTexture.transformed(rotateMatrix);
      QImage texture = QGLWidget::convertToGLFormat (rawTexture);
      assert (!texture.isNull ());
      glTexSubImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 6 * i + j, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
    }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);

  const float cubeProperties[] = { 1,     1,     1,     0,
                                   0.85,  1.333, 1,     0,
                                   0,     99,    0,     0,
                                   0,     99,    0,     0,
                                   0,     99,    0,     0,
                                   0,     99,    0,     0,
                                   0,     0.001, 0,     0  };


  glGenBuffers (1, &m_cubePropertiesBuffer);
  glBindBuffer (GL_TEXTURE_BUFFER, m_cubePropertiesBuffer);
  glBufferData (GL_TEXTURE_BUFFER, sizeof (cubeProperties), cubeProperties, GL_STATIC_DRAW);
  glBindBuffer (GL_TEXTURE_BUFFER, 0);

  glGenTextures   (1, &m_cubePropertiesTexture);
  glActiveTexture (GL_TEXTURE3);
  glBindTexture   (GL_TEXTURE_BUFFER, m_cubePropertiesTexture);
  glTexBuffer     (GL_TEXTURE_BUFFER, GL_RGBA32F, m_cubePropertiesBuffer);
  glBindTexture   (GL_TEXTURE_BUFFER, 0);


  const int siblingShiftTable[] = {
/* Z-  Y-  X-      X+  Y+  Z+ */
   0,  0,  0,  0,  1,  2,  4,  /* 0 */
   0,  0, -1,  0,  0,  2,  4,  /* 1 */
   0, -2,  0,  0,  1,  0,  4,  /* 2 */
   0, -2, -1,  0,  0,  0,  4,  /* 3 */
  -4,  0,  0,  0,  1,  2,  0,  /* 4 */
  -4,  0, -1,  0,  0,  2,  0,  /* 5 */
  -4, -2,  0,  0,  1,  0,  0,  /* 6 */
  -4, -2, -1,  0,  0,  0,  0   /* 7 */ };

  glGenBuffers (1, &m_siblingShiftTableBuffer);
  glBindBuffer (GL_TEXTURE_BUFFER, m_siblingShiftTableBuffer);
  glBufferData (GL_TEXTURE_BUFFER, sizeof (siblingShiftTable), siblingShiftTable, GL_STATIC_DRAW);
  glBindBuffer (GL_TEXTURE_BUFFER, 0);

  glGenTextures   (1, &m_siblingShiftTableTexture);
  glActiveTexture (GL_TEXTURE4);
  glBindTexture   (GL_TEXTURE_BUFFER, m_siblingShiftTableTexture);
  glTexBuffer     (GL_TEXTURE_BUFFER, GL_R32I, m_siblingShiftTableBuffer);
  glBindTexture   (GL_TEXTURE_BUFFER, 0);



  glGenTextures(1, &m_lightMapTexture);
  glBindTexture(GL_TEXTURE_3D, m_lightMapTexture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D (GL_TEXTURE_3D, 0, GL_RGBA16F_ARB, MAP_SIZE, MAP_SIZE, MAP_SIZE, 0,
                GL_RGBA, GL_FLOAT, nullptr);
  simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, Vec3i (0, 0, 0), Vec3i (MAP_SIZE-1, MAP_SIZE-1, MAP_SIZE-1));

  glGenTextures(1, &m_sunVisibilityTexture);
  glBindTexture(GL_TEXTURE_3D, m_sunVisibilityTexture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D (GL_TEXTURE_3D, 0, GL_RGBA16F_ARB, MAP_SIZE, MAP_SIZE, MAP_SIZE, 0,
                GL_RGBA, GL_FLOAT, nullptr);
  simpleLightMap.loadVisibilityMapToTexture (m_sunVisibilityTexture);


}

// TODO: rename shader program files
void GLWidget::initShaders () {
//   if (!QGLShaderProgram::hasOpenGLShaderPrograms()) {
//     std::cout << "Your system does not support OpenGL custom shader programs :-(" << std::endl;
//     exit (1);
//   }

  //Raytracing shader initialization
  m_raytracingDepthPassShader = gltLoadShaderPairWithAttributes ("resources/RaytracingShader.vp", "resources/RaytracingShaderDepthPass.fp", 2,
                                                                 0, "vPosition",
                                                                 1, "vDirection");
  glLinkProgram (m_raytracingDepthPassShader);
  glUseProgram (m_raytracingDepthPassShader);
  m_locDepthPassOctTree       = glGetUniformLocation (m_raytracingDepthPassShader, "octTree");
  m_locDepthPassSiblingShiftTableTexture  =  glGetUniformLocation (m_raytracingDepthPassShader, "siblingShiftTable");
  m_locDepthPassOrigin        = glGetUniformLocation (m_raytracingDepthPassShader, "origin");
  m_locDepthPassViewMatrix    = glGetUniformLocation (m_raytracingDepthPassShader, "matView");
  m_locDepthPassCubeNormalMap = glGetUniformLocation (m_raytracingDepthPassShader, "cubeNormalMap");

  m_raytracingShader = gltLoadShaderPairWithAttributes ("resources/RaytracingShader.vp", "resources/RaytracingShader.fp", 2,
                                                        0, "vPosition",
                                                        1, "vDirection");
  glLinkProgram (m_raytracingShader);
  glUseProgram (m_raytracingShader);
  m_locOctTree                   =  glGetUniformLocation (m_raytracingShader, "octTree");
  m_locCubePropertiesTexture     =  glGetUniformLocation (m_raytracingShader, "cubeProperties");
  m_locSiblingShiftTableTexture  =  glGetUniformLocation (m_raytracingShader, "siblingShiftTable");
  m_locDepthTexture              =  glGetUniformLocation (m_raytracingShader, "depthTexture");
  m_locOrigin                    =  glGetUniformLocation (m_raytracingShader, "origin");
  m_locViewMatrix                =  glGetUniformLocation (m_raytracingShader, "matView");
  m_locCubeTexture               =  glGetUniformLocation (m_raytracingShader, "cubeTexture");
  m_locCubeNormalMap             =  glGetUniformLocation (m_raytracingShader, "cubeNormalMap");
  m_locCubeDecal                 =  glGetUniformLocation (m_raytracingShader, "cubeDecal");
  m_locLightMap                  =  glGetUniformLocation (m_raytracingShader, "lightMap");
  m_locSunVisibilityMap          =  glGetUniformLocation (m_raytracingShader, "sunVisibilityMap");

  m_UIShader = gltLoadShaderPairWithAttributes ("resources/UIShader.vp", "resources/UIShader.fp", 2,
                                                0, "vPosition",
                                                1, "vDirection");
  glLinkProgram (m_UIShader);
  glUseProgram (m_UIShader);
  m_locUITexture                 =  glGetUniformLocation (m_UIShader, "UITexture");
}

void GLWidget::initQueries () {
  glGenQueries (3, m_renderTimeQuery);
}

void GLWidget::setupRenderContext () {
  glClearColor  (0.0f, 0.0f, 0.0f, 1.0f);

  initShaders();
  initBuffers();
  initTextures();
  initQueries();
}

void GLWidget::shutdownRenderContext () {
  glDeleteProgram (m_UIShader);
  glDeleteProgram (m_raytracingDepthPassShader);
  glDeleteProgram (m_raytracingShader);

  glDeleteTextures (1, &m_cubePropertiesTexture);
  glDeleteTextures (1, &m_cubeTexture);
  glDeleteTextures (1, &m_raytracingFirstPassResult);
  glDeleteTextures (1, &m_octTreeTexture);
  glDeleteTextures (1, &m_siblingShiftTableTexture);

  glDeleteBuffers (1, &m_octTreeBuffer);
  glDeleteBuffers (1, &m_cubePropertiesBuffer);
  glDeleteBuffers (1, &m_siblingShiftTableBuffer);

  glDeleteBuffers (1, &m_raytracingVBO);
  glDeleteBuffers (1, &m_raytracingFBO);

  glDeleteVertexArrays (1, &m_raytracingVAO);

  glDeleteQueries (3, m_renderTimeQuery);
}




GLWidget::GLWidget (sf::Window& app) :
  m_app (app)
{
  m_isMovingForward   = 0;
  m_isMovingBackward  = 0;
  m_isMovingLeft      = 0;
  m_isMovingRight     = 0;
  m_isJumping         = 0;
  m_worldFreezed      = true;

  renderingEngine     = this;
  initializeGL();
}

GLWidget::~GLWidget () {
  // TODO: Is it the right place to call it?
  shutdownRenderContext ();
}

void GLWidget::initializeGL () {
  m_nFramesDrawn = 0;
  m_nPhysicsStepsProcessed = 0;

  player.setPos (Vec3d (0.1, 0.1, MAP_SIZE / 8.) + Vec3d::replicated (MAP_SIZE / 2.));
  player.viewFrame ().rotateLocalX (M_PI / 2. - 0.01);
  player.viewFrame ().rotateWorld (M_PI / 2. - 0.1, 0., 0., 1.);
  player.setFlying (true);

  setupRenderContext ();
  loadGameMap ();
  simpleLightMap.calculateLight (Vec3i::replicated (0), Vec3i::replicated (MAP_SIZE), 1.);
  simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, Vec3i::replicated (0), Vec3i::replicated (MAP_SIZE));
  for (int a = 0; a < MAP_SIZE; ++a)
    for (int b = 0; b < MAP_SIZE; ++b) {
      simpleLightMap.calculateSunlight (Vec3i (a, b, MAP_SIZE - 1), 1);
      simpleLightMap.calculateSunlight (Vec3i (a, 0, b), 1);
      simpleLightMap.calculateSunlight (Vec3i (a, MAP_SIZE - 1, b), 1);
      simpleLightMap.calculateSunlight (Vec3i (0, a, b), 1);
      simpleLightMap.calculateSunlight (Vec3i (MAP_SIZE - 1, a, b), 1);
    }
  simpleLightMap.loadVisibilityMapToTexture (m_sunVisibilityTexture);

//   glBindBuffer (GL_ARRAY_BUFFER, m_cubeVbo);
//   GLfloat* bufferPos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET, N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
//                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
//   GLfloat* bufferType = (GLfloat *) (bufferPos + 4 * N_MAX_BLOCKS_DRAWN);
//
//   cubeArray.setPointers (bufferPos, bufferType);
//   loadGameMap ();
//
//   glUnmapBuffer (GL_ARRAY_BUFFER);

  m_time.Reset ();
  m_fpsTime.Reset ();
  m_physicsTime.Reset ();
}


void GLWidget::paintGL () {

  GLenum windowBuff[] = {GL_BACK_LEFT};
  //GLenum fboBuffs[] = {GL_COLOR_ATTACHMENT0};
  //glCullFace (GL_BACK);
  glDisable (GL_CULL_FACE);
  glDisable (GL_DEPTH_TEST);
  //glEnable (GL_MULTISAMPLE);
  M3DMatrix44f matView;
  player.viewFrame().getCameraMatrix (matView, true);

  glBeginQuery (GL_TIME_ELAPSED, m_renderTimeQuery[0]);
  //Depth-pass of raytracing
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_raytracingFBO);
  //glDrawBuffers(1, fboBuffs);
  glViewport(0, 0, SCREEN_WIDTH / RAY_PACKET_WIDTH, SCREEN_HEIGHT / RAY_PACKET_HEIGHT);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (m_raytracingDepthPassShader);

  glUniform3fv (m_locDepthPassOrigin, 1, Vec3f::fromVectorConverted (player.viewFrame().origin() - Vec3d::replicated (MAP_SIZE / 2.)).data());
  glUniformMatrix4fv (m_locDepthPassViewMatrix, 1, GL_TRUE, matView);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_BUFFER, m_octTreeTexture);
  glUniform1i  (m_locDepthPassOctTree, 0);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture (GL_TEXTURE_BUFFER, m_siblingShiftTableTexture);
  glUniform1i  (m_locDepthPassSiblingShiftTableTexture, 4);

//   glActiveTexture (GL_TEXTURE5);
//   glBindTexture (GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeNormalMap);
//   glUniform1i (m_locCubeNormalMap, 5);

  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);

  glEndQuery (GL_TIME_ELAPSED);

  glBeginQuery (GL_TIME_ELAPSED, m_renderTimeQuery[1]);
  //Window-pass of raytracing
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_raytracingFBO);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  glDrawBuffers(1, windowBuff);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glUseProgram (m_raytracingShader);

  glUniform3fv (m_locOrigin, 1, Vec3f::fromVectorConverted (player.viewFrame().origin() - Vec3d::replicated (MAP_SIZE / 2.)).data());
  glUniformMatrix4fv (m_locViewMatrix, 1, GL_TRUE, matView);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_BUFFER, m_octTreeTexture);
  glUniform1i  (m_locOctTree, 0);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (GL_TEXTURE_2D, m_raytracingFirstPassResult);
  glUniform1i  (m_locDepthTexture, 1);

  glActiveTexture (GL_TEXTURE2);
  glBindTexture (GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeTexture);
  glUniform1i  (m_locCubeTexture, 2);

  glActiveTexture (GL_TEXTURE3);
  glBindTexture (GL_TEXTURE_BUFFER, m_cubePropertiesTexture);
  glUniform1i  (m_locCubePropertiesTexture, 3);

  glActiveTexture (GL_TEXTURE4);
  glBindTexture (GL_TEXTURE_BUFFER, m_siblingShiftTableTexture);
  glUniform1i  (m_locSiblingShiftTableTexture, 4);

  glActiveTexture (GL_TEXTURE5);
  glBindTexture (GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeNormalMap);
  glUniform1i (m_locCubeNormalMap, 5);

  glActiveTexture (GL_TEXTURE6);
  glBindTexture (GL_TEXTURE_CUBE_MAP_ARRAY, m_cubeDecal);
  glUniform1i (m_locCubeDecal, 6);

  glActiveTexture (GL_TEXTURE7);
  glBindTexture (GL_TEXTURE_3D, m_lightMapTexture);
  glUniform1i (m_locLightMap, 7);

  glActiveTexture (GL_TEXTURE8);
  glBindTexture (GL_TEXTURE_3D, m_sunVisibilityTexture);
  glUniform1i (m_locSunVisibilityMap, 8);

  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);

  glEndQuery (GL_TIME_ELAPSED);

  glBeginQuery (GL_TIME_ELAPSED, m_renderTimeQuery[2]);
  renderUI ();
  glEndQuery (GL_TIME_ELAPSED);

  GLuint depthPassTime, mainPassTime, UITime;
  glGetQueryObjectuiv(m_renderTimeQuery[0], GL_QUERY_RESULT, &depthPassTime);
  glGetQueryObjectuiv(m_renderTimeQuery[1], GL_QUERY_RESULT, &mainPassTime);
  glGetQueryObjectuiv(m_renderTimeQuery[2], GL_QUERY_RESULT, &UITime);

  m_totalDepthPassTime += depthPassTime;
  m_totalMainPassTime  += mainPassTime;
  m_totalUITime        += UITime;
  m_nFramesDrawn++;
}

void GLWidget::renderUI () {
  glUseProgram (m_UIShader);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, m_UITexture);
  glUniform1i  (m_locUITexture, 0);
  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);
  glBindTexture (GL_TEXTURE_2D, 0);
}


void GLWidget::updateGL() {
  m_app.SetActive();
  paintGL();
}


void GLWidget::resizeEvent (const sf::Event::SizeEvent& /*event*/) {
  updateGL ();
}

void GLWidget::keyPressEvent (const sf::Event::KeyEvent& event) {
  if (!event.Control && !event.Alt && !event.Shift) {
    switch (event.Code) {
      case sf::Keyboard::Key::W:
        m_isMovingForward = true;
        break;
      case sf::Keyboard::Key::S:
        m_isMovingBackward = true;
        break;
      case sf::Keyboard::Key::A:
        m_isMovingLeft = true;
        break;
      case sf::Keyboard::Key::D:
        m_isMovingRight = true;
        break;
      case sf::Keyboard::Key::Space:
        m_isJumping = true;
        break;
      case sf::Keyboard::Key::X: {
        Vec3d playerPos = player.pos ();
        summonMeteorite ((int) (playerPos[0]), (int) (playerPos[1]));
        break;
      }
      case sf::Keyboard::Key::Escape:
        exit (0);
        break;
      default:
        break;
    }
  }
  else if (event.Control && !event.Alt && !event.Shift) {
    switch (event.Code) {
      case sf::Keyboard::Key::S:
        simpleWorldMap.saveToFile ();
        break;
      case sf::Keyboard::Key::L:
        simpleWorldMap.loadFromFile ();
        simpleLightMap.clear();
        simpleLightMap.calculateLight (Vec3i::replicated (0), Vec3i::replicated (MAP_SIZE), 1.);
        simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, Vec3i::replicated (0), Vec3i::replicated (MAP_SIZE));
        break;
      case sf::Keyboard::Key::F:
        m_worldFreezed = !m_worldFreezed;
        break;
      case sf::Keyboard::Key::G:
        player.setFlying (!player.flying());
        break;
      default:
        break;
    }
  }

  updateGL ();
}

void GLWidget::keyReleaseEvent (const sf::Event::KeyEvent& event) {
  switch (event.Code) {
    case sf::Keyboard::Key::W:
      m_isMovingForward = false;
      break;
    case sf::Keyboard::Key::S:
      m_isMovingBackward = false;
      break;
    case sf::Keyboard::Key::A:
      m_isMovingLeft = false;
      break;
    case sf::Keyboard::Key::D:
      m_isMovingRight = false;
      break;
    case sf::Keyboard::Key::Space:
      m_isJumping = false;
      break;
    default:
      break;
  }
}

void GLWidget::mousePressEvent (const sf::Event::MouseButtonEvent& event) {
  switch (event.Button) {
    case sf::Mouse::Left: {
      Vec3i headOnCube = player.getHeadOnCube ().cube;
      if (!cubeIsValid (headOnCube))
        break;
//       explosion (XYZ_LIST (cube), 2);

      // WARNING: This light recomputation will not work correctly tor the sunlight
      simpleLightMap.calculateLight (headOnCube, -1.);
      simpleWorldMap.set (headOnCube, BT_AIR);
      simpleLightMap.calculateLight (headOnCube, 1.);
      simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, headOnCube);
      break;
    }
    case sf::Mouse::Right: {
      CubeWithFace headOnCube = player.getHeadOnCube ();
      if (!directionIsValid (headOnCube.face)) {
        std::cout << "Invalid cube position" << std::endl;
        break;
      }
      Vec3i newCube = getAdjacentCube (headOnCube).cube;
      if (!cubeIsValid (newCube) || player.intersectsCube (newCube)) {
        std::cout << "Invalid cube position" << std::endl;
        break;
      }
      simpleLightMap.calculateLight (newCube, -1.);
      simpleWorldMap.set (newCube, player.getBlockInHand ());
      simpleLightMap.calculateLight (newCube, 1.);
      //simpleLightMap.lightThatCubePlease(newCube);
      simpleLightMap.loadSubLightMapToTexture (m_lightMapTexture, newCube);
      std::cout << "Cube " << player.getBlockInHand () << " set" << std::endl;
      break;
    }
    default:
      break;
  }
}

void GLWidget::mouseWheelEvent (const sf::Event::MouseWheelEvent& event) {
  player.setBlockInHand (static_cast <BlockType> ((player.getBlockInHand () + xSgn (event.Delta) + N_BLOCK_TYPES) % N_BLOCK_TYPES));
  std::cout << "Cur cube type: " << player.getBlockInHand () << std::endl;
}

void GLWidget::timerEvent() {
  double timeElasped = m_time.GetElapsedTime () / 1000.;
  m_time.Reset ();

  if (m_isMovingForward)
    player.moveForward (8. * timeElasped);
  if (m_isMovingBackward)
    player.moveForward (-6. * timeElasped);
  if (m_isMovingLeft)
    player.moveRight (-6. * timeElasped);
  if (m_isMovingRight)
    player.moveRight (6. * timeElasped);
  if (m_isJumping)
    player.jump();

  sf::Vector2i windowsCenter (m_app.GetWidth() / 2, m_app.GetHeight() / 2);
  sf::Vector2i mouseDelta = sf::Mouse::GetPosition (m_app) - windowsCenter;
  player.viewFrame ().rotateWorld (mouseDelta.x / 100., 0., 0., 1.);
  player.viewFrame ().rotateLocalX (-mouseDelta.y / 100.);
  sf::Mouse::SetPosition (windowsCenter, m_app);

  double fpsTimeElapsed = m_fpsTime.GetElapsedTime () / 1000.;
  if (fpsTimeElapsed > FPS_MEASURE_INTERVAL) {
    std::cout << "fps =" << std::setw (4) << m_nFramesDrawn << ", pps =" << std::setw (4) << m_nPhysicsStepsProcessed
              << ", video fps =" << std::setw (8) << 1000. / (0.000001 * (m_totalDepthPassTime + m_totalMainPassTime + m_totalUITime) / m_nFramesDrawn) << std::endl;
//     std::cout << "Depth pass time: "      << 0.000001 * m_totalDepthPassTime / m_nFramesDrawn
//               << ", Main pass time: "     << 0.000001 * m_totalMainPassTime  / m_nFramesDrawn
//               << ", UI time: "            << 0.000001 * m_totalUITime        / m_nFramesDrawn
//               << ", total drawing time: " << 0.000001 * (m_totalDepthPassTime + m_totalMainPassTime + m_totalUITime) / m_nFramesDrawn << std::endl;
    m_nFramesDrawn = 0;
    m_totalDepthPassTime = 0;
    m_totalMainPassTime = 0;
    m_totalUITime = 0;

    m_nPhysicsStepsProcessed = 0;
    m_fpsTime.Reset ();
  }

  if (!m_worldFreezed) {
    double physicsTimeElapsed = m_physicsTime.GetElapsedTime () / 1000.;
    if (physicsTimeElapsed > PHYSICS_PROCESSING_INTERVAL) {
      waterEngine.processWater ();
      m_nPhysicsStepsProcessed++;
      m_physicsTime.Reset ();
    }
  }

  player.processPlayer (timeElasped);

  updateGL ();
}
