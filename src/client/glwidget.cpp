#include <cmath>
#include <cassert>
#include <iostream>   // TODO: delete
#include <fstream>    // TODO: delete

#include <QTime>
#include <QKeyEvent>

#include "common/utils.hpp"
#include "common/math_utils.hpp"
#include "common/string_utils.hpp"
#include "common/game_parameters.hpp"

#include "client/cube_array.hpp"

#include "glwidget.hpp"



const int N_MAX_BLOCKS_DRAWN = N_MAP_BLOCKS;

const GLfloat MIN_CAMERA_DISTANCE = 0.1;

const double FPS_MEASURE_INTERVAL = 1.; /* sec */



void GLWidget::lockCubes () {
  GLfloat* buffer_pos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET,
                                                      N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                                                      GL_MAP_WRITE_BIT);
  GLfloat* buffer_type = (GLfloat *) (buffer_pos + 4 * N_MAX_BLOCKS_DRAWN);
  m_cubeArray.set_pointers (buffer_pos, buffer_type);
}

void GLWidget::unlockCubes () {
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

bool GLWidget::coordinatesValid (int x, int y, int z) {
  return    (x >= 0) && (x < MAP_SIZE)
         && (y >= 0) && (y < MAP_SIZE)
         && (z >= 0) && (z < MAP_SIZE);
}

void GLWidget::getCubeByPoint (M3DVector3i cube, M3DVector3f point, M3DVector3f direction) {
  const float EPSILON = 0.00001;
  for (int i = 0; i < 3; ++i)
    cube[i] = (int) floor (point[i] + xSgn (direction[i]) * EPSILON + 0.5);
}

void GLWidget::lookAt (M3DVector3f result) {
  M3DVector3f currentPoint;
  M3DVector3f forwardVector;
  M3DVector3f parameter;
  M3DVector3f nearestInt;

  m_viewFrame.GetOrigin (currentPoint);
  m_viewFrame.GetForwardVector (forwardVector);

  M3DVector3i cube;
  getCubeByPoint (cube, currentPoint, forwardVector);
  while  (  coordinatesValid (cube[0] + MAP_SIZE / 2, cube[1] + MAP_SIZE / 2, cube[2] + MAP_SIZE / 2)
         && !m_cubeArray.cube_presents (cube[0] + MAP_SIZE / 2, cube[1] + MAP_SIZE / 2, cube[2] + MAP_SIZE / 2)) {
    for (int i = 0; i < 3; ++i) {
      nearestInt[i] = (forwardVector[i] > 0) ? (floor (currentPoint[i] - 0.5) + 1.5) : (ceil (currentPoint[i] + 0.5) - 1.5);
      parameter[i] = (nearestInt[i] - currentPoint[i]) / forwardVector[i];
      assert (parameter[i] >= 0);
    }
    float t = xMin (parameter[0], parameter[1], parameter[2]);
    assert (t >= 0);

    for (int i = 0; i < 3; ++i)
      currentPoint[i] += forwardVector[i] * t;

    getCubeByPoint (cube, currentPoint, forwardVector);
  }
  m3dCopyVector3 (result, currentPoint);
}

void GLWidget::explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius) {
  lockCubes ();

  for  (int x = std::max (explosionX - explosionRadius, 0); x <= std::min (explosionX + explosionRadius, MAP_SIZE - 1); ++x)
    for  (int y = std::max (explosionY - explosionRadius, 0); y <= std::min (explosionY + explosionRadius, MAP_SIZE - 1); ++y)
      for  (int z = std::max (explosionZ - explosionRadius, 0); z <= std::min (explosionZ + explosionRadius, MAP_SIZE - 1); ++z) {
        if  (xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius)) {
          m_cubeArray.remove_cube (x, y, z);
        }
        else if  (   m_cubeArray.cube_presents (x, y, z)
                  && xSqr (x - explosionX) + xSqr (y - explosionY) + xSqr (z - explosionZ) < xSqr (explosionRadius + 1)) {
          m_cubeArray.add_cube (x, y, z, 239);
        }
      }

  unlockCubes ();
}

void GLWidget::summonMeteorite (int meteoriteX, int meteoriteY) {
  const int METEORITE_RADIUS = 10;
  int meteoriteZ = MAP_SIZE - 1;
  while  (meteoriteZ > 0 && !m_cubeArray.cube_presents (meteoriteX, meteoriteY, meteoriteZ))
    meteoriteZ--;
  explosion (meteoriteX, meteoriteY, meteoriteZ, METEORITE_RADIUS);
}

