#ifndef GL_SHADER_LOADER
#define GL_SHADER_LOADER


const int MAX_SHADER_LENGTH = 65536;


// Load the shader from the source text
void gltLoadShaderSrc (const char *szShaderSrc, GLuint shader);

// Load the shader from the specified file. Returns false if the
// shader could not be loaded
bool gltLoadShaderFile (const char *szFile, GLuint shader);

// Load a pair of shaders, compile, and link together. Specify the complete
// source text for each shader. After the shader names, specify the number
// of attributes, followed by the index and attribute name of each attribute
GLuint gltLoadShaderPairWithAttributes (const char *szVertexProg, const char *szFragmentProg, ...);


#endif
