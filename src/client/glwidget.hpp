#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <QTime>
#include <QGLWidget>
#include <QGLShaderProgram>

#include "common/linear_algebra.hpp"
#include "client/cube_array.hpp"
#include "client/GLTools/GLFrustum.h"



class GLWidget : public QGLWidget
{
public:
  GLWidget ();
  ~GLWidget ();

  void initializeGL ();
  void paintGL ();
  void resizeGL (int width, int height);

protected:
  int m_nFramesDrawn;

  int m_CUBES_INFORMATION_OFFSET;

  QGLShaderProgram m_shaderProgram;

  GLint   m_instancedCubeShader;
  GLuint  m_cubesVao;
  GLuint  m_cubeVbo;
  GLuint  m_locMvp, m_locMapSize, m_locColor, m_locSquareTexture;
  GLuint  m_squareTextureArray;

  GLFrustum     m_viewFrustum;

  bool m_isMovingForward;
  bool m_isMovingBackward;
  bool m_isMovingLeft;
  bool m_isMovingRight;

  QTime m_time;
  QTime m_fpsTime;

  Visible_cube_set <GLfloat, GLfloat> m_cubeArray;


  // TODO: add ``m_'' to function too
  void lockCubes ();
  void unlockCubes ();
  bool coordinatesValid (int x, int y, int z);
  Vec3i getCubeByPoint (Vec3d point, Vec3d direction);
  Vec3d lookAt ();
  void explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius);
  void summonMeteorite (int meteoriteX, int meteoriteY);
  bool loadTGATexture (const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
  void initBuffers ();
  void initTextures ();
  void initShaders ();
  void updateCamera ();
  void setupRenderContext ();
  void shutdownRenderContext ();

  void keyPressEvent (QKeyEvent* event);
  void keyReleaseEvent (QKeyEvent* event);
  void mouseMoveEvent (QMouseEvent* event);
  void mousePressEvent (QMouseEvent* event);
  void timerEvent (QTimerEvent* event);
};


#endif