/*
bool GLWidget::loadTGATexture (const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
  GLbyte *pBits;
  int nWidth, nHeight, nComponents;
  GLenum eFormat;

  // Read the texture bits
  pBits = gltReadTGABits (szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
  if (!pBits)
    return false;

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D (GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
                eFormat, GL_UNSIGNED_BYTE, pBits);

  free (pBits);

  if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
      minFilter == GL_LINEAR_MIPMAP_NEAREST ||
      minFilter == GL_NEAREST_MIPMAP_LINEAR ||
      minFilter == GL_NEAREST_MIPMAP_NEAREST)
    glGenerateMipmap (GL_TEXTURE_2D);

  return true;
}
*/

//           up
//                  back
//          +------+                      6+------+7
//         /|     /|                      /|     /|
//        +------+ |                    4+------+5|
//  left  | |    | |  right              | |    | |
//        | +----|-+                     |2+----|-+3
//        |/     |/                      |/     |/
//        +------+                      0+------+1
//    front
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
  glGenTextures (1, &m_squareTextureArray);
  glBindTexture (GL_TEXTURE_2D_ARRAY, m_squareTextureArray);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage3D (GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 16, 16, 256, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  for (int i = 0; i < 256; i++) {
    char cFile[32];
    sprintf (cFile, "resources/textures/tile_%d.tga", i);

    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;

    // Read the texture bits
    pBits = gltReadTGABits (cFile, &nWidth, &nHeight, &nComponents, &eFormat);
    assert (pBits);
    glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, nWidth, nHeight, 1, GL_BGRA, GL_UNSIGNED_BYTE, pBits);

    free (pBits);
  }
}

void GLWidget::initShaders () {
  m_instancedCubeShader = gltLoadShaderPairWithAttributes ("resources/InstancedCube.vp", "resources/InstancedCube.fp", 5,
                                                           0, "v_vertex",
                                                           1, "v_normal",
                                                           2, "v_tex_coord",
                                                           3, "v_pos_and_size",
                                                           4, "v_type");

  glLinkProgram (m_instancedCubeShader);
  glUseProgram (m_instancedCubeShader);
  m_locMvp      = glGetUniformLocation (m_instancedCubeShader, "mvp_matrix");
  m_locColor    = glGetUniformLocation (m_instancedCubeShader, "color");
  //m_locTexture  = glGetUniformLocation (m_instancedCubeShader, "colorMap");
  m_locMapSize = glGetUniformLocation (m_instancedCubeShader, "MAP_SIZE");
  m_locSquareTexture = glGetUniformLocation (m_instancedCubeShader, "squareTexture");

  glUniform1i (m_locMapSize, MAP_SIZE);
}

void GLWidget::updateCamera () {
  M3DVector3f cameraPosition, cameraUp;

  cameraPosition[0] = m_cameraDistance * cos (m_cameraBeta) * sin (m_cameraAlpha);
  cameraPosition[1] = m_cameraDistance * cos (m_cameraBeta) * cos (m_cameraAlpha);
  cameraPosition[2] = m_cameraDistance * sin (m_cameraBeta);

  cameraUp[0] = -sin (m_cameraBeta) * sin (m_cameraAlpha);
  cameraUp[1] = -sin (m_cameraBeta) * cos (m_cameraAlpha);
  cameraUp[2] =  cos (m_cameraBeta);

  m_viewFrame.SetOrigin   (cameraPosition);
  m_viewFrame.SetUpVector (cameraUp);

  m3dNormalizeVector3 (cameraPosition);
  m3dScaleVector3 (cameraPosition, -1);
  m_viewFrame.SetForwardVector (cameraPosition);
}

void GLWidget::setupRenderContext () {
  glClearColor  (0.0f, 0.0f, 0.0f, 1.0f);

  initShaders ();
  initBuffers ();
  initTextures ();
  updateCamera ();

}

// TODO: Where should we call it?
void GLWidget::shutdownRenderContext () {
  glDeleteProgram (m_instancedCubeShader);
  glDeleteBuffers (1, &m_cubeVbo);
  glDeleteVertexArrays (1, &m_cubesVao);
}







