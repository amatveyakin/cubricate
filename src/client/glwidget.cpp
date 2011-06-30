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

const double FPS_MEASURE_INTERVAL = 1.; // sec



// void GLWidget::lockCubes () {
//   GLfloat* bufferPos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET,
//                                                      N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
//                                                      GL_MAP_WRITE_BIT);
//   GLfloat* bufferType = (GLfloat *) (bufferPos + 4 * N_MAX_BLOCKS_DRAWN);
//   cubeArray.setPointers (bufferPos, bufferType);
// }
//
// void GLWidget::unlockCubes () {
//   glUnmapBuffer (GL_ARRAY_BUFFER);
// }

void GLWidget::lockCubes () {
  TreeDataT* buffer = (TreeDataT *) glMapBufferRange (GL_TEXTURE_BUFFER, 0,
                                                      cubeOctree.nNodes() * sizeof (TreeNodeT),
                                                      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
  cubeOctree.setPointer (buffer);
}

void GLWidget::unlockCubes () {
  glUnmapBuffer (GL_TEXTURE_BUFFER);
  cubeOctree.restorePointer ();
}

void GLWidget::explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius) {
  lockCubes ();

  for  (int x = std::max (explosionX - explosionRadius, 0); x <= std::min (explosionX + explosionRadius, MAP_SIZE - 1); ++x)
    for  (int y = std::max (explosionY - explosionRadius, 0); y <= std::min (explosionY + explosionRadius, MAP_SIZE - 1); ++y)
      for  (int z = std::max (explosionZ - explosionRadius, 0); z <= std::min (explosionZ + explosionRadius, MAP_SIZE - 1); ++z) {
        if  (xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius)) {
          cubeOctree.set (x, y, z, 0);
        }
        else if  (   cubeOctree.get (x, y, z) != 0
                  && xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius + 1)) {
//           cubeOctree.set (x, y, z, 239);
          cubeOctree.set (x, y, z, 1);
        }
      }

  unlockCubes ();
}

