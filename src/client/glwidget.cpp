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

#include "client/cube_array.hpp"
#include "client/client_world.hpp"
#include "client/glwidget.hpp"



const int N_MAX_BLOCKS_DRAWN = N_MAP_BLOCKS;

const double FPS_MEASURE_INTERVAL = 1.; /* sec */



void GLWidget::lockCubes () {
  GLfloat* buffer_pos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET,
                                                      N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                                                      GL_MAP_WRITE_BIT);
  GLfloat* buffer_type = (GLfloat *) (buffer_pos + 4 * N_MAX_BLOCKS_DRAWN);
  cubeArray.set_pointers (buffer_pos, buffer_type);
}

void GLWidget::unlockCubes () {
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

void GLWidget::explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius) {
  lockCubes ();

  for  (int x = std::max (explosionX - explosionRadius, 0); x <= std::min (explosionX + explosionRadius, MAP_SIZE - 1); ++x)
    for  (int y = std::max (explosionY - explosionRadius, 0); y <= std::min (explosionY + explosionRadius, MAP_SIZE - 1); ++y)
      for  (int z = std::max (explosionZ - explosionRadius, 0); z <= std::min (explosionZ + explosionRadius, MAP_SIZE - 1); ++z) {
        if  (xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius)) {
          cubeArray.remove_cube (x, y, z);
        }
        else if  (   cubeArray.cube_presents (x, y, z)
                  && xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius + 1)) {
          cubeArray.add_cube (x, y, z, 239);
        }
      }

  unlockCubes ();
}

void GLWidget::summonMeteorite (int meteoriteX, int meteoriteY) {
  const int METEORITE_RADIUS = 10;
  int meteoriteZ = MAP_SIZE - 1;
  while  (meteoriteZ > 0 && !cubeArray.cube_presents (meteoriteX, meteoriteY, meteoriteZ))
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
  GLfloat cube_vertices[] = { -1., -1.,  1.,    1., -1.,  1.,    1.,  1.,  1.,   -1.,  1.,  1.,    // up
                               1., -1., -1.,    1.,  1., -1.,    1.,  1.,  1.,    1., -1.,  1.,    // right
                              -1.,  1., -1.,   -1., -1., -1.,   -1., -1.,  1.,   -1.,  1.,  1.,    // left
                              -1., -1., -1.,    1., -1., -1.,    1., -1.,  1.,   -1., -1.,  1.,    // front
                               1.,  1., -1.,   -1.,  1., -1.,   -1.,  1.,  1.,    1.,  1.,  1.,    // back
                              -1., -1., -1.,   -1.,  1., -1.,    1.,  1., -1.,    1., -1., -1.  }; // down

  GLfloat cube_normals[] =  {  0.,  0.,  1.,    0.,  0.,  1.,    0.,  0.,  1.,    0.,  0.,  1.,
                               1.,  0.,  0.,    1.,  0.,  0.,    1.,  0.,  0.,    1.,  0.,  0.,
                              -1.,  0.,  0.,   -1.,  0.,  0.,   -1.,  0.,  0.,   -1.,  0.,  0.,
                               0., -1.,  0.,    0., -1.,  0.,    0., -1.,  0.,    0., -1.,  0.,
                               0.,  1.,  0.,    0.,  1.,  0.,    0.,  1.,  0.,    0.,  1.,  0.,
                               0.,  0., -1.,    0.,  0., -1.,    0.,  0., -1.,    0.,  0., -1.  };

  GLfloat cube_tex_coords[] = {  0., 0.,   1., 0.,   1., 1.,   0., 1.,
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
                sizeof (cube_vertices) + sizeof (cube_normals) + sizeof (cube_tex_coords) + N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                nullptr,
                GL_DYNAMIC_DRAW);


  GLint offset = 0;
  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cube_vertices),   cube_vertices);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cube_vertices);

  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cube_normals),    cube_normals);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cube_normals);

  glBufferSubData (GL_ARRAY_BUFFER, offset, sizeof (cube_tex_coords), cube_tex_coords);
  glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) offset);
  offset += sizeof (cube_tex_coords);

  m_CUBES_INFORMATION_OFFSET = offset;
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

  result = m_shaderProgram.link ();
  if (!result) {
    std::cout << "Unable to link shaders:" << std::endl
              << m_shaderProgram.log ().toStdString () << std::endl;
    exit (1);
  }

  m_shaderProgram.bindAttributeLocation ("v_vertex", 0);
  m_shaderProgram.bindAttributeLocation ("v_normal", 1);
  m_shaderProgram.bindAttributeLocation ("v_tex_coord", 2);
  m_shaderProgram.bindAttributeLocation ("v_pos_and_size", 3);
  m_shaderProgram.bindAttributeLocation ("v_type", 4);

  m_instancedCubeShader = m_shaderProgram.programId ();