// 0 means success
int loadGameMap (Visible_cube_set <GLfloat, GLfloat>& cubeArray) {
  std::ifstream height_map ("resources/height_map" + stringify (TREE_HEIGHT) + ".txt");
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




GLWidget::GLWidget () :
  m_cubeArray (MAP_SIZE, MAP_SIZE, MAP_SIZE)
{
  setMouseTracking (true);
  setCursor (Qt::BlankCursor);

  isMovingForward   = false;
  isMovingBackward  = false;
  isMovingLeft      = false;
  isMovingRight     = false;
}

GLWidget::~GLWidget () { }

void GLWidget::initializeGL () {
  m_nFramesDrawn = 0;

  m_cameraAlpha = 0.;
  m_cameraBeta = 0.;
  m_cameraDistance = MIN_CAMERA_DISTANCE;

  GLenum err = glewInit ();
  if (GLEW_OK != err) {
    std::cout << "GLEW Error: " << glewGetErrorString (err) << std::endl;
    exit (1);
  }

  setupRenderContext ();


  GLfloat* buffer_pos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET, N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
  GLfloat* buffer_type = (GLfloat *) (buffer_pos + 4 * N_MAX_BLOCKS_DRAWN);

  m_cubeArray.set_pointers (buffer_pos, buffer_type);
  loadGameMap (m_cubeArray);

  glUnmapBuffer (GL_ARRAY_BUFFER);

  time.start ();
  fpsTime.start ();
  startTimer (1);
}

void GLWidget::paintGL () {
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_MULTISAMPLE);

  glUseProgram  (m_instancedCubeShader);
  glBindVertexArray (m_cubesVao);

  GLfloat m_rotate_cameraAlpha[16], m_rotate_cameraBeta[16];
  m3dRotationMatrix44 (m_rotate_cameraAlpha, m_cameraAlpha, 1., 0., 0.);
  m3dRotationMatrix44 (m_rotate_cameraBeta,  m_cameraBeta,  0., 1., 0.);
  M3DMatrix44f mat_View, mat_VP, mat_World, mat_WVP;
  m3dTranslationMatrix44 (mat_World, -MAP_SIZE / 2., -MAP_SIZE / 2., -MAP_SIZE / 2.);
  m_viewFrame.GetCameraMatrix (mat_View, false);
  m3dMatrixMultiply44 (mat_VP, m_viewFrustum.GetProjectionMatrix (), mat_View);
  m3dMatrixMultiply44 (mat_WVP, mat_VP, mat_World);
  glUniformMatrix4fv (m_locMvp, 1, GL_FALSE, mat_WVP);

  glBindTexture (GL_TEXTURE_2D, m_squareTextureArray);
  glUniform1i (m_locSquareTexture, 0);
  GLfloat color[] = {1.0f, 1.0f, 0.0f, 0.0f};
  glUniform4fv (m_locColor, 1, color);
  glDrawArraysInstancedARB (GL_QUADS, 0, 24, m_cubeArray.n_cubes ());

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
      isMovingForward = true;
      break;
    case Qt::Key_S:
      isMovingBackward = true;
      break;
    case Qt::Key_A:
      isMovingLeft = true;
      break;
    case Qt::Key_D:
      isMovingRight = true;
      break;
    case Qt::Key_X:
      M3DVector3f vOrigin;
      m_viewFrame.GetOrigin (vOrigin);
      summonMeteorite ((int) (vOrigin[0] + MAP_SIZE / 2.), (int) (vOrigin[1] + MAP_SIZE / 2.));
      break;
    case Qt::Key_B:
      M3DVector3f lookingAt;
      M3DVector3f forward;
      M3DVector3i cube;
      lookAt (lookingAt);
      m_viewFrame.GetForwardVector (forward);
      getCubeByPoint (cube, lookingAt, forward);
      if (coordinatesValid (cube[0] + MAP_SIZE / 2, cube[1] + MAP_SIZE / 2, cube[2] + MAP_SIZE / 2))
        explosion (cube[0] + MAP_SIZE / 2, cube[1] + MAP_SIZE / 2, cube[2] + MAP_SIZE / 2, 2);
      break;
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
      isMovingForward = false;
      break;
    case Qt::Key_S:
      isMovingBackward = false;
      break;
    case Qt::Key_A:
      isMovingLeft = false;
      break;
    case Qt::Key_D:
      isMovingRight = false;
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
  m_viewFrame.RotateWorld (-(event->x () - centerX) / 100., 0., 0., 1.);
  m_viewFrame.RotateLocalX ((event->y () - centerY) / 100.);
  cursor ().setPos (mapToGlobal (QPoint (centerX, centerY)));
  isLocked = false;
  updateGL ();
}

void GLWidget::timerEvent (QTimerEvent* event) {
  FIX_UNUSED (event);

  double timeElasped = time.elapsed () / 1000.;
  if (isMovingForward)
    m_viewFrame.MoveForward (5. * timeElasped);
  if (isMovingBackward)
    m_viewFrame.MoveForward (-3. * timeElasped);
  if (isMovingLeft)
    m_viewFrame.MoveRight (3. * timeElasped);
  if (isMovingRight)
    m_viewFrame.MoveRight (-3. * timeElasped);
  time.restart ();

  double fpsTimeElapsed = fpsTime.elapsed () / 1000.;
  if (fpsTimeElapsed > FPS_MEASURE_INTERVAL) {
    std::cout << "fps = " << m_nFramesDrawn << std::endl;
    m_nFramesDrawn = 0;
    fpsTime.restart ();
  }
  updateGL ();
}
