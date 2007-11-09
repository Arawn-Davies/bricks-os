#include "GLES/gl.h"
#include "../gl/fixedPoint.h"


// -----------------------------------------------------------------------------
const GLfloat pyramidVertF[] =
{
  // Square (strip)
  -1.0f, -1.5f,  1.0f,
   1.0f, -1.5f,  1.0f,
  -1.0f, -1.5f, -1.0f,
   1.0f, -1.5f, -1.0f,

  // Pyramid (fan)
   0.0f,  1.0f,  0.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
};

// -----------------------------------------------------------------------------
const GLfixed pyramidVertFx[] =
{
  // Square (strip)
  gl_fpfromf(-1.0f), gl_fpfromf(-1.5f), gl_fpfromf( 1.0f),
  gl_fpfromf( 1.0f), gl_fpfromf(-1.5f), gl_fpfromf( 1.0f),
  gl_fpfromf(-1.0f), gl_fpfromf(-1.5f), gl_fpfromf(-1.0f),
  gl_fpfromf( 1.0f), gl_fpfromf(-1.5f), gl_fpfromf(-1.0f),

  // Pyramid (fan)
  gl_fpfromf( 0.0f), gl_fpfromf( 1.0f), gl_fpfromf( 0.0f),
  gl_fpfromf(-1.0f), gl_fpfromf(-1.0f), gl_fpfromf( 1.0f),
  gl_fpfromf( 1.0f), gl_fpfromf(-1.0f), gl_fpfromf( 1.0f),
  gl_fpfromf( 1.0f), gl_fpfromf(-1.0f), gl_fpfromf(-1.0f),
  gl_fpfromf(-1.0f), gl_fpfromf(-1.0f), gl_fpfromf(-1.0f),
  gl_fpfromf(-1.0f), gl_fpfromf(-1.0f), gl_fpfromf( 1.0f)
};

// -----------------------------------------------------------------------------
const GLfloat pyramidColF[] =
{
  // Square
  0.4f, 0.4f, 0.4f, 1.0f,
  0.4f, 0.4f, 0.4f, 1.0f,
  0.4f, 0.4f, 0.4f, 1.0f,
  0.4f, 0.4f, 0.4f, 1.0f,

  // Pyramid
  1.0f, 0.0f, 0.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f, 1.0f,
  0.0f, 1.0f, 0.0f, 1.0f
};

// -----------------------------------------------------------------------------
const GLfixed pyramidColFx[] =
{
  // Square
  gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(1.0f),
  gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(1.0f),
  gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(1.0f),
  gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(0.4f), gl_fpfromf(1.0f),

  // Pyramid
  gl_fpfromf(1.0f), gl_fpfromf(0.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f),
  gl_fpfromf(0.0f), gl_fpfromf(1.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f),
  gl_fpfromf(0.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f), gl_fpfromf(1.0f),
  gl_fpfromf(0.0f), gl_fpfromf(1.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f),
  gl_fpfromf(0.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f), gl_fpfromf(1.0f),
  gl_fpfromf(0.0f), gl_fpfromf(1.0f), gl_fpfromf(0.0f), gl_fpfromf(1.0f)
};


// -----------------------------------------------------------------------------
void
initPyramidF()
{
}

// -----------------------------------------------------------------------------
void
initPyramidFx()
{
}

// -----------------------------------------------------------------------------
void
drawPyramidF()
{
  glVertexPointer(3, GL_FLOAT, 0, pyramidVertF);
  glColorPointer(4, GL_FLOAT, 0, pyramidColF);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  //glDrawArrays(GL_TRIANGLE_STRIP,  0, 4); // 'Shade'
  glDrawArrays(GL_TRIANGLE_FAN,    4, 6); // Pyramid
}

// -----------------------------------------------------------------------------
void
drawPyramidFx()
{
  glVertexPointer(3, GL_FIXED, 0, pyramidVertFx);
  glColorPointer(4, GL_FIXED, 0, pyramidColFx);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  //glDrawArrays(GL_TRIANGLE_STRIP,  0, 4); // 'Shade'
  glDrawArrays(GL_TRIANGLE_FAN,    4, 6); // Pyramid
}
