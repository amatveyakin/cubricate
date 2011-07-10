#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <QTime>
#include <QGLWidget>
#include <QGLShaderProgram>

#include "common/linear_algebra.hpp"



class GLWidget : public QGLWidget
{
public:
  GLWidget ();
  ~GLWidget ();

  void initializeGL ();
  void paintGL ();
  void resizeGL (int width, int height);

  void lockCubes ();
  void unlockCubes ();

protected:
  int m_nFramesDrawn;
  int m_nPhysicsStepsProcessed;

  QGLShaderProgram m_shaderProgram;
  QGLShaderProgram m_basicShaderProgram;

  //Raytracing
  QGLShaderProgram m_raytracingShaderProgram;
  QGLShaderProgram m_raytracingDepthPassShaderProgram;
  GLint   m_raytracingShader, m_raytracingDepthPassShader;
  GLuint  m_locOctTree,     m_locOrigin,       m_locViewMatrix;
  GLuint  m_locCubeTexture, m_locDepthTexture, m_locCubePropertiesTexture, m_locSiblingShiftTableTexture;
  GLuint  m_locCubeNormalMap;
  GLuint  m_locDepthPassOctTree, m_locDepthPassOrigin, m_locDepthPassViewMatrix, m_locDepthPassSiblingShiftTableTexture;
  GLuint  m_octTreeBuffer,           m_octTreeTexture;
  GLuint  m_cubePropertiesBuffer,    m_cubePropertiesTexture;
  GLuint  m_siblingShiftTableBuffer, m_siblingShiftTableTexture;
  GLuint  m_raytracingFirstPassResult;
  GLuint  m_raytracingVAO,  m_raytracingVBO,   m_raytracingFBO;
  GLuint  m_cubeTexture, m_cubeNormalMap
;

  QGLShaderProgram m_UIShaderProgram;
  GLint  m_UIShader;
  GLuint m_locUITexture;
  GLuint m_UITexture;

  bool m_isMovingForward;
  bool m_isMovingBackward;
  bool m_isMovingLeft;
  bool m_isMovingRight;

  QTime m_time;
  QTime m_fpsTime;
  QTime m_physicsTime;


  void explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius);
  void summonMeteorite (int meteoriteX, int meteoriteY);
  void initBuffers ();
  void initTextures ();
  void initShaders ();
  void setupRenderContext ();
  void shutdownRenderContext ();

  void keyPressEvent (QKeyEvent* event);
  void keyReleaseEvent (QKeyEvent* event);
  void mouseMoveEvent (QMouseEvent* event);
  void mousePressEvent (QMouseEvent* event);
  void wheelEvent (QWheelEvent* event);
  void timerEvent (QTimerEvent* event);

  void renderUI ();
};


#endif
