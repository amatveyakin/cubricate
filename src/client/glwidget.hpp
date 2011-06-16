#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP


// #define GL_GLEXT_PROTOTYPES
// #include <GL/gl.h>
// #include <GL/glext.h>

#include <GLTools.h>            // OpenGL toolkit
// #include <GLShaderManager.h>    // Shader Manager Class
// #include <math3d.h>
// #include <GL/glew.h>

#include <GLFrame.h>
#include <GLFrustum.h>
// #include <GLMatrixStack.h>
// #include <GLGeometryTransform.h>


#include <QTime>
#include <QGLWidget>

#include "client/cube_array.hpp"



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

  GLint   m_instancedCubeShader;
  GLuint  m_cubesVao;
  GLuint  m_cubeVbo;
  GLuint  m_locMvp, m_locMapSize, m_locColor, m_locSquareTexture;
  GLuint  m_squareTextureArray;
  GLfloat m_cameraAlpha, m_cameraBeta, m_cameraDistance;

//   GLShaderManager shader_manager;

  GLFrame       m_viewFrame;
  GLFrustum     m_viewFrustum;
//   GLMatrixStack m_modelViewMatix;
//   GLMatrixStack m_projectionMatrix;
//   GLGeometryTransform m_transformPipeline;

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
  void getCubeByPoint (M3DVector3i cube, M3DVector3f point, M3DVector3f direction);
  void lookAt (M3DVector3f result);
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
