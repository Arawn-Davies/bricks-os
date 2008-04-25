#include "glMatrix.h"
#include "string.h"
typedef unsigned int wint_t;
#include <math.h>


#ifdef CONFIG_FPU
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CAGLESMatrixF::CAGLESMatrixF()
 : matrixMode_(GL_MODELVIEW)
 , pCurrentMatrix_(&matrixModelView)
{
}

//---------------------------------------------------------------------------
CAGLESMatrixF::~CAGLESMatrixF()
{
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
  GLfloat m[16];

  m[0*4+0] = (2.0f * zNear) / (right - left);
  m[0*4+1] = 0.0f;
  m[0*4+2] = (right + left) / (right - left);
  m[0*4+3] = 0.0f;

  m[1*4+0] = 0.0f;
  m[1*4+1] = (2.0f * zNear) / (top - bottom);
  m[1*4+2] = (top + bottom) / (top - bottom);
  m[1*4+3] = 0.0f;

  m[2*4+0] = 0.0f;
  m[2*4+1] = 0.0f;
  m[2*4+2] = -((zFar + zNear) / (zFar - zNear));
  m[2*4+3] = -((2.0f * zFar * zNear) / (zFar - zNear));

  m[3*4+0] = 0.0f;
  m[3*4+1] = 0.0f;
  m[3*4+2] = -1.0f;
  m[3*4+3] = 0.0f;

  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glLoadMatrixf(const GLfloat *m)
{
  *pCurrentMatrix_ = m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glMultMatrixf(const GLfloat *m)
{
  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
  GLfloat m[16];

  m[0*4+0] = 2.0f / (right - left);
  m[0*4+1] = 0.0f;
  m[0*4+2] = 0.0f;
  m[0*4+3] = -((right + left) / (right - left));

  m[1*4+0] = 0.0f;
  m[1*4+1] = 2.0f / (top - bottom);
  m[1*4+2] = 0.0f;
  m[1*4+3] = -((top + bottom) / (top - bottom));

  m[2*4+0] = 0.0f;
  m[2*4+1] = 0.0f;
  m[2*4+2] = -2.0f / (zFar - zNear);
  m[2*4+3] = -((zFar + zNear) / (zFar - zNear));

  m[3*4+0] = 0.0f;
  m[3*4+1] = 0.0f;
  m[3*4+2] = 0.0f;
  m[3*4+3] = 1.0f;

  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  pCurrentMatrix_->rotate(angle, x, y, z);
  if(matrixMode_ == GL_MODELVIEW)
    matrixNormal.rotate(angle, x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
  pCurrentMatrix_->scale(x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
  pCurrentMatrix_->translate(x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glLoadIdentity(void)
{
  pCurrentMatrix_->loadIdentity();
  if(matrixMode_ == GL_MODELVIEW)
    matrixNormal.loadIdentity();
}

//---------------------------------------------------------------------------
void
CAGLESMatrixF::glMatrixMode(GLenum mode)
{
  matrixMode_ = mode;

  switch(mode)
  {
    case GL_MODELVIEW:  pCurrentMatrix_ = &matrixModelView;  break;
    case GL_PROJECTION: pCurrentMatrix_ = &matrixProjection; break;
    case GL_TEXTURE:    pCurrentMatrix_ = &matrixTexture;    break;
  };
}
#else // CONFIG_FPU
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CAGLESMatrixFx::CAGLESMatrixFx()
 : matrixMode_(GL_MODELVIEW)
 , pCurrentMatrix_(&matrixModelView)
{
}

//---------------------------------------------------------------------------
CAGLESMatrixFx::~CAGLESMatrixFx()
{
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
  GLfixed m[16];

  m[0*4+0] = gl_fpdiv((zNear << 1), (right - left));
  m[0*4+1] = gl_fpfromi(0);
  m[0*4+2] = gl_fpdiv((right + left), (right - left));
  m[0*4+3] = gl_fpfromi(0);

  m[1*4+0] = gl_fpfromi(0);
  m[1*4+1] = gl_fpdiv((zNear << 1), (top - bottom));
  m[1*4+2] = gl_fpdiv((top + bottom), (top - bottom));
  m[1*4+3] = gl_fpfromi(0);

  m[2*4+0] = gl_fpfromi(0);
  m[2*4+1] = gl_fpfromi(0);
  m[2*4+2] = -gl_fpdiv((zFar + zNear), (zFar - zNear));
  m[2*4+3] = -gl_fpdiv((gl_fpmul(zFar, zNear) << 1), (zFar - zNear));

  m[3*4+0] = gl_fpfromi(0);
  m[3*4+1] = gl_fpfromi(0);
  m[3*4+2] = gl_fpfromi(-1);
  m[3*4+3] = gl_fpfromi(0);

  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glLoadMatrixx(const GLfixed *m)
{
  *pCurrentMatrix_ = m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glMultMatrixx(const GLfixed *m)
{
  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
  GLfixed m[16];

  m[0*4+0] = gl_fpdiv(gl_fpfromi(2), (right - left));
  m[0*4+1] = gl_fpfromi(0);
  m[0*4+2] = gl_fpfromi(0);
  m[0*4+3] = -gl_fpdiv((right + left), (right - left));

  m[1*4+0] = gl_fpfromi(0);
  m[1*4+1] = gl_fpdiv(gl_fpfromi(2), (top - bottom));
  m[1*4+2] = gl_fpfromi(0);
  m[1*4+3] = -gl_fpdiv((top + bottom), (top - bottom));

  m[2*4+0] = gl_fpfromi(0);
  m[2*4+1] = gl_fpfromi(0);
  m[2*4+2] = gl_fpdiv(gl_fpfromi(-2), (zFar - zNear));
  m[2*4+3] = -gl_fpdiv((zFar + zNear), (zFar - zNear));

  m[3*4+0] = gl_fpfromi(0);
  m[3*4+1] = gl_fpfromi(0);
  m[3*4+2] = gl_fpfromi(0);
  m[3*4+3] = gl_fpfromi(1);

  *pCurrentMatrix_ *= m;
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
  pCurrentMatrix_->rotate(angle, x, y, z);
  if(matrixMode_ == GL_MODELVIEW)
    matrixNormal.rotate(angle, x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glScalex(GLfixed x, GLfixed y, GLfixed z)
{
  pCurrentMatrix_->scale(x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
  pCurrentMatrix_->translate(x, y, z);
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glLoadIdentity(void)
{
  pCurrentMatrix_->loadIdentity();
  if(matrixMode_ == GL_MODELVIEW)
    matrixNormal.loadIdentity();
}

//---------------------------------------------------------------------------
void
CAGLESMatrixFx::glMatrixMode(GLenum mode)
{
  matrixMode_ = mode;

  switch(mode)
  {
    case GL_MODELVIEW:  pCurrentMatrix_ = &matrixModelView;  break;
    case GL_PROJECTION: pCurrentMatrix_ = &matrixProjection; break;
    case GL_TEXTURE:    pCurrentMatrix_ = &matrixTexture;    break;
  };
}
#endif // CONFIG_FPU
