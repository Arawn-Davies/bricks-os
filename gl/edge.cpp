#include "edge.h"
#include "fixedPoint.h"


// Fixed Point Macros
#define DELTA_F_Y() \
  GLfloat dy(1.0f / (GLfloat)(y2 - y1))
#define INTERPOLATE_F_X() \
  GLfloat x((GLfloat)x1); \
  GLfloat mx((GLfloat)(x2 - x1) * dy)
#define INTERPOLATE_F_Z() \
  GLfloat mz((z2 - z1) * dy)
#define INTERPOLATE_F_C() \
  GLfloat r(c1.r); \
  GLfloat g(c1.g); \
  GLfloat b(c1.b); \
  GLfloat a(c1.a); \
  GLfloat mr((c2.r - c1.r) * dy); \
  GLfloat mg((c2.g - c1.g) * dy); \
  GLfloat mb((c2.b - c1.b) * dy); \
  GLfloat ma((c2.a - c1.a) * dy)
#define INTERPOLATE_F_T() \
  GLfloat ts(ts1); \
  GLfloat tt(tt1); \
  GLfloat mts((ts2 - ts1) / (GLfloat)(y2 - y1)); \
  GLfloat mtt((tt2 - tt1) / (GLfloat)(y2 - y1))
#define STORE_F_X() \
  x_[y1] = (GLint)x
#define STORE_F_Z() \
  z_[y1] = z1
#define STORE_F_C() \
  c_[y1].r = r; \
  c_[y1].g = g; \
  c_[y1].b = b; \
  c_[y1].a = a
#define STORE_F_T() \
  ts_[y1] = ts; \
  tt_[y1] = tt

// Fixed Point Macros
#define DELTA_FX_Y() \
  GLfixed dy(gl_fpdiv(gl_fpfromi(1), gl_fpfromi(y2 - y1)))
#define INTERPOLATE_FX_X() \
  GLfixed x(gl_fpfromi(x1)); \
  GLfixed mx(gl_fpmul(gl_fpfromi(x2 - x1), dy))
#define INTERPOLATE_FX_Z() \
  GLfixed mz(gl_fpmul(z2 - z1, dy))
#define INTERPOLATE_FX_C() \
  GLfixed r(c1.r); \
  GLfixed g(c1.g); \
  GLfixed b(c1.b); \
  GLfixed a(c1.a); \
  GLfixed mr(gl_fpmul(c2.r - c1.r, dy)); \
  GLfixed mg(gl_fpmul(c2.g - c1.g, dy)); \
  GLfixed mb(gl_fpmul(c2.b - c1.b, dy)); \
  GLfixed ma(gl_fpmul(c2.a - c1.a, dy))
#define INTERPOLATE_FX_T() \
  GLfixed ts(ts1); \
  GLfixed tt(tt1); \
  GLfixed mts(gl_fpmul(ts2 - ts1, dy)); \
  GLfixed mtt(gl_fpmul(tt2 - tt1, dy))
#define STORE_FX_X() \
  x_[y1] = gl_fptoi(x)
#define STORE_FX_Z() \
  z_[y1] = z1
#define STORE_FX_C() \
  c_[y1].r = r; \
  c_[y1].g = g; \
  c_[y1].b = b; \
  c_[y1].a = a
#define STORE_FX_T() \
  ts_[y1] = ts; \
  tt_[y1] = tt

// Common Macros
#define INCREMENT_X() \
  x += mx
#define INCREMENT_Z() \
  z1 += mz
#define INCREMENT_C() \
  r += mr; \
  g += mg; \
  b += mb; \
  a += ma
#define INCREMENT_T() \
  ts += mts; \
  tt += mtt


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CEdgeF::CEdgeF(uint32_t height)
 : iHeight_(height)
{
  x_  = new GLint[iHeight_];
  z_  = new GLfloat[iHeight_];
  ts_ = new GLfloat[iHeight_];
  tt_ = new GLfloat[iHeight_];
  c_  = new SColorF[iHeight_];
}

//-----------------------------------------------------------------------------
CEdgeF::~CEdgeF()
{
  delete x_;
  delete z_;
  delete ts_;
  delete tt_;
  delete c_;
}

