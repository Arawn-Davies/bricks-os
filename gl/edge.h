#ifndef EDGE_H
#define EDGE_H


#include "GLES/gl.h"
#include "context.h"
#include "inttypes.h"
#include "asm/arch/config.h"


#ifdef CONFIG_FPU
//-----------------------------------------------------------------------------
class CEdgeF
{
public:
  CEdgeF(uint32_t height);
  ~CEdgeF();

  void add  (GLint x1, GLint y1, GLint x2, GLint y2);
  void addZ (GLint x1, GLint y1, GLfloat z1, GLint x2, GLint y2, GLfloat z2);
  void addC (GLint x1, GLint y1, SColorF & c1, GLint x2, GLint y2, SColorF & c2);
  void addZT(GLint x1, GLint y1, GLfloat z1, GLfloat ts1, GLfloat tt1, GLint x2, GLint y2, GLfloat z2, GLfloat ts2, GLfloat tt2);
  void addZC(GLint x1, GLint y1, GLfloat z1, SColorF & c1, GLint x2, GLint y2, GLfloat z2, SColorF & c2);

public:
  // Edge x
  GLint * x_;
  // Edge depth
  GLfloat * z_;
  // Texture coordinates
  GLfloat * ts_;
  GLfloat * tt_;
  // Edge color
  SColorF * c_;

  int32_t iHeight_;
};
#else
//-----------------------------------------------------------------------------
class CEdgeFx
{
public:
  CEdgeFx(uint32_t height);
  ~CEdgeFx();

  void add  (GLint x1, GLint y1, GLint x2, GLint y2);
  void addZ (GLint x1, GLint y1, GLfixed z1, GLint x2, GLint y2, GLfixed z2);
  void addC (GLint x1, GLint y1, SColorFx & c1, GLint x2, GLint y2, SColorFx & c2);
  void addZT(GLint x1, GLint y1, GLfixed z1, GLfixed ts1, GLfixed tt1, GLint x2, GLint y2, GLfixed z2, GLfixed ts2, GLfixed tt2);
  void addZC(GLint x1, GLint y1, GLfixed z1, SColorFx & c1, GLint x2, GLint y2, GLfixed z2, SColorFx & c2);

public:
  // Edge x
  GLint * x_;
  // Edge depth
  GLfixed * z_;
  // Texture coordinates
  GLfixed * ts_;
  GLfixed * tt_;
  // Edge color
  SColorFx * c_;

  int32_t iHeight_;
};
#endif // CONFIG_FPU


#endif
