#include <cmath>
#include <cassert>
#include <iostream>   // TODO: delete
#include <fstream>    // TODO: delete

#include <QTime>
#include <QKeyEvent>

#include "common/utils.hpp"
#include "common/game_parameters.hpp"
#include "common/math_utils.hpp"
#include "common/string_utils.hpp"
#include "common/linear_algebra.hpp"
#include "common/cube_geometry.hpp"

#include "client/visible_cube_set.hpp"
#include "client/client_world.hpp"
#include "client/glwidget.hpp"



const int N_MAX_BLOCKS_DRAWN = N_MAP_BLOCKS;

const double FPS_MEASURE_INTERVAL         = 1.; // sec
const double PHYSICS_PROCESSING_INTERVAL  = 0.02; // sec



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
  TreeDataT* buffer = (TreeDataT *) glMapBufferRange (GL_TEXTURE_BUFFER, 0,
                                                      cubeOctree.nNodes() * sizeof (TreeNodeT),
                                                      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
  cubeOctree.setPointer (buffer);
}

void GLWidget::unlockCubes () {
  glUnmapBuffer (GL_TEXTURE_BUFFER);
  glBindBuffer (GL_TEXTURE_BUFFER, 0);
}

void GLWidget::explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius) {
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
}

void GLWidget::summonMeteorite (int meteoriteX, int meteoriteY) {
  const int METEORITE_RADIUS = 10;
  int meteoriteZ = MAP_SIZE - 1;
  while  (meteoriteZ > 0 && simpleWorldMap.get (meteoriteX, meteoriteY, meteoriteZ).type == BT_AIR)
    meteoriteZ--;
  explosion (meteoriteX, meteoriteY, meteoriteZ, METEORITE_RADIUS);
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
  const int N_TEXTURES    = 6;
  const int TEXTURE_SIZE  = 16;

//   glGenTextures (1, &m_squareTextureArray);
//   glBindTexture (GL_TEXTURE_2D_ARRAY, m_squareTextureArray);
//   glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
//   glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
//   glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
//   glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//   glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//   glTexImage3D (GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, N_TEXTURES, 0,
//                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//
//   for (int i = 0; i < N_TEXTURES; i++) {
//     char textureFileName[256];
//     sprintf (textureFileName, "resources/textures/tile_%d.tga", i);
//
//     QImage rawTexture (textureFileName);
//     if (rawTexture.isNull ()) {
//       std::cout << "Cannot open texture file ``" << textureFileName << "''" << std::endl;
//       exit (1);
//     }
//
//     QImage texture = convertToGLFormat (rawTexture);
//     assert (!texture.isNull ());
//
//     glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
//   }


  QImage imageTarget ("resources/images/target.png");
  assert (!imageTarget.isNull ());
  assert (imageTarget.format () == QImage::Format_ARGB32);

  QImage textureTarget = convertToGLFormat (imageTarget);
  assert (!textureTarget.isNull ());

  glGenTextures (1, &m_UITexture);
  //glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, m_UITexture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
//   std::cout << "textureTarget.width () = " << textureTarget.width () << ", textureTarget.height () = " << textureTarget.height () << std::endl;
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, textureTarget.width (), textureTarget.height (), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureTarget.bits ());
  glBindTexture (GL_TEXTURE_2D, 0);

  //here we go! EPIC TEXTURE BUFFERS!
  glGenBuffers(1, &m_octTreeBuffer);
  glBindBuffer(GL_TEXTURE_BUFFER, m_octTreeBuffer);
  glBufferData(GL_TEXTURE_BUFFER, cubeOctree.nNodes () * sizeof (TreeNodeT), cubeOctree.nodes (), GL_STATIC_DRAW);  // TODO: STATIC ?
  glBindBuffer(GL_TEXTURE_BUFFER, 0);

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
      sprintf (textureFileName, "resources/textures/cubemaps/%d/%s.tga", i, szCubeFaces[j]);
      //QImage rawTexture (("resources/textures/cubemaps/" + std::string(szCubeFaces[i])).c_str());
      QImage rawTexture (textureFileName);
      if (rawTexture.isNull ()) {
        std::cout << "Cannot open texture file !!!" << std::endl;
        exit (1);
      }
      QMatrix rotateMatrix;
      rotateMatrix.rotate (angles[j]);
      rawTexture =  rawTexture.transformed(rotateMatrix);
    QImage texture = convertToGLFormat (rawTexture);
    assert (!texture.isNull ());
    glTexSubImage3D (GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 6 * i + j, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
  }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);

  const float cubeProperties[] = { 0.993, 1,
                                   0.95,  1.333,
//                                    0.,    1.333,
                                   0,     1,
                                   0,     1,
                                   0,     1,
                                   0,     1,
                                   0,     1      };


  glGenBuffers (1, &m_cubePropertiesBuffer);
  glBindBuffer (GL_TEXTURE_BUFFER, m_cubePropertiesBuffer);
  glBufferData (GL_TEXTURE_BUFFER, sizeof (cubeProperties), cubeProperties, GL_STATIC_DRAW);
  glBindBuffer (GL_TEXTURE_BUFFER, 0);

  glGenTextures   (1, &m_cubePropertiesTexture);
  glActiveTexture (GL_TEXTURE3);
  glBindTexture   (GL_TEXTURE_BUFFER, m_cubePropertiesTexture);
  glTexBuffer     (GL_TEXTURE_BUFFER, GL_RG32F, m_cubePropertiesBuffer);
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


}

