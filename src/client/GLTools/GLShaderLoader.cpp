#include <cstdarg>
#include <cstdio>
#include <cassert>


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include "client/GLTools/GLShaderLoader.h"


// Rather than malloc/free a block everytime a shader must be loaded,
// I will dedicate a single 4k page for reading in shaders. Thanks to
// modern OS design, this page will be swapped out to disk later if never
// used again after program initialization. Where-as mallocing different size
// shader blocks could lead to heap fragmentation, which would actually be worse.
// BTW... this does make this function unsafe to use in two threads simultaneously
// BTW BTW... personally.... I do this also to my own texture loading code - RSW
static GLubyte shaderText [MAX_SHADER_LENGTH];


void gltLoadShaderSrc (const char *szShaderSrc, GLuint shader)
{
  GLchar *fsStringPtr[1];

  fsStringPtr[0] =  (GLchar *)szShaderSrc;
  glShaderSource (shader, 1,  (const GLchar **)fsStringPtr, NULL);
}


bool gltLoadShaderFile (const char *szFile, GLuint shader)
{
  GLint shaderLength = 0;
  FILE *fp;

  // Open the shader file
  fp = fopen (szFile, "r");
  if (fp != NULL)
  {
    // See how long the file is
    while  (fgetc (fp) != EOF)
      shaderLength++;

    // Allocate a block of memory to send in the shader
      assert (shaderLength < MAX_SHADER_LENGTH);   // make me bigger!
      if (shaderLength > MAX_SHADER_LENGTH)
      {
        fclose (fp);
        return false;
      }

      // Go back to beginning of file
      rewind (fp);

      // Read the whole file in
      if  (shaderText != NULL)
        fread (shaderText, 1, shaderLength, fp);

      // Make sure it is null terminated and close the file
        shaderText[shaderLength] = '\0';
  fclose (fp);
  }
  else
    return false;

  // Load the string
    gltLoadShaderSrc ((const char*) shaderText, shader);

    return true;
}


GLuint gltLoadShaderPairWithAttributes (const char *szVertexProg, const char *szFragmentProg, ...)
{
  // Temporary Shader objects
  GLuint hVertexShader;
  GLuint hFragmentShader;
  GLuint hReturn = 0;
  GLint testVal;

  // Create shader objects
  hVertexShader = glCreateShader (GL_VERTEX_SHADER);
  hFragmentShader = glCreateShader (GL_FRAGMENT_SHADER);

  // Load them. If fail clean up and return null
  // Vertex Program
  if (gltLoadShaderFile (szVertexProg, hVertexShader) == false)
  {
    glDeleteShader (hVertexShader);
    glDeleteShader (hFragmentShader);
    fprintf (stderr, "The shader at %s could not be found.\n", szVertexProg);
    return (GLuint)NULL;
  }

  // Fragment Program
  if (gltLoadShaderFile (szFragmentProg, hFragmentShader) == false)
  {
    glDeleteShader (hVertexShader);
    glDeleteShader (hFragmentShader);
    fprintf (stderr,"The shader at %s  could not be found.\n", szFragmentProg);
    return (GLuint)NULL;
  }

  // Compile them both
  glCompileShader (hVertexShader);
  glCompileShader (hFragmentShader);

  // Check for errors in vertex shader
  glGetShaderiv (hVertexShader, GL_COMPILE_STATUS, &testVal);
  if (testVal == GL_FALSE)
  {
    char infoLog[1024];
    glGetShaderInfoLog (hVertexShader, 1024, NULL, infoLog);
    fprintf (stderr, "The shader at %s failed to compile with the following error:\n%s\n", szVertexProg, infoLog);
    glDeleteShader (hVertexShader);
    glDeleteShader (hFragmentShader);
    return (GLuint)NULL;
  }

  // Check for errors in fragment shader
  glGetShaderiv (hFragmentShader, GL_COMPILE_STATUS, &testVal);
  if (testVal == GL_FALSE)
  {
    char infoLog[1024];
    glGetShaderInfoLog (hFragmentShader, 1024, NULL, infoLog);
    fprintf (stderr, "The shader at %s failed to compile with the following error:\n%s\n", szFragmentProg, infoLog);
    glDeleteShader (hVertexShader);
    glDeleteShader (hFragmentShader);
    return (GLuint)NULL;
  }

  // Create the final program object, and attach the shaders
  hReturn = glCreateProgram ();
  glAttachShader (hReturn, hVertexShader);
  glAttachShader (hReturn, hFragmentShader);


  // Now, we need to bind the attribute names to their specific locations
  // List of attributes
  va_list attributeList;
  va_start (attributeList, szFragmentProg);

  // Iterate over this argument list
  char *szNextArg;
  int iArgCount = va_arg (attributeList, int);     // Number of attributes
  for (int i = 0; i < iArgCount; i++)
  {
    int index = va_arg (attributeList, int);
    szNextArg = va_arg (attributeList, char*);
    glBindAttribLocation (hReturn, index, szNextArg);
  }
  va_end (attributeList);

  // Attempt to link
  glLinkProgram (hReturn);

  // These are no longer needed
  glDeleteShader (hVertexShader);
  glDeleteShader (hFragmentShader);

  // Make sure link worked too
  glGetProgramiv (hReturn, GL_LINK_STATUS, &testVal);
  if (testVal == GL_FALSE)
  {
    char infoLog[1024];
    glGetProgramInfoLog (hReturn, 1024, NULL, infoLog);
    fprintf (stderr,"The programs %s and %s failed to link with the following errors:\n%s\n",
             szVertexProg, szFragmentProg, infoLog);
    glDeleteProgram (hReturn);
    return (GLuint)NULL;
  }

  // All done, return our ready to use shader program
  return hReturn;
}