void GLWidget::summonMeteorite (int meteoriteX, int meteoriteY) {
  const int METEORITE_RADIUS = 10;
  int meteoriteZ = MAP_SIZE - 1;
  while  (meteoriteZ > 0 && cubeOctree.get (meteoriteX, meteoriteY, meteoriteZ) == 0)
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
  GLfloat cubeVertices[] = { -1., -1.,  1.,    1., -1.,  1.,    1.,  1.,  1.,   -1.,  1.,  1.,    // up
                              1., -1., -1.,    1.,  1., -1.,    1.,  1.,  1.,    1., -1.,  1.,    // right
                             -1.,  1., -1.,   -1., -1., -1.,   -1., -1.,  1.,   -1.,  1.,  1.,    // left
                             -1., -1., -1.,    1., -1., -1.,    1., -1.,  1.,   -1., -1.,  1.,    // front
                              1.,  1., -1.,   -1.,  1., -1.,   -1.,  1.,  1.,    1.,  1.,  1.,    // back
                             -1., -1., -1.,   -1.,  1., -1.,    1.,  1., -1.,    1., -1., -1.  }; // down

  GLfloat cubeNormals[] =  {  0.,  0.,  1.,    0.,  0.,  1.,    0.,  0.,  1.,    0.,  0.,  1.,
                              1.,  0.,  0.,    1.,  0.,  0.,    1.,  0.,  0.,    1.,  0.,  0.,
                             -1.,  0.,  0.,   -1.,  0.,  0.,   -1.,  0.,  0.,   -1.,  0.,  0.,
                              0., -1.,  0.,    0., -1.,  0.,    0., -1.,  0.,    0., -1.,  0.,
                              0.,  1.,  0.,    0.,  1.,  0.,    0.,  1.,  0.,    0.,  1.,  0.,
                              0.,  0., -1.,    0.,  0., -1.,    0.,  0., -1.,    0.,  0., -1.  };

  GLfloat cubeTexCoords[] = {  0., 0.,   1., 0.,   1., 1.,   0., 1.,
                               0., 0.,   1., 0.,   1., 1.,   0., 1.,
                               0., 0.,   1., 0.,   1., 1.,   0., 1.,
                               0., 0.,   1., 0.,   1., 1.,   0., 1.,
                               0., 0.,   1., 0.,   1., 1.,   0., 1.,
                               0., 0.,   1., 0.,   1., 1.,   0., 1.  };

  glGenVertexArrays (1, &m_cubesVao);
  glBindVertexArray (m_cubesVao);

  glGenBuffers (1, &m_cubeVbo);
  glBindBuffer (GL_ARRAY_BUFFER, m_cubeVbo);
  glBufferData (GL_ARRAY_BUFFER,
                sizeof (cubeVertices) + sizeof (cubeNormals) + sizeof (cubeTexCoords) + N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                nullptr,
                GL_DYNAMIC_DRAW);


  GLint offset = 0;
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cubeVertices),   cubeVertices);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cubeVertices);

  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cubeNormals),    cubeNormals);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cubeNormals);

  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cubeTexCoords), cubeTexCoords);
  glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cubeTexCoords);

  m_CUBES_INFORMATION_OFFSET = offset;
  chunksForRender.init (offset);
  // TODO: It's probably better to pack each cube's data instead of doing a shift
  glVertexAttribPointer (3, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += N_MAX_BLOCKS_DRAWN * 4 * sizeof (GLfloat);

  glVertexAttribPointer (4, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  glUnmapBuffer (GL_ARRAY_BUFFER);

  glEnableVertexAttribArray (0);
  glEnableVertexAttribArray (1);
  glEnableVertexAttribArray (2);
  glEnableVertexAttribArray (3);
  glEnableVertexAttribArray (4);
  glVertexAttribDivisorARB (3, 1);
  glVertexAttribDivisorARB (4, 1);
  glBindVertexArray (0);


  glGenVertexArrays (1, &m_selectingBoxVao);
  glBindVertexArray (m_selectingBoxVao);
  glGenBuffers (1, &m_selectingBoxVbo);
  glBindBuffer (GL_ARRAY_BUFFER, m_selectingBoxVbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (cubeVertices), cubeVertices, GL_STATIC_DRAW);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);
  glEnableVertexAttribArray (0);
  glBindVertexArray (0);



  GLfloat proxySurfaceVertices[] = { 1, 1, 0,   -1, 1, 0,  -1, -1, 0,   1, -1, 0};
  GLfloat proxySurfaceDirections[] = { 1, 1, 1, -1, 1, 1,  -1, -1, 1,   1, -1, 1};

  glGenVertexArrays(1, &m_raytracingVAO);
  glBindVertexArray(m_raytracingVAO);
  glGenBuffers(1, &m_raytracingVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_raytracingVBO);
  glBufferData    (GL_ARRAY_BUFFER, sizeof (proxySurfaceVertices) + sizeof (proxySurfaceDirections), nullptr, GL_STATIC_DRAW);
  offset = 0;
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (proxySurfaceVertices)  , proxySurfaceVertices);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (proxySurfaceVertices);
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (proxySurfaceDirections), proxySurfaceDirections);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  glEnableVertexAttribArray (0);
  glEnableVertexAttribArray (1);
  glBindVertexArray (0);

}