//   glLinkProgram (m_instancedCubeShader);
  glUseProgram (m_instancedCubeShader);
  m_locMvp            = glGetUniformLocation (m_instancedCubeShader, "mvp_matrix");
  m_locColor          = glGetUniformLocation (m_instancedCubeShader, "color");
  m_locMapSize        = glGetUniformLocation (m_instancedCubeShader, "MAP_SIZE");
  m_locSquareTexture  = glGetUniformLocation (m_instancedCubeShader, "squareTexture");

  glUniform1i (m_locMapSize, MAP_SIZE);
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
int loadGameMap (Visible_cube_set <GLfloat, GLfloat>& cubeArray) {
  std::ifstream height_map ("resources/height_map" + toStr (TREE_HEIGHT) + ".txt");
  if (!height_map.is_open ()) {
    std::cout << "Unable to open height map!\n";
    return 1;
  }
  for (int x = 0; x < MAP_SIZE; ++x) {
    for (int y = 0; y < MAP_SIZE; ++y) {
      int height;
      height_map >> height;
      if (height > MAP_SIZE / 2) {
        cubeArray.add_cube (x, y, height - 1, 66);
        height--;
      }
      for (int z = 0; z < height; ++z) {
        cubeArray.add_cube (x, y, z, 2);
      }
    }
  }
  std::cout << "n_cubes = " << cubeArray.n_cubes () << std::endl;
  return 0;
}

/*
// 0 means success
int loadGameMap (Visible_cube_set <GLfloat, GLfloat>& cubeArray) {
  std::ifstream map ("resources/World1.schematic");
  if (!map.is_open ()) {
    std::cout << "Unable to open map!\n";
    return 1;
  }
  for (int z = 0; z < 128; ++z) {
    for (int y = 0; y < 80; ++y) {
      for (int x = 0; x < 80; ++x) {
        char cur_block;
        map >> cur_block;
        map >> cur_block;
        if (x < MAP_SIZE && y < MAP_SIZE && z < MAP_SIZE)
          cubeArray.add_cube (x, y, z, cur_block);
      }
    }
  }
  std::cout << "n_cubes = " << cubeArray.n_cubes () << std::endl;
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

  setupRenderContext ();


  GLfloat* buffer_pos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET, N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
  GLfloat* buffer_type = (GLfloat *) (buffer_pos + 4 * N_MAX_BLOCKS_DRAWN);

  cubeArray.set_pointers (buffer_pos, buffer_type);
  loadGameMap (cubeArray);

  glUnmapBuffer (GL_ARRAY_BUFFER);

  m_time.start ();
  m_fpsTime.start ();
  startTimer (1);
}

void GLWidget::paintGL () {
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_MULTISAMPLE);

  glUseProgram (m_instancedCubeShader);
  glBindVertexArray (m_cubesVao);

//   GLfloat m_rotate_cameraAlpha[16], m_rotate_cameraBeta[16];
//   m3dRotationMatrix44 (m_rotate_cameraAlpha, m_cameraAlpha, 1., 0., 0.);
//   m3dRotationMatrix44 (m_rotate_cameraBeta,  m_cameraBeta,  0., 1., 0.);
  M3DMatrix44f mat_View, mat_VP, mat_World, mat_WVP;
  m3dTranslationMatrix44 (mat_World, -MAP_SIZE / 2., -MAP_SIZE / 2., -MAP_SIZE / 2.);
  player.viewFrame ().getCameraMatrix (mat_View, false);
  m3dMatrixMultiply44 (mat_VP, m_viewFrustum.GetProjectionMatrix (), mat_View);
  m3dMatrixMultiply44 (mat_WVP, mat_VP, mat_World);
  glUniformMatrix4fv (m_locMvp, 1, GL_FALSE, mat_WVP);

  glBindTexture (GL_TEXTURE_2D, m_squareTextureArray);
  glUniform1i (m_locSquareTexture, 0);
  GLfloat color[] = {1.0f, 1.0f, 0.0f, 0.0f};
  glUniform4fv (m_locColor, 1, color);
  glDrawArraysInstancedARB (GL_QUADS, 0, 24, cubeArray.n_cubes ());

  glBindVertexArray (0);

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
  player.viewFrame ().RotateWorld (-(event->x () - centerX) / 100., 0., 0., 1.);
  player.viewFrame ().RotateLocalX ((event->y () - centerY) / 100.);
  cursor ().setPos (mapToGlobal (QPoint (centerX, centerY)));
  isLocked = false;
  updateGL ();
}

void GLWidget::mousePressEvent (QMouseEvent* event) {
  switch (event->button ()) {
    case Qt::LeftButton: {
      CubeWithFace headOnCube = player.getHeadOnCube ();
      if (!directionIsValid (headOnCube.face))
        break;
      headOnCube.cube += Vec3i::replicatedValuesVector (MAP_SIZE / 2);
      Vec3i newCube = getAdjacentCube (headOnCube).cube;
      if (!cubeValid (newCube))
        break;
      lockCubes ();
      cubeArray.add_cube (XYZ_LIST (newCube), 7);
      unlockCubes ();
      break;
    }
    case Qt::RightButton: {
      Vec3i headOnCube = player.getHeadOnCube ().cube + Vec3i::replicatedValuesVector (MAP_SIZE / 2);
      if (!cubeValid (headOnCube))
        break;
//       explosion (XYZ_LIST (cube), 2);
      lockCubes ();
      cubeArray.remove_cube (XYZ_LIST (headOnCube));
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
    player.moveForward (5. * timeElasped);
  if (m_isMovingBackward)
    player.moveForward (-3. * timeElasped);
  if (m_isMovingLeft)
    player.moveRight (3. * timeElasped);
  if (m_isMovingRight)
    player.moveRight (-3. * timeElasped);
  m_time.restart ();

  double fpsTimeElapsed = m_fpsTime.elapsed () / 1000.;
  if (fpsTimeElapsed > FPS_MEASURE_INTERVAL) {
    std::cout << "fps = " << m_nFramesDrawn << std::endl;
    m_nFramesDrawn = 0;
    m_fpsTime.restart ();
  }
  updateGL ();
}
