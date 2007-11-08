#ifndef PS2_TEXTURES_H
#define PS2_TEXTURES_H


#include "../../../../gl/context.h"


#define MAX_TEXTURE_COUNT 32


//-----------------------------------------------------------------------------
struct STexturePS2
{
  bool used;

  GLsizei width;
  GLsizei height;

  const void * data;
};

//-----------------------------------------------------------------------------
class CAGLESTexturesPS2
 : public virtual IGLESContext
{
public:
  CAGLESTexturesPS2();
  virtual ~CAGLESTexturesPS2();

  virtual void glBindTexture(GLenum target, GLuint texture);
  virtual void glDeleteTextures(GLsizei n, const GLuint *textures);
  virtual void glGenTextures(GLsizei n, GLuint *textures);
  virtual void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);

protected:
  STexturePS2 * pCurrentTex_;

private:
  STexturePS2 textures_[MAX_TEXTURE_COUNT];
};


#endif