// TODO: rename shader program files
void GLWidget::initShaders () {
  if (!QGLShaderProgram::hasOpenGLShaderPrograms()) {
    std::cout << "Your system does not support OpenGL custom shader programs :-(" << std::endl;
    exit (1);
  }

  bool result;

  //Raytracing shader initialization
  result = m_raytracingDepthPassShaderProgram.addShaderFromSourceFile (QGLShader::Vertex, "resources/RaytracingShader.vp");
  result = m_raytracingDepthPassShaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/RaytracingShaderDepthPass.fp");
  m_raytracingDepthPassShaderProgram.bindAttributeLocation ("vPosition",  0);
  m_raytracingDepthPassShaderProgram.bindAttributeLocation ("vDirection", 1);
  result = m_raytracingDepthPassShaderProgram.link ();
  m_raytracingDepthPassShader = m_raytracingDepthPassShaderProgram.programId ();
  glLinkProgram (m_raytracingDepthPassShader);
  glUseProgram (m_raytracingDepthPassShader);
  m_locDepthPassOctTree    = glGetUniformLocation (m_raytracingDepthPassShader, "octTree");
  m_locDepthPassSiblingShiftTableTexture  =  glGetUniformLocation (m_raytracingDepthPassShader, "siblingShiftTable");
  m_locDepthPassOrigin     = glGetUniformLocation (m_raytracingDepthPassShader, "origin");
  m_locDepthPassViewMatrix = glGetUniformLocation (m_raytracingDepthPassShader, "matView");

  result = m_raytracingShaderProgram.addShaderFromSourceFile (QGLShader::Vertex, "resources/RaytracingShader.vp");
  result = m_raytracingShaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/RaytracingShader.fp");
  m_raytracingShaderProgram.bindAttributeLocation ("vPosition",  0);
  m_raytracingShaderProgram.bindAttributeLocation ("vDirection", 1);
  result = m_raytracingShaderProgram.link ();
  m_raytracingShader = m_raytracingShaderProgram.programId ();
  glLinkProgram (m_raytracingShader);
  glUseProgram (m_raytracingShader);
  m_locOctTree                   =  glGetUniformLocation (m_raytracingShader, "octTree");
  m_locCubePropertiesTexture     =  glGetUniformLocation (m_raytracingShader, "cubeProperties");
  m_locSiblingShiftTableTexture  =  glGetUniformLocation (m_raytracingShader, "siblingShiftTable");
  m_locDepthTexture              =  glGetUniformLocation (m_raytracingShader, "depthTexture");
  m_locOrigin                    =  glGetUniformLocation (m_raytracingShader, "origin");
  m_locViewMatrix                =  glGetUniformLocation (m_raytracingShader, "matView");
  m_locCubeTexture               =  glGetUniformLocation (m_raytracingShader, "cubeTexture");

  result = m_UIShaderProgram.addShaderFromSourceFile (QGLShader::Vertex,   "resources/UIShader.vp");
  result = m_UIShaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/UIShader.fp");
  m_UIShaderProgram.bindAttributeLocation ("vPosition",  0);
  m_UIShaderProgram.bindAttributeLocation ("vDirection", 1);
  result = m_UIShaderProgram.link ();
  m_UIShader = m_UIShaderProgram.programId ();
  glLinkProgram (m_UIShader);
  glUseProgram (m_UIShader);
  m_locUITexture                 =  glGetUniformLocation (m_UIShader, "UITexture");
}