//-----------------------------------------------------------------------------
void
CEdgeF::add(GLint x1, GLint y1, GLint x2, GLint y2)
{
  if(y1 < y2)
  {
    DELTA_F_Y();
    INTERPOLATE_F_X();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_F_X();
      }
      INCREMENT_X();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeF::addZ(GLint x1, GLint y1, GLfloat z1, GLint x2, GLint y2, GLfloat z2)
{
  if(y1 < y2)
  {
    DELTA_F_Y();
    INTERPOLATE_F_X();
    INTERPOLATE_F_Z();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_F_X();
        STORE_F_Z();
      }
      INCREMENT_X();
      INCREMENT_Z();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeF::addC(GLint x1, GLint y1, SColorF & c1, GLint x2, GLint y2, SColorF & c2)
{
  if(y1 < y2)
  {
    DELTA_F_Y();
    INTERPOLATE_F_X();
    INTERPOLATE_F_C();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_F_X();
        STORE_F_C();
      }
      INCREMENT_X();
      INCREMENT_C();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeF::addZT(GLint x1, GLint y1, GLfloat z1, GLfloat ts1, GLfloat tt1, GLint x2, GLint y2, GLfloat z2, GLfloat ts2, GLfloat tt2)
{
  if(y1 < y2)
  {
    DELTA_F_Y();
    INTERPOLATE_F_X();
    INTERPOLATE_F_Z();
    INTERPOLATE_F_T();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_F_X();
        STORE_F_Z();
        STORE_F_T();
      }

      INCREMENT_X();
      INCREMENT_Z();
      INCREMENT_T();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeF::addZC(GLint x1, GLint y1, GLfloat z1, SColorF & c1, GLint x2, GLint y2, GLfloat z2, SColorF & c2)
{
  if(y1 < y2)
  {
    DELTA_F_Y();
    INTERPOLATE_F_X();
    INTERPOLATE_F_Z();
    INTERPOLATE_F_C();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_F_X();
        STORE_F_Z();
        STORE_F_C();
      }
      INCREMENT_X();
      INCREMENT_Z();
      INCREMENT_C();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CEdgeFx::CEdgeFx(uint32_t height)
 : iHeight_(height)
{
  x_  = new GLint[iHeight_];
  z_  = new GLfixed[iHeight_];
  ts_ = new GLfixed[iHeight_];
  tt_ = new GLfixed[iHeight_];
  c_  = new SColorFx[iHeight_];
}

//-----------------------------------------------------------------------------
CEdgeFx::~CEdgeFx()
{
  delete x_;
  delete z_;
  delete ts_;
  delete tt_;
  delete c_;
}

//-----------------------------------------------------------------------------
void
CEdgeFx::add(GLint x1, GLint y1, GLint x2, GLint y2)
{
  if(y1 < y2)
  {
    DELTA_FX_Y();
    INTERPOLATE_FX_X();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;
      if(y1 >= 0)
      {
        STORE_FX_X();
      }
      INCREMENT_X();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeFx::addZ(GLint x1, GLint y1, GLfixed z1, GLint x2, GLint y2, GLfixed z2)
{
  if(y1 < y2)
  {
    DELTA_FX_Y();
    INTERPOLATE_FX_X();
    INTERPOLATE_FX_Z();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;
      if(y1 >= 0)
      {
        STORE_FX_X();
        STORE_FX_Z();
      }
      INCREMENT_X();
      INCREMENT_Z();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeFx::addC(GLint x1, GLint y1, SColorFx & c1, GLint x2, GLint y2, SColorFx & c2)
{
  if(y1 < y2)
  {
    DELTA_FX_Y();
    INTERPOLATE_FX_X();
    INTERPOLATE_FX_C();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_FX_X();
        STORE_FX_C();
      }

      INCREMENT_X();
      INCREMENT_C();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeFx::addZT(GLint x1, GLint y1, GLfixed z1, GLfixed ts1, GLfixed tt1, GLint x2, GLint y2, GLfixed z2, GLfixed ts2, GLfixed tt2)
{
  if(y1 < y2)
  {
    DELTA_FX_Y();
    INTERPOLATE_FX_X();
    INTERPOLATE_FX_Z();
    INTERPOLATE_FX_T();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_FX_X();
        STORE_FX_Z();
        STORE_FX_T();
      }

      INCREMENT_X();
      INCREMENT_Z();
      INCREMENT_T();
    }
  }
}

//-----------------------------------------------------------------------------
void
CEdgeFx::addZC(GLint x1, GLint y1, GLfixed z1, SColorFx & c1, GLint x2, GLint y2, GLfixed z2, SColorFx & c2)
{
  if(y1 < y2)
  {
    DELTA_FX_Y();
    INTERPOLATE_FX_X();
    INTERPOLATE_FX_Z();
    INTERPOLATE_FX_C();

    for(;y1 < y2; y1++)
    {
      if(y1 >= iHeight_)
        break;

      if(y1 >= 0)
      {
        STORE_FX_X();
        STORE_FX_Z();
        STORE_FX_C();
      }

      INCREMENT_X();
      INCREMENT_Z();
      INCREMENT_C();
    }
  }
}
