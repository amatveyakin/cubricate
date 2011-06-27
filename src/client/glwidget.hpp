#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <QTime>
#include <QGLWidget>
#include <QGLShaderProgram>

#include "common/linear_algebra.hpp"

// #include "client/visible_cube_set.hpp"
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
  QGLShaderProgram m_basicShaderProgram;

  GLint   m_instancedCubeShader;
  GLint   m_basicShader;
  
  GLuint  m_cubesVao, m_selectingBoxVao;
  GLuint  m_cubeVbo,  m_selectingBoxVbo;
  
  GLuint  m_locInstancedCubeMvp, m_locInstancedCubeMapSize, m_locColor, m_locInstancedCubeSquareTexture;
  GLuint  m_locBasicShaderWVP, m_locBasicShaderColor;
  
  //Raytracing
  QGLShaderProgram m_raytracingShaderProgram;
  GLint   m_raytracingShader;
  GLuint  m_locOctTree, m_locOrigin;
  GLuint  m_octTreeBuffer;
  GLuint  m_octTreeTexture;
  
  GLuint  m_squareTextureArray;

  GLFrustum     m_viewFrustum;

  bool m_isMovingForward;
  bool m_isMovingBackward;
  bool m_isMovingLeft;
  bool m_isMovingRight;

  QTime m_time;
  QTime m_fpsTime;


  void lockCubes ();
  void unlockCubes ();
  Vec3i getCubeByPoint (Vec3d point, Vec3d direction);
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