void GLWidget::setupRenderContext () {
  glClearColor  (0.0f, 0.0f, 0.0f, 1.0f);

  initShaders ();
  initBuffers ();
  initTextures ();
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
}




GLWidget::GLWidget () {
  setMouseTracking (true);
  setCursor (Qt::BlankCursor);

  m_isMovingForward   = false;
  m_isMovingBackward  = false;
  m_isMovingLeft      = false;
  m_isMovingRight     = false;
}

GLWidget::~GLWidget () {
  // TODO: Is it the right place to call it?
  shutdownRenderContext ();
}

void GLWidget::initializeGL () {
  m_nFramesDrawn = 0;

  player.setPos (Vec3d (0.1, 0.1, MAP_SIZE / 8.));
  player.viewFrame ().rotateLocalX (-M_PI / 2. + 0.01);
  player.viewFrame ().rotateWorld (M_PI / 2. - 0.1, 0., 0., 1.);

  setupRenderContext ();
  loadGameMap ();

//   glBindBuffer (GL_ARRAY_BUFFER, m_cubeVbo);
//   GLfloat* bufferPos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET, N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
//                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
//   GLfloat* bufferType = (GLfloat *) (bufferPos + 4 * N_MAX_BLOCKS_DRAWN);
//
//   cubeArray.setPointers (bufferPos, bufferType);
//   loadGameMap ();
//
//   glUnmapBuffer (GL_ARRAY_BUFFER);

  m_time.start ();
  m_fpsTime.start ();
  m_physicsTime.start ();
  startTimer (1);
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

  //Depth-pass of raytracing
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_raytracingFBO);
  //glDrawBuffers(1, fboBuffs);
  glViewport(0, 0, SCREEN_WIDTH / RAY_PACKET_WIDTH, SCREEN_HEIGHT / RAY_PACKET_HEIGHT);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (m_raytracingDepthPassShader);

  glUniform3fv (m_locDepthPassOrigin, 1, Vec3f::fromVectorConverted(player.pos()).data());
  glUniformMatrix4fv (m_locDepthPassViewMatrix, 1, GL_TRUE, matView);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_BUFFER, m_octTreeTexture);
  glUniform1i  (m_locDepthPassOctTree, 0);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture (GL_TEXTURE_BUFFER, m_siblingShiftTableTexture);
  glUniform1i  (m_locDepthPassSiblingShiftTableTexture, 4);

  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);


  //Window-pass of raytracing
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_raytracingFBO);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  glDrawBuffers(1, windowBuff);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glUseProgram (m_raytracingShader);

  glUniform3fv (m_locOrigin, 1, Vec3f::fromVectorConverted(player.pos()).data());
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

  glActiveTexture(GL_TEXTURE4);
  glBindTexture (GL_TEXTURE_BUFFER, m_siblingShiftTableTexture);
  glUniform1i  (m_locSiblingShiftTableTexture, 4);


  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);

  renderUI ();
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

