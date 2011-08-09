#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <SFML/Window.hpp>

#include "common/linear_algebra.hpp"



class GLWidget
{
public:
  GLWidget (sf::Window& app);
  ~GLWidget();

  int  getGameTime() const; // msec  // TODO: Should it be in this class?

  void updateGL();

  void lockCubes();
  void unlockCubes();

  void resizeEvent (const sf::Event::SizeEvent& event);
  void keyPressEvent (const sf::Event::KeyEvent& event);
  void keyReleaseEvent (const sf::Event::KeyEvent& event);
  void mousePressEvent (const sf::Event::MouseButtonEvent& event);
  void mouseWheelEvent (const sf::Event::MouseWheelEvent& event);
  void timerEvent();

protected:
  void initializeGL();
  void paintGL();

  sf::Window& m_app;

  int m_nFramesDrawn;
  int m_nPhysicsStepsProcessed;

  //Raytracing
  GLint   m_raytracingShader, m_raytracingDepthPassShader;
  GLuint  m_locOctTree,     m_locOrigin,       m_locViewMatrix;
  GLuint  m_locCubeTexture, m_locDepthTexture, m_locCubePropertiesTexture, m_locSiblingShiftTableTexture;
  GLuint  m_locCubeNormalMap, m_locCubeDecal, m_locLightMap, m_locSunVisibilityMap, m_locSunlightSH;
  GLuint  m_locDepthPassOctTree, m_locDepthPassOrigin, m_locDepthPassViewMatrix, m_locDepthPassSiblingShiftTableTexture;
  GLuint  m_locDepthPassCubeNormalMap;
  GLuint  m_octTreeBuffer,           m_octTreeTexture;
  GLuint  m_cubePropertiesBuffer,    m_cubePropertiesTexture;
  GLuint  m_siblingShiftTableBuffer, m_siblingShiftTableTexture;
  GLuint  m_raytracingFirstPassResult;
  GLuint  m_raytracingVAO,  m_raytracingVBO,   m_raytracingFBO;
  GLuint  m_cubeTexture, m_cubeNormalMap, m_cubeDecal;
  GLuint  m_lightMapTexture, m_sunVisibilityTexture;

  GLuint  m_renderTimeQuery[3];
  GLuint  m_totalDepthPassTime, m_totalMainPassTime, m_totalUITime;

  GLint  m_UIShader;
  GLuint m_locUITexture;
  GLuint m_UITexture;

  bool m_isMovingForward;
  bool m_isMovingBackward;
  bool m_isMovingLeft;
  bool m_isMovingRight;
  bool m_isJumping;

  bool m_worldFreezed;

  sf::Clock m_gameTime;
  sf::Clock m_eventTime;
  sf::Clock m_fpsTime;
  sf::Clock m_physicsTime;


  void explosion (int explosionX, int explosionY, int explosionZ, int explosionRadius);
  void summonMeteorite (int meteoriteX, int meteoriteY);
  void initBuffers ();
  void initTextures ();
  void initShaders ();
  void initQueries ();
  void setupRenderContext ();
  void shutdownRenderContext ();

  void renderUI ();
};


#endif
