#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP


#ifdef WINDOWS
  #define GL_GLEXT_PROTOTYPES   // TODO: is this necessary?
  #include "common/gl/gl.h"
  #include "common/gl/glprocs.h"
#else // !WINDOWS
  #define GL_GLEXT_PROTOTYPES
  #include <GL/gl.h>
#endif // !WINDOWS


#endif