void GLWidget::resizeGL (int width, int height) {
  FIX_UNUSED (width);
  if (height <= 0)
    height = 1;

  updateGL ();
}


void GLWidget::keyPressEvent (QKeyEvent* event) {
//   TODO: use this: event->nativeScanCode ()

  switch (event->key ()) {
    case Qt::Key_W:
      m_isMovingForward = true;
      break;
    case Qt::Key_S:
      m_isMovingBackward = true;
      break;
    case Qt::Key_A:
      m_isMovingLeft = true;
      break;
    case Qt::Key_D:
      m_isMovingRight = true;
      break;
    case Qt::Key_X: {
      Vec3d playerPos = player.pos ();
      summonMeteorite ((int) (playerPos[0] + MAP_SIZE / 2.), (int) (playerPos[1] + MAP_SIZE / 2.));
      break;
    }
    case Qt::Key_Escape:
      exit (0);
      break;
  }

  updateGL ();
}

void GLWidget::keyReleaseEvent (QKeyEvent* event) {
//   TODO: use this: event->nativeScanCode ()

  switch (event->key ()) {
    case Qt::Key_W:
      m_isMovingForward = false;
      break;
    case Qt::Key_S:
      m_isMovingBackward = false;
      break;
    case Qt::Key_A:
      m_isMovingLeft = false;
      break;
    case Qt::Key_D:
      m_isMovingRight = false;
      break;
  }
}

void GLWidget::mouseMoveEvent (QMouseEvent* event) {
  static bool isLocked = false;
  if (isLocked)
    return;
  isLocked = true;
  int centerX = width ()  / 2;
  int centerY = height () / 2;
  player.viewFrame ().rotateWorld ((event->x () - centerX) / 100., 0., 0., 1.);
  player.viewFrame ().rotateLocalX (-(event->y () - centerY) / 100.);
  cursor ().setPos (mapToGlobal (QPoint (centerX, centerY)));
  isLocked = false;
  updateGL ();
}

void GLWidget::mousePressEvent (QMouseEvent* event) {
  switch (event->button ()) {
    case Qt::LeftButton: {
      Vec3i headOnCube = player.getHeadOnCube ().cube + Vec3i::replicated (MAP_SIZE / 2);
      if (!cubeIsValid (headOnCube))
        break;
//       explosion (XYZ_LIST (cube), 2);
      simpleWorldMap.set (XYZ_LIST (headOnCube), BT_AIR);
      break;
    }
    case Qt::RightButton: {
      CubeWithFace headOnCube = player.getHeadOnCube ();
      if (!directionIsValid (headOnCube.face))
        break;
      headOnCube.cube += Vec3i::replicated (MAP_SIZE / 2);
      Vec3i newCube = getAdjacentCube (headOnCube).cube;
      if (!cubeIsValid (newCube))
        break;
      simpleWorldMap.set (XYZ_LIST (newCube), BT_BRICKS);
      break;
    }
    default:
      break;
  }
}

void GLWidget::timerEvent (QTimerEvent* event) {
  FIX_UNUSED (event);

  double timeElasped = m_time.elapsed () / 1000.;
  if (m_isMovingForward)
    player.moveForward (-17. * timeElasped);
  if (m_isMovingBackward)
    player.moveForward (13. * timeElasped);
  if (m_isMovingLeft)
    player.moveRight (13. * timeElasped);
  if (m_isMovingRight)
    player.moveRight (-13. * timeElasped);
  m_time.restart ();

  double fpsTimeElapsed = m_fpsTime.elapsed () / 1000.;
  if (fpsTimeElapsed > FPS_MEASURE_INTERVAL) {
    std::cout << "fps = " << m_nFramesDrawn << std::endl;
    m_nFramesDrawn = 0;
    m_fpsTime.restart ();
  }

  double physicsTimeElapsed = m_physicsTime.elapsed () / 1000.;
  if (physicsTimeElapsed > FPS_MEASURE_INTERVAL) {
    waterEngine.processWater ();
    m_physicsTime.restart ();
  }

  updateGL ();
}
