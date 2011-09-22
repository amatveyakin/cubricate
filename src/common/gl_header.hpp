#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP


#ifdef _WIN32
  #include "windows.h"
  #include "common/gl/gl.h"
  #include "common/gl/glprocs.h"
#else // !WINDOWS
  #define GL_GLEXT_PROTOTYPES
  #include <GL/gl.h>
#endif // !WINDOWS


#endif