void GLWidget::initTextures () {
  const int N_TEXTURES    = 256;
  const int TEXTURE_SIZE  = 16;

  glGenTextures (1, &m_squareTextureArray);
  glBindTexture (GL_TEXTURE_2D_ARRAY, m_squareTextureArray);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage3D (GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, N_TEXTURES, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  for (int i = 0; i < N_TEXTURES; i++) {
    char textureFileName[256];
    sprintf (textureFileName, "resources/textures/tile_%d.tga", i);

    QImage rawTexture (textureFileName);
    if (rawTexture.isNull ()) {
      std::cout << "Cannot open texture file ``" << textureFileName << "''" << std::endl;
      exit (1);
    }

    QImage texture = convertToGLFormat (rawTexture);
    assert (!texture.isNull ());

    glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texture.width (), texture.height (), 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits ());
  }

  //here we go! EPIC TEXTURE BUFFERS!
  glGenBuffers(1, &m_octTreeBuffer);
  glBindBuffer(GL_TEXTURE_BUFFER, m_octTreeBuffer);
  glBufferData(GL_TEXTURE_BUFFER, cubeOctree.nNodes () * sizeof (TreeNodeT), cubeOctree.nodes (), GL_STATIC_DRAW);
  glBindBuffer(GL_TEXTURE_BUFFER, 0);

  glGenTextures(1, &m_octTreeTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_BUFFER, m_octTreeTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, m_octTreeBuffer);
  glBindTexture(GL_TEXTURE_BUFFER, 0);

  const char *szCubeFaces[6] = { "right.tga", "left.tga", "front.tga", "back.tga", "up.tga", "down.tga" };
  const float angles[6] = {-90, 90, 0, 180, 0, 0};
  GLenum  cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                      GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                      GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                      GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                      GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                      GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


  glGenTextures(1, &m_cubeTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTexture);

  // Set up texture maps
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Load Cube Map images
  for(int i = 0; i < 6; ++i) {
    // Load this texture map
    //pBytes = gltReadTGABits(("resources/textures" + std::string(szCubeFaces[i])).c_str(), &iWidth, &iHeight, &iComponents, &eFormat);
    QImage rawTexture (("resources/textures/cubemaps/" + std::string(szCubeFaces[i])).c_str());
    if (rawTexture.isNull ()) {
      std::cout << "Cannot open texture file !!!" << std::endl;
      exit (1);
    }
    QMatrix rotateMatrix;
    rotateMatrix.rotate (angles[i]);
    rawTexture =  rawTexture.transformed(rotateMatrix);
    QImage texture = convertToGLFormat (rawTexture);
    assert (!texture.isNull ());
    glTexImage2D(cube[i], 0, GL_RGBA8, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
  }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

// TODO: rename shader program files
void GLWidget::initShaders () {
  if (!QGLShaderProgram::hasOpenGLShaderPrograms()) {
    std::cout << "Your system does not support OpenGL custom shader programs :-(" << std::endl;
    exit (1);
  }

//   QGLShader vertexShader   (QGLShader::Vertex);
//   QGLShader fragmentShader (QGLShader::Fragment);
//
//   bool result;
//
//   result = vertexShader.compileSourceFile ("resources/InstancedCube.vp");
//   if (!result) {
//     std::cout << "Unable to compile vertex shader:" << std::endl
//               << vertexShader.log ().toStdString () << std::endl;
//     exit (1);
//   }
//
//   result = fragmentShader.compileSourceFile ("resources/InstancedCube.fp");
//   if (!result) {
//     std::cout << "Unable to compile fragment shader:" << std::endl
//               << fragmentShader.log ().toStdString () << std::endl;
//     exit (1);
//   }

  bool result;

  result = m_shaderProgram.addShaderFromSourceFile (QGLShader::Vertex, "resources/InstancedCube.vp");
  if (!result) {
    std::cout << "Unable to compile vertex shader:" << std::endl
              << m_shaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }

  result = m_shaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/InstancedCube.fp");
  if (!result) {
    std::cout << "Unable to compile fragment shader:" << std::endl
              << m_shaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }



  m_shaderProgram.bindAttributeLocation ("v_vertex", 0);
  m_shaderProgram.bindAttributeLocation ("v_normal", 1);
  m_shaderProgram.bindAttributeLocation ("v_tex_coord", 2);
  m_shaderProgram.bindAttributeLocation ("v_pos_and_size", 3);
  m_shaderProgram.bindAttributeLocation ("v_type", 4);

  result = m_shaderProgram.link ();
  if (!result) {
    std::cout << "Unable to link shaders:" << std::endl
              << m_shaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }

  m_instancedCubeShader = m_shaderProgram.programId ();
  glLinkProgram (m_instancedCubeShader);
  glUseProgram (m_instancedCubeShader);
  m_locInstancedCubeMvp            = glGetUniformLocation (m_instancedCubeShader, "mvp_matrix");
  m_locInstancedCubeMapSize        = glGetUniformLocation (m_instancedCubeShader, "MAP_SIZE");
  m_locInstancedCubeSquareTexture  = glGetUniformLocation (m_instancedCubeShader, "squareTexture");

  glUniform1i (m_locInstancedCubeMapSize, MAP_SIZE);


  result = m_basicShaderProgram.addShaderFromSourceFile (QGLShader::Vertex, "resources/BasicShader.vp");
  if (!result) {
    std::cout << "Unable to compile vertex shader:" << std::endl
              << m_basicShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }

  result = m_basicShaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/BasicShader.fp");
  if (!result) {
    std::cout << "Unable to compile fragment shader:" << std::endl
              << m_basicShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }

  m_basicShaderProgram.bindAttributeLocation ("vPosition", 0);
  //m_basicShaderProgram.bindAttributeLocation ("vColor", 1);

  result = m_basicShaderProgram.link ();
  if (!result) {
    std::cout << "Unable to link shaders:" << std::endl
              << m_basicShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }
  m_locBasicShaderWVP = 0;
  m_basicShader = m_basicShaderProgram.programId ();
  glLinkProgram (m_basicShader);
  glUseProgram (m_basicShader);
  m_locBasicShaderWVP   = glGetUniformLocation (m_basicShader, "wvpMatrix");
  //m_locBasicShaderColor = glGetUniformLocation (m_basicShader, "color");

  //Raytracing shader initialization
  result = m_raytracingShaderProgram.addShaderFromSourceFile (QGLShader::Vertex, "resources/RaytracingShader.vp");
  if (!result) {
//     std::cout << "Unable to compile vertex shader:" << std::endl
//               << m_raytracingShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }
  result = m_raytracingShaderProgram.addShaderFromSourceFile (QGLShader::Fragment, "resources/RaytracingShader.fp");
  if (!result) {
//     std::cout << "Unable to compile fragment shader:" << std::endl
//               << m_raytracingShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }
  m_raytracingShaderProgram.bindAttributeLocation ("vPosition",  0);
  m_raytracingShaderProgram.bindAttributeLocation ("vDirection", 1);
  result = m_raytracingShaderProgram.link ();
  if (!result) {
//     std::cout << "Unable to link shaders:" << std::endl
//               << m_basicShaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }
  m_raytracingShader = m_raytracingShaderProgram.programId ();
  glLinkProgram (m_raytracingShader);
  glUseProgram (m_raytracingShader);
  m_locOctTree  =       glGetUniformLocation (m_raytracingShader, "octTree");
  m_locOrigin   =       glGetUniformLocation (m_raytracingShader, "origin");
  m_locRaytracingView = glGetUniformLocation (m_raytracingShader, "matView");
  m_locCubeTexture    = glGetUniformLocation (m_raytracingShader, "cubeTexture");
}

void GLWidget::setupRenderContext () {
  glClearColor  (0.0f, 0.0f, 0.0f, 1.0f);

  initShaders ();
  initBuffers ();
  initTextures ();
}

void GLWidget::shutdownRenderContext () {
  glDeleteProgram (m_instancedCubeShader);
  glDeleteBuffers (1, &m_cubeVbo);
  glDeleteVertexArrays (1, &m_cubesVao);
}







// 0 means success
int loadGameMap () {
  std::ifstream heightMap ("resources/height_map" + toStr (TREE_HEIGHT) + ".txt");
  if (!heightMap.is_open ()) {
    std::cout << "Unable to open height map!\n";
    return 1;
  }
  for (int x = 0; x < MAP_SIZE; ++x) {
    for (int y = 0; y < MAP_SIZE; ++y) {
      int height;
      heightMap >> height;
//       if (height > MAP_SIZE / 2) {
// //         cubeArray.addCube (x, y, height - 1, 66);
//         cubeOctree.set (x, y, height - 1, 1);
//         height--;
//       }
      for (int z = 0; z < height; ++z) {
//         cubeArray.addCube (x, y, z, 2);
        cubeOctree.set (x, y, z, 1);
      }
//       for (int z = height; z < MAP_SIZE / 2; ++z) {
//         cubeOctree.set (x, y, z, 2);
//       }
    }
  }

  cubeOctree.computeNeighbours ();

  // testing map
  for (int x = 0; x < MAP_SIZE; ++x) {
    for (int y = 0; y < MAP_SIZE; ++y) {
      int state = 1;
      for (int z = 0; z < MAP_SIZE; ++z) {
        switch (state) {
          case 1:  // dirt
            switch (int (cubeOctree.get (x, y, z))) {
              case 1:  // dirt
                break;
              case 2:  // water
                state = 2;
                break;
              case 0:  // air
                state = 0;
                break;
            }
            break;
          case 2:  // water
            switch (int (cubeOctree.get (x, y, z))) {
              case 1:  // dirt
                abort ();
                break;
              case 2:  // water
                break;
              case 0:  // air
                state = 0;
                break;
            }
            break;
          case 0:  // air
            switch (int (cubeOctree.get (x, y, z))) {
              case 1:  // dirt
                abort ();
                break;
              case 2:  // water
                abort ();
                break;
              case 0:  // air
                state = 0;
                break;
            }
            break;
        }
      }
    }
  }

  int MAX_NODE_VALUE = 256;
  const TreeDataT* nodes = cubeOctree.nodes();
  int nNodeValues[MAX_NODE_VALUE];
  std::fill (nNodeValues, nNodeValues + MAX_NODE_VALUE, 0);
  for (int i = 0; i < cubeOctree.nNodes(); ++i) {
    std::cout.width (3);
    std::cout << i << ": ";
    for (int j = 0; j < NODE_STRUCT_SIZE; ++j) {
      std::cout.width (3);
      int value = nodes[i * NODE_STRUCT_SIZE + j];
      if (value == -1)
        std::cout << "." << " ";
      else {
        assert (value >= 0);
        std::cout << value << " ";
      }
    }
    std::cout << std::endl;
    int nodeValue = nodes[i * NODE_STRUCT_SIZE];
    assert (nodeValue >= 0);
    assert (nodeValue < MAX_NODE_VALUE);
    nNodeValues [nodeValue]++;
  }
  std::cout << std::endl;

  std::cout << "Cube type frequency:" << std::endl;
  for (int i = 0;i < MAX_NODE_VALUE; ++i)
    if (nNodeValues[i] != 0)
      std::cout << i << ": " << nNodeValues[i] << std::endl;
  std::cout << std::endl;

  std::cout << "nOctreeNodes = " << cubeOctree.nNodes () << std::endl;
  std::cout << std::endl;

  //   std::cout << "nCubes = " << cubeArray.nCubes () << std::endl;
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

  loadGameMap ();
  player.setPos (Vec3d (0., 0., MAP_SIZE / 8.));

  setupRenderContext ();

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
  startTimer (1);
}

void GLWidget::paintGL () {
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  //glCullFace (GL_BACK);
  glDisable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);
  //glEnable (GL_MULTISAMPLE);
  M3DMatrix44f matView;
  glUseProgram (m_raytracingShader);

  //const GLfloat origin[] = {50, 50 , 0};
  glUniform1i  (m_locOctTree, 0);
  glUniform1i  (m_locCubeTexture, 1);
  glUniform3fv (m_locOrigin, 1, Vec3f::fromVectorConverted(player.pos()).data());
  player.viewFrame().getCameraMatrix (matView, true);
  glUniformMatrix4fv (m_locRaytracingView, 1, GL_TRUE, matView);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_BUFFER, m_octTreeTexture);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (GL_TEXTURE_CUBE_MAP, m_cubeTexture);

  glBindVertexArray (m_raytracingVAO);
  glDrawArrays (GL_QUADS, 0, 4);



// FUCK ALL THIS SHIT, RAYTRACING FTW
//   M3DMatrix44f mat_View, mat_VP, mat_World, mat_WVP, currentTransform, currentResult;
//   player.viewFrame ().getCameraMatrix (mat_View, false);
//   m3dMatrixMultiply44 (mat_VP, m_viewFrustum.GetProjectionMatrix (), mat_View);
//
//   glUseProgram (m_instancedCubeShader);
//   glBindVertexArray (m_cubesVao);
//
// //   GLfloat m_rotateCameraAlpha[16], m_rotateCameraBeta[16];
// //   m3dRotationMatrix44 (m_rotateCameraAlpha, m_cameraAlpha, 1., 0., 0.);
// //   m3dRotationMatrix44 (m_rotateCameraBeta,  m_cameraBeta,  0., 1., 0.);
//   m3dTranslationMatrix44 (mat_World, -MAP_SIZE / 2., -MAP_SIZE / 2., -MAP_SIZE / 2.);
//   m3dMatrixMultiply44 (mat_WVP, mat_VP, mat_World);
//   glUniformMatrix4fv (m_locInstancedCubeMvp, 1, GL_FALSE, mat_WVP);
//
//   glBindTexture (GL_TEXTURE_2D, m_squareTextureArray);
//   glUniform1i (m_locInstancedCubeSquareTexture, 0);
//   glDrawArraysInstancedARB (GL_QUADS, 0, 24, cubeArray.nCubes ());
//
//   glBindVertexArray (0);
//
//   //are we need some useless checks?
//   //yes we fucking are =(
//   CubeWithFace headOnCube = player.getHeadOnCube();
//   float distance = L2::distance(player.pos(), Vec3d::fromVectorConverted (headOnCube.cube));
//   if ( directionIsValid (headOnCube.face) ) {
//
//     const float   SELECTING_BOX_THICKNESS = 0.125;
//     const GLfloat SELECTING_BOX_COLOR[]   = {0.0f, 0.0f, 0.0f, 1.0f};
//     Vec3f selectedCube = Vec3f::fromVectorConverted (getAdjacentCube (headOnCube).cube);
//     Vec3f direction    = selectedCube - Vec3f::fromVectorConverted (headOnCube.cube);
//
//     m3dScaleMatrix44 (mat_World, 5 * (1 - xAbs(direction.x ()) * (1 - SELECTING_BOX_THICKNESS)),
//                                  5 * (1 - xAbs(direction.y ()) * (1 - SELECTING_BOX_THICKNESS)),
//                                  5 * (1 - xAbs(direction.z ()) * (1 - SELECTING_BOX_THICKNESS))); //some shitty magic
//     Vec3f pos = selectedCube - direction * ((1 - SELECTING_BOX_THICKNESS) / 2.);
//     m3dTranslationMatrix44 (currentTransform, pos.x (), pos.y (), pos.z ());
//     m3dMatrixMultiply44 (currentResult, currentTransform, mat_World);
//     m3dCopyMatrix44(mat_World, currentResult);
//     //m3dScaleMatrix44(mat_World, 1, 1, 1);
//     m3dMatrixMultiply44 (mat_WVP, mat_VP, mat_World);
//
//     glUseProgram (m_basicShader);
//     glBindVertexArray (m_selectingBoxVao);
//     glDisable(GL_CULL_FACE);
//     glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
//     glLineWidth(6. / distance);
//
//     glUniform4fv (m_locBasicShaderColor, 1, SELECTING_BOX_COLOR);
//     glUniformMatrix4fv (m_locBasicShaderWVP, 1, GL_FALSE, mat_WVP);
//
//     glDrawArrays(GL_QUADS, 0, 24);
//
//     glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
//     glBindVertexArray (0);
//   }
  m_nFramesDrawn++;
}

void GLWidget::resizeGL (int width, int height) {
  if (height <= 0)
    height = 1;

  glViewport (0, 0, width, height);
  m_viewFrustum.SetPerspective (90.0f, float (width) / float (height), 0.1f, MAP_SIZE * 10.);

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
  player.viewFrame ().RotateWorld ((event->x () - centerX) / 100., 0., 0., 1.);
  player.viewFrame ().RotateLocalX (-(event->y () - centerY) / 100.);
  cursor ().setPos (mapToGlobal (QPoint (centerX, centerY)));
  isLocked = false;
  updateGL ();
}

void GLWidget::mousePressEvent (QMouseEvent* event) {
  switch (event->button ()) {
    case Qt::LeftButton: {
      Vec3i headOnCube = player.getHeadOnCube ().cube + Vec3i::replicated (MAP_SIZE / 2);
      if (!cubeValid (headOnCube))
        break;
//       explosion (XYZ_LIST (cube), 2);
      lockCubes ();
//       cubeArray.removeCube (XYZ_LIST (headOnCube));
      cubeOctree.set (XYZ_LIST (headOnCube), 0);
      unlockCubes ();
      break;
    }
    case Qt::RightButton: {
      CubeWithFace headOnCube = player.getHeadOnCube ();
      if (!directionIsValid (headOnCube.face))
        break;
      headOnCube.cube += Vec3i::replicated (MAP_SIZE / 2);
      Vec3i newCube = getAdjacentCube (headOnCube).cube;
      if (!cubeValid (newCube))
        break;
      lockCubes ();
//       cubeArray.addCube (XYZ_LIST (newCube), 7);
      cubeOctree.set (XYZ_LIST (newCube), 1);
      unlockCubes ();
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
  updateGL ();
}
