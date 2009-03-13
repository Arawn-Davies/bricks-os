/*
 * Bricks-OS, Operating System for Game Consoles
 * Copyright (C) 2008 Maximus32 <Maximus32@bricks-os.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 */


#include "softGLF.h"
#include "vhl/fixedPoint.h"
#include "vhl/matrix.h"
#include "bitResolution.h"
#include "color.h"

#include "stdlib.h"
#include "math.h"


//-----------------------------------------------------------------------------
// Model-View matrix
//   from 'object coordinates' to 'eye coordinates'
#define CALC_OBJ_TO_EYE(v) \
  pCurrentModelView_->transform4((v).vo, (v).ve)
//-----------------------------------------------------------------------------
// Projection matrix
//   from 'eye coordinates' to 'clip coordinates'
#define CALC_EYE_TO_CLIP(v) \
  pCurrentProjection_->transform4((v).ve, (v).vc)
//-----------------------------------------------------------------------------
// Perspective division
//   from 'clip coordinates' to 'normalized device coordinates'
#define CALC_CLIP_TO_NDEV(v) \
  GLfloat inv_w; \
  inv_w = 1.0f / (v).vc.w; \
  (v).vd.x = (v).vc.x * inv_w; \
  (v).vd.y = (v).vc.y * inv_w; \
  (v).vd.z = (v).vc.z * inv_w; \
  (v).vd.w = inv_w

#define CLIP_ROUNDING_ERROR (0.00001f) // (1E-5)
inline void setClipFlags(SVertexF & v)
{
  GLfloat w = v.vc.w * (1.0f + CLIP_ROUNDING_ERROR);
  v.clip = ((v.vc.x < -w)     ) |
           ((v.vc.x >  w) << 1) |
           ((v.vc.y < -w) << 2) |
           ((v.vc.y >  w) << 3) |
           ((v.vc.z < -w) << 4) |
           ((v.vc.z >  w) << 5);
}

#define CALC_INTERSECTION(a,b,t) (b + ((a - b) * t))


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CASoftGLESFloat::CASoftGLESFloat()
 : CAGLESFxToFloatContext()
 , CAGLESBase()
 , CAGLESMatrixF()

 , depthTestEnabled_(false)
 , depthMask_(true)
 , depthFunction_(GL_LESS)
 , depthClear_(1.0f)
 , zClearValue_(0x0000ffff)
 , zRangeNear_(0.0f)
 , zRangeFar_(1.0f)
 , zNear_(0.0f)
 , zFar_(1.0f)
 , shadingModel_(GL_FLAT)
 , bSmoothShading_(false)
 , blendingEnabled_(false)
 , blendSFactor_(GL_ONE)
 , blendDFactor_(GL_ZERO)
 , alphaTestEnabled_(false)
 , alphaFunc_(GL_ALWAYS)
 , alphaValue_(0.0f)
 , alphaValueFX_(0)
 , lightingEnabled_(false)
 , normalizeEnabled_(false)
 , fogEnabled_(false)
 , texturesEnabled_(false)
 , texEnvMode_(GL_MODULATE)
 , texEnvColor_(0, 0, 0, 0)
 , beginValid_(false)
{
  clCurrent.r = 1.0f;
  clCurrent.g = 1.0f;
  clCurrent.b = 1.0f;
  clCurrent.a = 1.0f;

  clClear.r = 0.0f;
  clClear.g = 0.0f;
  clClear.b = 0.0f;
  clClear.a = 0.0f;

  // Light properties
  for(int iLight(0); iLight < 8; iLight++)
  {
    lights_[iLight].ambient.r = 0.0f;
    lights_[iLight].ambient.g = 0.0f;
    lights_[iLight].ambient.b = 0.0f;
    lights_[iLight].ambient.a = 1.0f;

    if(iLight == 0)
    {
      lights_[iLight].diffuse.r = 1.0f;
      lights_[iLight].diffuse.g = 1.0f;
      lights_[iLight].diffuse.b = 1.0f;
      lights_[iLight].diffuse.a = 1.0f;

      lights_[iLight].specular.r = 1.0f;
      lights_[iLight].specular.g = 1.0f;
      lights_[iLight].specular.b = 1.0f;
      lights_[iLight].specular.a = 1.0f;
    }
    else
    {
      lights_[iLight].diffuse.r = 0.0f;
      lights_[iLight].diffuse.g = 0.0f;
      lights_[iLight].diffuse.b = 0.0f;
      lights_[iLight].diffuse.a = 0.0f;

      lights_[iLight].specular.r = 0.0f;
      lights_[iLight].specular.g = 0.0f;
      lights_[iLight].specular.b = 0.0f;
      lights_[iLight].specular.a = 0.0f;
    }

    lights_[iLight].position.x = 0.0f;
    lights_[iLight].position.y = 0.0f;
    lights_[iLight].position.z = 1.0f;
    lights_[iLight].position.w = 0.0f;
    lights_[iLight].direction  = lights_[iLight].position.getInverted();

    lights_[iLight].enabled = false;
  }

  // Material properties
  matColorAmbient_.r  = 0.2f;
  matColorAmbient_.g  = 0.2f;
  matColorAmbient_.b  = 0.2f;
  matColorAmbient_.a  = 1.0f;

  matColorDiffuse_.r  = 0.8f;
  matColorDiffuse_.g  = 0.8f;
  matColorDiffuse_.b  = 0.8f;
  matColorDiffuse_.a  = 1.0f;

  matColorSpecular_.r = 0.0f;
  matColorSpecular_.g = 0.0f;
  matColorSpecular_.b = 0.0f;
  matColorSpecular_.a = 1.0f;

  matColorEmission_.r = 0.0f;
  matColorEmission_.g = 0.0f;
  matColorEmission_.b = 0.0f;
  matColorEmission_.a = 1.0f;

  matShininess_       = 0.0f;

  float zsize = (1 << (DEPTH_Z + SHIFT_Z));
  zA_ = ((zsize - 0.5) / 2.0);
  zB_ = ((zsize - 0.5) / 2.0) + (1 << (SHIFT_Z-1));

  texEnvColorFX_.r = (int32_t)(texEnvColor_.r * (1 << SHIFT_COLOR));
  texEnvColorFX_.g = (int32_t)(texEnvColor_.g * (1 << SHIFT_COLOR));
  texEnvColorFX_.b = (int32_t)(texEnvColor_.b * (1 << SHIFT_COLOR));
  texEnvColorFX_.a = (int32_t)(texEnvColor_.a * (1 << SHIFT_COLOR));

  clipPlane_[0] = TVector4<GLfloat>( 1.0,  0.0,  0.0,  1.0); // left
  clipPlane_[1] = TVector4<GLfloat>(-1.0,  0.0,  0.0,  1.0); // right
  clipPlane_[2] = TVector4<GLfloat>( 0.0,  1.0,  0.0,  1.0); // bottom
  clipPlane_[3] = TVector4<GLfloat>( 0.0, -1.0,  0.0,  1.0); // top
  clipPlane_[4] = TVector4<GLfloat>( 0.0,  0.0,  1.0,  1.0); // near
  clipPlane_[5] = TVector4<GLfloat>( 0.0,  0.0, -1.0,  1.0); // far
}

//-----------------------------------------------------------------------------
CASoftGLESFloat::~CASoftGLESFloat()
{
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glAlphaFunc(GLenum func, GLclampf ref)
{
  switch(func)
  {
    case GL_NEVER:
    case GL_LESS:
    case GL_EQUAL:
    case GL_LEQUAL:
    case GL_GREATER:
    case GL_NOTEQUAL:
    case GL_GEQUAL:
    case GL_ALWAYS:
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  };

  alphaFunc_    = func;
  alphaValue_   = clampf(ref);
  alphaValueFX_ = (int32_t)(alphaValue_ * (1 << SHIFT_COLOR));
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
  clClear.r = clampf(red);
  clClear.g = clampf(green);
  clClear.b = clampf(blue);
  clClear.a = clampf(alpha);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glClearDepthf(GLclampf depth)
{
  depthClear_ = clampf(depth);

  zClearValue_ = (uint32_t)(depthClear_ * ((1<<DEPTH_Z)-1));
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glDepthRangef(GLclampf zNear, GLclampf zFar)
{
  zRangeNear_ = clampf(zNear);
  zRangeFar_  = clampf(zFar);

  // FIXME: zA_ and zB_ need to be modified, this function is now useless
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glDepthFunc(GLenum func)
{
  depthFunction_ = func;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glDepthMask(GLboolean flag)
{
  depthMask_ = flag;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glDisable(GLenum cap)
{
  switch(cap)
  {
    case GL_ALPHA_TEST: alphaTestEnabled_  = false; break;
    case GL_BLEND:      blendingEnabled_   = false; break;
    case GL_LIGHTING:   lightingEnabled_   = false; break;
    case GL_LIGHT0:     lights_[0].enabled = false; break;
    case GL_LIGHT1:     lights_[1].enabled = false; break;
    case GL_LIGHT2:     lights_[2].enabled = false; break;
    case GL_LIGHT3:     lights_[3].enabled = false; break;
    case GL_LIGHT4:     lights_[4].enabled = false; break;
    case GL_LIGHT5:     lights_[5].enabled = false; break;
    case GL_LIGHT6:     lights_[6].enabled = false; break;
    case GL_LIGHT7:     lights_[7].enabled = false; break;

    case GL_DEPTH_TEST:
      depthTestEnabled_ = false;
      zbuffer(false); // Notify rasterizer
      break;
    case GL_CULL_FACE:  cullFaceEnabled_   = false; break;
    case GL_FOG:        fogEnabled_        = false; break;
    case GL_TEXTURE_2D: texturesEnabled_   = false; break;
    case GL_NORMALIZE:  normalizeEnabled_  = false; break;

    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  _glDrawArrays(mode, first, count);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glEnable(GLenum cap)
{
  switch(cap)
  {
    case GL_ALPHA_TEST: alphaTestEnabled_  = true; break;
    case GL_BLEND:      blendingEnabled_   = true; break;
    case GL_LIGHTING:   lightingEnabled_   = true; break;
    case GL_LIGHT0:     lights_[0].enabled = true; break;
    case GL_LIGHT1:     lights_[1].enabled = true; break;
    case GL_LIGHT2:     lights_[2].enabled = true; break;
    case GL_LIGHT3:     lights_[3].enabled = true; break;
    case GL_LIGHT4:     lights_[4].enabled = true; break;
    case GL_LIGHT5:     lights_[5].enabled = true; break;
    case GL_LIGHT6:     lights_[6].enabled = true; break;
    case GL_LIGHT7:     lights_[7].enabled = true; break;

    case GL_DEPTH_TEST:
      depthTestEnabled_ = true;
      zbuffer(true); // Notify rasterizer
      break;
    case GL_CULL_FACE:  cullFaceEnabled_   = true; break;
    case GL_FOG:        fogEnabled_        = true; break;
    case GL_TEXTURE_2D: texturesEnabled_   = true; break;
    case GL_NORMALIZE:  normalizeEnabled_  = true; break;

    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glFinish(void)
{
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glFlush(void)
{
  flush();
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glFogf(GLenum pname, GLfloat param)
{
  switch(pname)
  {
    case GL_FOG_DENSITY: fogDensity_ = param; break;
    case GL_FOG_START:   fogStart_   = param; break;
    case GL_FOG_END:     fogEnd_     = param; break;
    case GL_FOG_MODE:                         break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glFogfv(GLenum pname, const GLfloat * params)
{
  switch(pname)
  {
    case GL_FOG_COLOR:
      fogColor_.r = params[0];
      fogColor_.g = params[1];
      fogColor_.b = params[2];
      fogColor_.a = params[3];
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glLightf(GLenum light, GLenum pname, GLfloat param)
{
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glLightfv(GLenum light, GLenum pname, const GLfloat * params)
{
  SLightF * pLight = 0;
  switch(light)
  {
    case GL_LIGHT0: pLight = &lights_[0]; break;
    case GL_LIGHT1: pLight = &lights_[1]; break;
    case GL_LIGHT2: pLight = &lights_[2]; break;
    case GL_LIGHT3: pLight = &lights_[3]; break;
    case GL_LIGHT4: pLight = &lights_[4]; break;
    case GL_LIGHT5: pLight = &lights_[5]; break;
    case GL_LIGHT6: pLight = &lights_[6]; break;
    case GL_LIGHT7: pLight = &lights_[7]; break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  }

  SColorF * pColor = 0;
  switch(pname)
  {
    case GL_AMBIENT:  pColor = &pLight->ambient; break;
    case GL_DIFFUSE:  pColor = &pLight->diffuse; break;
    case GL_SPECULAR: pColor = &pLight->specular; break;
    case GL_POSITION:
      pLight->position.x = params[0];
      pLight->position.y = params[1];
      pLight->position.z = params[2];
      pLight->position.w = params[3];
      // Invert and normalize
      pLight->direction = pLight->position.getInverted().normalize();
      return;
    default:
      setError(GL_INVALID_ENUM);
      return;
  }

  pColor->r = params[0];
  pColor->g = params[1];
  pColor->b = params[2];
  pColor->a = params[3];
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
  switch(pname)
  {
    case GL_SHININESS:
      matShininess_ = param;
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  }
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glMaterialfv(GLenum face, GLenum pname, const GLfloat * params)
{
  switch(pname)
  {
    case GL_AMBIENT:
      matColorAmbient_.r = params[0];
      matColorAmbient_.g = params[1];
      matColorAmbient_.b = params[2];
      matColorAmbient_.a = params[3];
      break;
    case GL_DIFFUSE:
      matColorDiffuse_.r = params[0];
      matColorDiffuse_.g = params[1];
      matColorDiffuse_.b = params[2];
      matColorDiffuse_.a = params[3];
      break;
    case GL_SPECULAR:
      matColorSpecular_.r = params[0];
      matColorSpecular_.g = params[1];
      matColorSpecular_.b = params[2];
      matColorSpecular_.a = params[3];
      break;
    case GL_EMISSION:
      matColorEmission_.r = params[0];
      matColorEmission_.g = params[1];
      matColorEmission_.b = params[2];
      matColorEmission_.a = params[3];
      break;
    case GL_SHININESS:
      matShininess_ = params[0];
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      matColorAmbient_.r = params[0];
      matColorAmbient_.g = params[1];
      matColorAmbient_.b = params[2];
      matColorAmbient_.a = params[3];
      matColorDiffuse_.r = params[0];
      matColorDiffuse_.g = params[1];
      matColorDiffuse_.b = params[2];
      matColorDiffuse_.a = params[3];
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  }
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glShadeModel(GLenum mode)
{
  shadingModel_ = mode;
  bSmoothShading_ = (shadingModel_ == GL_SMOOTH);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
  viewportXOffset    = x;
  viewportYOffset    = y;
  viewportWidth      = width;
  viewportHeight     = height;
  viewportPixelCount = width * height;

  xA_ =     (viewportWidth  >> 1);
  xB_ = x + (viewportWidth  >> 1);
  yA_ =     (viewportHeight >> 1);
  yB_ = y + (viewportHeight >> 1);

  // FIXME:
  yA_  = -yA_;
  xB_ -= x;
  yB_ -= y;

  // Use fixed point format for xy
  xA_ *= (1<<SHIFT_XY);
  xB_ *= (1<<SHIFT_XY);
  yA_ *= (1<<SHIFT_XY);
  yB_ *= (1<<SHIFT_XY);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glBegin(GLenum mode)
{
  if(beginValid_ == true)
  {
    setError(GL_INVALID_OPERATION);
    return;
  }

  switch(mode)
  {
//    case GL_POINTS:
//    case GL_LINES:
//    case GL_LINE_STRIP:
//    case GL_LINE_LOOP:
    case GL_TRIANGLES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_QUADS:
//    case GL_QUAD_STRIP:
    case GL_POLYGON:
      beginValid_ = true;
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  }

  rasterMode_ = mode;

  // Initialize for default triangle
  triangle_[0] = &vertices[0];
  triangle_[1] = &vertices[1];
  triangle_[2] = &vertices[2];
  bFlipFlop_ = true;
  vertIdx_   = 0;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glEnd()
{
  if(beginValid_ == false)
  {
    setError(GL_INVALID_OPERATION);
    return;
  }

  beginValid_ = false;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  if(beginValid_ == false)
  {
    setError(GL_INVALID_OPERATION);
    return;
  }

  SVertexF v;

  // Set vertex
  v.vo.x = x;
  v.vo.y = y;
  v.vo.z = z;
  v.vo.w = w;
  // Set normal
  v.n = normal_;
  // Set color
  v.cl = clCurrent;
  // Set texture
  v.t[0] = texCoordCurrent_[0];
  v.t[1] = texCoordCurrent_[1];
  // Vertex not processed yet
  v.processed = false;

  vertexShaderTransform(v);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
  clCurrent.r = red;
  clCurrent.g = green;
  clCurrent.b = blue;
  clCurrent.a = alpha;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
  texCoordCurrent_[0] = s;
  texCoordCurrent_[1] = t;
  texCoordCurrent_[2] = r;
  texCoordCurrent_[3] = q;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
  normal_.x = nx;
  normal_.y = ny;
  normal_.z = nz;

  if(normalizeEnabled_  == true)
    normal_.normalize();
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glGetFloatv(GLenum pname, GLfloat * params)
{
  // Return 4x4 matrix
#ifdef ROW_MAJOR
  #define GL_GET_MATRIX_COPY(matrix) \
    for(int i(0); i < 4; i++) \
      for(int j(0); j < 4; j++) \
        params[i*4+j] = matrix[j*4+i]
#else
  #define GL_GET_MATRIX_COPY(matrix) \
    for(int i(0); i < 16; i++) \
      params[i] = matrix[i]
#endif

  switch(pname)
  {
    case GL_MATRIX_MODE:
    {
      params[0] = matrixMode_;
      break;
    }
    case GL_MODELVIEW_MATRIX:
    {
      GL_GET_MATRIX_COPY(pCurrentModelView_->matrix);
      break;
    }
    case GL_MAX_MODELVIEW_STACK_DEPTH:
    case GL_MODELVIEW_STACK_DEPTH:
    {
      params[0] = GL_MATRIX_MODELVIEW_STACK_SIZE;
      break;
    }
    case GL_PROJECTION_MATRIX:
    {
      GL_GET_MATRIX_COPY(pCurrentProjection_->matrix);
      break;
    }
    case GL_MAX_PROJECTION_STACK_DEPTH:
    case GL_PROJECTION_STACK_DEPTH:
    {
      params[0] = GL_MATRIX_PROJECTION_STACK_SIZE;
      break;
    }
    case GL_TEXTURE_MATRIX:
    {
      GL_GET_MATRIX_COPY(pCurrentTexture_->matrix);
      break;
    }
    case GL_MAX_TEXTURE_STACK_DEPTH:
    case GL_TEXTURE_STACK_DEPTH:
    {
      params[0] = GL_MATRIX_TEXTURE_STACK_SIZE;
      break;
    }
    default:
      setError(GL_INVALID_ENUM);
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
  blendSFactor_ = sfactor;
  blendDFactor_ = dfactor;

  if((sfactor == GL_ONE) && (dfactor == GL_ZERO))
    blendFast_ = FB_SOURCE;
  else if((sfactor == GL_ZERO) && (dfactor == GL_ONE))
    blendFast_ = FB_DEST;
  else if((sfactor == GL_SRC_ALPHA) && (dfactor == GL_ONE_MINUS_SRC_ALPHA))
    blendFast_ = FB_BLEND;
  else
    blendFast_ = FB_OTHER;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
  if(target != GL_TEXTURE_ENV)
  {
    setError(GL_INVALID_ENUM);
    return;
  }
  if(pname != GL_TEXTURE_ENV_MODE)
  {
    setError(GL_INVALID_ENUM);
    return;
  }

  switch((GLenum)param)
  {
    case GL_MODULATE:
    case GL_DECAL:
    case GL_BLEND:
    case GL_REPLACE:
      texEnvMode_ = (GLenum)param;
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::glTexEnvfv(GLenum target, GLenum pname, const GLfloat * params)
{
  if(target != GL_TEXTURE_ENV)
  {
    setError(GL_INVALID_ENUM);
    return;
  }
  if((pname != GL_TEXTURE_ENV_MODE) && (pname != GL_TEXTURE_ENV_COLOR))
  {
    setError(GL_INVALID_ENUM);
    return;
  }

  switch(pname)
  {
    case GL_TEXTURE_ENV_MODE:
      switch((GLenum)params[0])
      {
        case GL_MODULATE:
        case GL_DECAL:
        case GL_BLEND:
        case GL_REPLACE:
          texEnvMode_ = (GLenum)params[0];
          break;
        default:
          setError(GL_INVALID_ENUM);
          return;
      };
      break;
    case GL_TEXTURE_ENV_COLOR:
      texEnvColor_.r = params[0];
      texEnvColor_.g = params[1];
      texEnvColor_.b = params[2];
      texEnvColor_.a = params[3];
      texEnvColorFX_.r = (int32_t)(texEnvColor_.r * (1 << SHIFT_COLOR));
      texEnvColorFX_.g = (int32_t)(texEnvColor_.g * (1 << SHIFT_COLOR));
      texEnvColorFX_.b = (int32_t)(texEnvColor_.b * (1 << SHIFT_COLOR));
      texEnvColorFX_.a = (int32_t)(texEnvColor_.a * (1 << SHIFT_COLOR));
      break;
    default:
      setError(GL_INVALID_ENUM);
      return;
  };
}

//-----------------------------------------------------------------------------
inline GLfloat
my_pow(GLfloat x, int y)
{
  GLfloat rv(x);
  for(int i(1); i < y; i++)
    rv *= x;
  return rv;
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::vertexShaderTransform(SVertexF & v)
{
  _vertexShaderTransform(v);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::vertexShaderLight(SVertexF & v)
{
  _vertexShaderLight(v);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::fragmentCull(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  _fragmentCull(v0, v1, v2);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::fragmentClip(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  _fragmentClip(v0, v1, v2);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::primitiveAssembly(SVertexF & v)
{
  _primitiveAssembly(v);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  if(bBufVertexEnabled_ == false)
    return;

  GLint idxVertex  (first * bufVertex_.size);
  GLint idxColor   (first * bufColor_.size);
  GLint idxNormal  (first * bufNormal_.size);
  GLint idxTexCoord(first * bufTexCoord_.size);
  SVertexF v;

  glBegin(mode);

  // Process all vertices
  for(GLint i(0); i < count; i++)
  {
    // Vertex
    switch(bufVertex_.type)
    {
      case GL_FLOAT:
        v.vo.x = ((GLfloat *)bufVertex_.pointer)[idxVertex++];
        v.vo.y = ((GLfloat *)bufVertex_.pointer)[idxVertex++];
        v.vo.z = ((GLfloat *)bufVertex_.pointer)[idxVertex++];
        v.vo.w = 1.0f;
        break;
      case GL_FIXED:
        v.vo.x = gl_fptof(((GLfixed *)bufVertex_.pointer)[idxVertex++]);
        v.vo.y = gl_fptof(((GLfixed *)bufVertex_.pointer)[idxVertex++]);
        v.vo.z = gl_fptof(((GLfixed *)bufVertex_.pointer)[idxVertex++]);
        v.vo.w = 1.0f;
        break;
    };

    // Normal
    if(bBufNormalEnabled_ == true)
    {
      switch(bufNormal_.type)
      {
        case GL_FLOAT:
          v.n.x = ((GLfloat *)bufNormal_.pointer)[idxNormal++];
          v.n.y = ((GLfloat *)bufNormal_.pointer)[idxNormal++];
          v.n.z = ((GLfloat *)bufNormal_.pointer)[idxNormal++];
          break;
        case GL_FIXED:
          v.n.x = gl_fptof(((GLfixed *)bufNormal_.pointer)[idxNormal++]);
          v.n.y = gl_fptof(((GLfixed *)bufNormal_.pointer)[idxNormal++]);
          v.n.z = gl_fptof(((GLfixed *)bufNormal_.pointer)[idxNormal++]);
          break;
      };
    }
    else
      v.n = normal_;

    // Color
    if(lightingEnabled_ == false)
    {
      if(bBufColorEnabled_ == true)
      {
        switch(bufColor_.type)
        {
          case GL_FLOAT:
            v.cl.r = ((GLfloat *)bufColor_.pointer)[idxColor++];
            v.cl.g = ((GLfloat *)bufColor_.pointer)[idxColor++];
            v.cl.b = ((GLfloat *)bufColor_.pointer)[idxColor++];
            v.cl.a = ((GLfloat *)bufColor_.pointer)[idxColor++];
            break;
          case GL_FIXED:
            v.cl.r = gl_fptof(((GLfixed *)bufColor_.pointer)[idxColor++]);
            v.cl.g = gl_fptof(((GLfixed *)bufColor_.pointer)[idxColor++]);
            v.cl.b = gl_fptof(((GLfixed *)bufColor_.pointer)[idxColor++]);
            v.cl.a = gl_fptof(((GLfixed *)bufColor_.pointer)[idxColor++]);
            break;
        };
      }
      else
        v.cl = clCurrent;
    }

    // Textures
    if((texturesEnabled_ == true) && (bBufTexCoordEnabled_ == true))
    {
      switch(bufTexCoord_.type)
      {
        case GL_FLOAT:
          v.t[0] =         ((GLfloat *)bufTexCoord_.pointer)[idxTexCoord++];
          v.t[1] = (1.0f - ((GLfloat *)bufTexCoord_.pointer)[idxTexCoord++]);
          break;
        case GL_FIXED:
          v.t[0] =         gl_fptof(((GLfixed *)bufTexCoord_.pointer)[idxTexCoord++]);
          v.t[1] = (1.0f - gl_fptof(((GLfixed *)bufTexCoord_.pointer)[idxTexCoord++]));
          break;
      };
    }

    v.processed = false;
    vertexShaderTransform(v);
  }

  glEnd();
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_vertexShaderTransform(SVertexF & v)
{
  // --------------
  // Transformation
  // --------------
  // Model-View matrix
  //   from 'object coordinates' to 'eye coordinates'
  CALC_OBJ_TO_EYE(v);
  // Projection matrix
  //   from 'eye coordinates' to 'clip coordinates'
  CALC_EYE_TO_CLIP(v);

  // Set clip flags
  setClipFlags(v);

  if(v.clip == 0)
  {
    // Perspective division
    //   from 'clip coordinates' to 'normalized device coordinates'
    CALC_CLIP_TO_NDEV(v);
  }

  primitiveAssembly(v);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_vertexShaderLight(SVertexF & v)
{
  // --------
  // Lighting
  // --------
  if(lightingEnabled_ == true)
  {
    SColorF c(0, 0, 0, 0);

    // Normal Rotation
    pCurrentModelView_->transform3(v.n, v.n2);

    for(int iLight(0); iLight < 8; iLight++)
    {
      if(lights_[iLight].enabled == true)
      {
        // Ambient light (it's everywhere!)
        c += lights_[iLight].ambient * matColorAmbient_;

        // Diffuse light
        GLfloat diffuse = lights_[iLight].direction.dotProduct(v.n2);
        if(diffuse > 0.0f)
        {
          c += lights_[iLight].diffuse * matColorDiffuse_ * diffuse;
        }

        if(matShininess_ >= 0.5f)
        {
          // Specular light
          TVector3<GLfloat> eye(v.ve);
          eye.normalize();
          GLfloat specular = lights_[iLight].direction.getReflection(v.n2).dotProduct(eye);
          if(specular > 0.0f)
          {
            specular = my_pow(specular, (int)(matShininess_ + 0.5f));
            c += lights_[iLight].specular * matColorSpecular_ * specular;
          }
        }
      }
    }

    // Clamp to 0..1
    c.clamp();
    // Multiply vertex color by calculated color
    v.cl = c;
  }

  // ---
  // Fog
  // ---
  if(fogEnabled_ == true)
  {
    GLfloat partFog, partColor;
    partFog = clampf((abs(v.ve.z) - fogStart_) / (fogEnd_ - fogStart_));
    partColor = 1.0f - partFog;
    v.cl = ((v.cl * partColor) + (fogColor_ * partFog)).getClamped();
  }
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_fragmentCull(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  if(v0.clip & v1.clip & v2.clip)
    return; // Not visible

  fragmentClip(v0, v1, v2);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_fragmentClip(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  // ----------------------
  // Vertex shader lighting
  // ----------------------
  if(bSmoothShading_ == true)
  {
    if(v0.processed == false)
    {
      vertexShaderLight(v0);
      v0.processed = true;
    }
    if(v1.processed == false)
    {
      vertexShaderLight(v1);
      v1.processed = true;
    }
  }
  if(v2.processed == false)
  {
    vertexShaderLight(v2);
    v2.processed = true;
  }

  rasterTriangleClip(v0, v1, v2);
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::_primitiveAssembly(SVertexF & v)
{
  // Copy vertex into vertex buffer
  *triangle_[vertIdx_] = v;

  // ------------------
  // Primitive Assembly
  // ------------------
  switch(rasterMode_)
  {
    case GL_TRIANGLES:
    {
      if(vertIdx_ == 2)
        fragmentCull(*triangle_[0], *triangle_[1], *triangle_[2]);
      vertIdx_++;
      if(vertIdx_ > 2)
        vertIdx_ = 0;
      break;
    }
    case GL_TRIANGLE_STRIP:
    {
      if(vertIdx_ == 2)
      {
        fragmentCull(*triangle_[0], *triangle_[1], *triangle_[2]);
        // Swap 3rd with 1st or 2nd vertex pointer
        if(bFlipFlop_ == true)
        {
          SVertexF * pTemp = triangle_[0];
          triangle_[0] = triangle_[2];
          triangle_[2] = pTemp;
        }
        else
        {
          SVertexF * pTemp = triangle_[1];
          triangle_[1] = triangle_[2];
          triangle_[2] = pTemp;
        }
        bFlipFlop_ = !bFlipFlop_;
      }
      else
        vertIdx_++;
      break;
    }
    case GL_POLYGON:
    case GL_TRIANGLE_FAN:
    {
      if(vertIdx_ == 2)
      {
        fragmentCull(*triangle_[0], *triangle_[1], *triangle_[2]);
        // Swap 3rd with 2nd vertex pointer
        SVertexF * pTemp = triangle_[1];
        triangle_[1] = triangle_[2];
        triangle_[2] = pTemp;
      }
      else
        vertIdx_++;
      break;
    }
    case GL_QUADS:
    {
      if(vertIdx_ == 2)
      {
        fragmentCull(*triangle_[0], *triangle_[1], *triangle_[2]);
        // Swap 3rd with 2nd vertex pointer
        SVertexF * pTemp = triangle_[1];
        triangle_[1] = triangle_[2];
        triangle_[2] = pTemp;
        // QUADS are TRIANGLE_FAN with only 2 triangles
        if(bFlipFlop_ == true)
          bFlipFlop_ = false;
        else
          vertIdx_ = 0;
      }
      else
        vertIdx_++;
      break;
    }
  };
}

//-----------------------------------------------------------------------------
// Clipping based on code from TinyGL
// t = -(plane.dotProduct(from) + planeDistance) / plane.dotProduct(delta);
#define clip_func(name,sign,dir,dir1,dir2)         \
inline GLfloat                                     \
name(TVector4<GLfloat> & c, TVector4<GLfloat> & a, TVector4<GLfloat> & b) \
{                                                  \
  TVector4<GLfloat> delta = b - a;                 \
  GLfloat t, den;                                  \
                                                   \
  den = -(sign delta.dir) + delta.w;               \
  if(den == 0)                                     \
    t = 0;                                         \
  else                                             \
    t = (sign a.dir - a.w) / den;                  \
                                                   \
  c.dir1 = a.dir1 + t * delta.dir1;                \
  c.dir2 = a.dir2 + t * delta.dir2;                \
  c.w    = a.w    + t * delta.w;                   \
  c.dir  = sign c.w;                               \
                                                   \
  return t;                                        \
}

clip_func(clip_xmin,-,x,y,z)
clip_func(clip_xmax,+,x,y,z)
clip_func(clip_ymin,-,y,x,z)
clip_func(clip_ymax,+,y,x,z)
clip_func(clip_zmin,-,z,x,y)
clip_func(clip_zmax,+,z,x,y)

GLfloat (*clip_proc[6])(TVector4<GLfloat> &, TVector4<GLfloat> &, TVector4<GLfloat> &) =
{
  clip_xmin,
  clip_xmax,
  clip_ymin,
  clip_ymax,
  clip_zmin,
  clip_zmax
};

#define CLIP_FUNC(plane,c,a,b) clip_proc[plane](c,a,b)

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::rasterTriangleClip(SVertexF & v0, SVertexF & v1, SVertexF & v2, uint32_t clipBit)
{
  uint32_t cc[3] =
  {
    v0.clip,
    v1.clip,
    v2.clip
  };
  uint32_t clipOr = cc[0] | cc[1] | cc[2];
  uint32_t clipAnd;

  if(clipOr == 0)
  {
    // Completely inside
    this->rasterTriangle(v0, v1, v2);
  }
  else
  {
    clipAnd = cc[0] & cc[1] & cc[2];
    if(clipAnd != 0)
      return; // Completely outside

    // find the next clip bit
    while(clipBit < 6 && (clipOr & (1 << clipBit)) == 0)
    {
      clipBit++;
    }

    if(clipBit >= 6)
    {
      // Rounding error?
      return;
    }

    uint32_t clipMask = (1 << clipBit);
    uint32_t clipXor  = (cc[0] ^ cc[1] ^ cc[2]) & clipMask;

    if(clipXor != 0)
    {
      // 1 vertex outside
      SVertexF vNew1;  // Intersection of 1 line
      SVertexF vNew2;  // Intersection of 1 line
      GLfloat tt;

      // Rearrange vertices
      SVertexF * v[3];
           if(cc[0] & clipMask){v[0] = &v0; v[1] = &v1; v[2] = &v2;} // v0 == outside
      else if(cc[1] & clipMask){v[0] = &v1; v[1] = &v2; v[2] = &v0;} // v1 == outside
      else                     {v[0] = &v2; v[1] = &v0; v[2] = &v1;} // v2 == outside

      // Interpolate 1-0
      tt = CLIP_FUNC(clipBit, vNew1.vc, v[1]->vc, v[0]->vc);
      interpolateVertex(vNew1, *v[0], *v[1], tt);
      // Interpolate 2-0
      tt = CLIP_FUNC(clipBit, vNew2.vc, v[2]->vc, v[0]->vc);
      interpolateVertex(vNew2, *v[0], *v[2], tt);

      // Raster 2 new triangles
      this->rasterTriangleClip(vNew1, *v[1], *v[2], clipBit + 1);
      this->rasterTriangleClip(vNew2, vNew1, *v[2], clipBit + 1);
    }
    else
    {
      // 2 vertices outside
      SVertexF vNew1;  // Intersection of 1 line
      SVertexF vNew2;  // Intersection of 1 line
      GLfloat tt;

      // Rearrange vertices
      SVertexF * v[3];
           if((cc[0] & clipMask) == 0){v[0] = &v0; v[1] = &v1; v[2] = &v2;} // v0 == inside
      else if((cc[1] & clipMask) == 0){v[0] = &v1; v[1] = &v2; v[2] = &v0;} // v1 == inside
      else                            {v[0] = &v2; v[1] = &v0; v[2] = &v1;} // v2 == inside

      // Interpolate 0-1
      tt = CLIP_FUNC(clipBit, vNew1.vc, v[0]->vc, v[1]->vc);
      interpolateVertex(vNew1, *v[1], *v[0], tt);
      // Interpolate 0-2
      tt = CLIP_FUNC(clipBit, vNew2.vc, v[0]->vc, v[2]->vc);
      interpolateVertex(vNew2, *v[2], *v[0], tt);

      // Raster new triangle
      this->rasterTriangleClip(*v[0], vNew1, vNew2, clipBit + 1);
    }
  }
}

//-----------------------------------------------------------------------------
void
CASoftGLESFloat::interpolateVertex(SVertexF & c, SVertexF & a, SVertexF & b, GLfloat t)
{
  // Color
  if(bSmoothShading_ == true)
    c.cl = CALC_INTERSECTION(a.cl, b.cl, t);
  else
    c.cl = b.cl;

  // Texture coordinates
  if(texturesEnabled_ == true)
  {
    c.t[0] = CALC_INTERSECTION(a.t[0], b.t[0], t);
    c.t[1] = CALC_INTERSECTION(a.t[1], b.t[1], t);
  }

  // Set clip flags
  setClipFlags(c);

  if(c.clip == 0)
  {
    // Perspective division
    //   from 'clip coordinates' to 'normalized device coordinates'
    CALC_CLIP_TO_NDEV(c);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSoftGLESFloat::CSoftGLESFloat()
 : CASoftGLESFloat()
 , CAGLESTextures()
 , pZBuffer_(NULL)
{
}

//-----------------------------------------------------------------------------
CSoftGLESFloat::~CSoftGLESFloat()
{
  if(pZBuffer_)
    delete pZBuffer_;
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::glClear(GLbitfield mask)
{
  if(mask & GL_COLOR_BUFFER_BIT)
  {
    color_t color = BxColorFormat_FromRGBA(renderSurface->mode.format, (uint8_t)(clClear.r * 255), (uint8_t)(clClear.g * 255), (uint8_t)(clClear.b * 255), (uint8_t)(clClear.a * 255));
    int yoff = renderSurface->mode.height - (viewportHeight + viewportYOffset);
    int xoff = viewportXOffset;

    switch(renderSurface->mode.bpp)
    {
      case 8:
      {
        for(int32_t y(0); y < viewportHeight; y++)
          for(int32_t x(0); x < viewportWidth; x++)
            ((uint8_t  *)renderSurface->p)[(y + yoff) * renderSurface->mode.xpitch + (x + xoff)] = color;
        break;
      }
      case 16:
      {
        for(int32_t y(0); y < viewportHeight; y++)
          for(int32_t x(0); x < viewportWidth; x++)
            ((uint16_t *)renderSurface->p)[(y + yoff) * renderSurface->mode.xpitch + (x + xoff)] = color;
        break;
      }
      case 32:
      {
        uint32_t * pBuffer = (uint32_t *)renderSurface->p;
        for(int i(0); i < viewportPixelCount; i++)
          pBuffer[i] = color;
        break;
      }
    };
  }
  if(mask & GL_DEPTH_BUFFER_BIT)
  {
    if(pZBuffer_ != NULL)
    {
      uint32_t zc = (zClearValue_ << 16) | zClearValue_;
      for(int i(0); i < (viewportPixelCount / 2); i++)
        ((uint32_t *)pZBuffer_)[i] = zc;
    }
  }
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
  CASoftGLESFloat::glViewport(x, y, width, height);

  // Update z-buffer
  if(depthTestEnabled_ == true)
    zbuffer(true);
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterTriangle(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  _rasterTriangle(v0, v1, v2);
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::zbuffer(bool enable)
{
  if(enable == true)
  {
    // (Re)create z-buffer
    if(pZBuffer_ == 0)
    {
      if(pZBuffer_)
        delete pZBuffer_;

      // FIXME: Should be viewportPixelCount
      pZBuffer_ = new uint16_t[(viewportWidth + viewportXOffset) * (viewportHeight + viewportYOffset)];
      if(pZBuffer_ == NULL)
        setError(GL_OUT_OF_MEMORY);
    }
  }
}

//-----------------------------------------------------------------------------
inline bool
CSoftGLESFloat::rasterDepth(uint16_t z_pix, uint16_t z_buf)
{
  switch(depthFunction_)
  {
    case GL_LESS:     return (z_pix <  z_buf); break;
    case GL_EQUAL:    return (z_pix == z_buf); break;
    case GL_LEQUAL:   return (z_pix <= z_buf); break;
    case GL_GREATER:  return (z_pix >  z_buf); break;
    case GL_NOTEQUAL: return (z_pix != z_buf); break;
    case GL_GEQUAL:   return (z_pix >= z_buf); break;
    case GL_ALWAYS:   return true;             break;
    case GL_NEVER:    return false;            break;
  };

  return false;
}

//-----------------------------------------------------------------------------
inline void
CSoftGLESFloat::rasterTexture(SRasterColor & out, const SRasterColor & cfragment, int32_t u, int32_t v, bool near)
{
  bool alphaChannel = pCurrentTex_->bRGBA_;
  SRasterColor ctemp;

  pCurrentTex_->getTexel(ctemp, u, v, near);

  switch(texEnvMode_)
  {
    case GL_REPLACE:
      out.r = ctemp.r;
      out.g = ctemp.g;
      out.b = ctemp.b;
      out.a = alphaChannel ? ctemp.a : cfragment.a;
      break;
    case GL_MODULATE:
      // Texture * Fragment color
      out.r = COLOR_MUL_COMP(cfragment.r, ctemp.r);
      out.g = COLOR_MUL_COMP(cfragment.g, ctemp.g);
      out.b = COLOR_MUL_COMP(cfragment.b, ctemp.b);
      out.a = alphaChannel ? COLOR_MUL_COMP(cfragment.a, ctemp.a) : cfragment.a;
      break;
    case GL_DECAL:
      out.r = alphaChannel ? (COLOR_MUL_COMP(cfragment.r, ((1<<SHIFT_COLOR)-ctemp.a)) + COLOR_MUL_COMP(ctemp.r, ctemp.a)) : ctemp.r;
      out.g = alphaChannel ? (COLOR_MUL_COMP(cfragment.g, ((1<<SHIFT_COLOR)-ctemp.a)) + COLOR_MUL_COMP(ctemp.g, ctemp.a)) : ctemp.g;
      out.b = alphaChannel ? (COLOR_MUL_COMP(cfragment.b, ((1<<SHIFT_COLOR)-ctemp.a)) + COLOR_MUL_COMP(ctemp.b, ctemp.a)) : ctemp.b;
      out.a = cfragment.a;
      break;
    case GL_BLEND:
      out.r = COLOR_MUL_COMP(cfragment.r, ((1<<SHIFT_COLOR)-ctemp.r)) + COLOR_MUL_COMP(texEnvColorFX_.r, ctemp.r);
      out.g = COLOR_MUL_COMP(cfragment.g, ((1<<SHIFT_COLOR)-ctemp.g)) + COLOR_MUL_COMP(texEnvColorFX_.g, ctemp.g);
      out.b = COLOR_MUL_COMP(cfragment.b, ((1<<SHIFT_COLOR)-ctemp.b)) + COLOR_MUL_COMP(texEnvColorFX_.b, ctemp.b);
      out.a = alphaChannel ? COLOR_MUL_COMP(cfragment.a, ctemp.a) : cfragment.a;
      break;
    case GL_ADD:
      out.r = COLOR_ADD_COMP(cfragment.r, ctemp.r);
      out.g = COLOR_ADD_COMP(cfragment.g, ctemp.g);
      out.b = COLOR_ADD_COMP(cfragment.b, ctemp.b);
      out.a = alphaChannel ? COLOR_MUL_COMP(cfragment.a, ctemp.a) : cfragment.a;
      break;
  };
}

//-----------------------------------------------------------------------------
inline void
CSoftGLESFloat::rasterBlend(SRasterColor & out, const SRasterColor & source, const SRasterColor & dest)
{
  #define BLEND_FACTOR(factor,c) \
  switch(factor) \
  { \
    case GL_ONE: \
      c.r = c.g = c.b = c.a = (1<<SHIFT_COLOR); \
      break; \
    case GL_SRC_COLOR: \
      c.r = source.r; \
      c.g = source.g; \
      c.b = source.b; \
      c.a = source.a; \
      break; \
    case GL_ONE_MINUS_SRC_COLOR: \
      c.r = (1<<SHIFT_COLOR) - source.r; \
      c.g = (1<<SHIFT_COLOR) - source.g; \
      c.b = (1<<SHIFT_COLOR) - source.b; \
      c.a = (1<<SHIFT_COLOR) - source.a; \
      break; \
    case GL_DST_COLOR: \
      c.r = dest.r; \
      c.g = dest.g; \
      c.b = dest.b; \
      c.a = dest.a; \
      break; \
    case GL_ONE_MINUS_DST_COLOR: \
      c.r = (1<<SHIFT_COLOR) - dest.r; \
      c.g = (1<<SHIFT_COLOR) - dest.g; \
      c.b = (1<<SHIFT_COLOR) - dest.b; \
      c.a = (1<<SHIFT_COLOR) - dest.a; \
      break; \
    case GL_SRC_ALPHA: \
      c.r = c.g = c.b = c.a = source.a; \
      break; \
    case GL_ONE_MINUS_SRC_ALPHA: \
      c.r = c.g = c.b = c.a = (1<<SHIFT_COLOR) - source.a; \
      break; \
    case GL_DST_ALPHA: \
      c.r = c.g = c.b = c.a = dest.a; \
      break; \
    case GL_ONE_MINUS_DST_ALPHA: \
      c.r = c.g = c.b = c.a = (1<<SHIFT_COLOR) - dest.a; \
      break; \
    case GL_ZERO: \
    default: \
      c.r = c.g = c.b = c.a = 0; \
  }

  switch(blendFast_)
  {
    case FB_SOURCE:
      out.r = source.r;
      out.g = source.g;
      out.b = source.b;
      out.a = source.a;
      break;
    case FB_DEST:
      out.r = dest.r;
      out.g = dest.g;
      out.b = dest.b;
      out.a = dest.a;
      break;
    case FB_BLEND:
    {
      COLOR_AVG(out, dest, source, source.a);
      COLOR_CLAMP(out, out);
      break;
    }
    default:
    {
      SRasterColor cs, cd;
      BLEND_FACTOR(blendSFactor_, cs);
      BLEND_FACTOR(blendDFactor_, cd);

      cd.r = ((source.r * cs.r) + (dest.r * cd.r)) >> SHIFT_COLOR;
      cd.g = ((source.g * cs.g) + (dest.g * cd.g)) >> SHIFT_COLOR;
      cd.b = ((source.b * cs.b) + (dest.b * cd.b)) >> SHIFT_COLOR;
      cd.a = ((source.a * cs.a) + (dest.a * cd.a)) >> SHIFT_COLOR;

      COLOR_CLAMP_TOP(out, cd);
    }
  }
}

//-----------------------------------------------------------------------------
inline void
FloorDivMod(int32_t Numerator, int32_t Denominator, int32_t &Floor, int32_t &Mod)
{
  if(Numerator >= 0)
  {
    // positive case, C is okay
    Floor = Numerator / Denominator;
    Mod   = Numerator % Denominator;
  }
  else
  {
    // Numerator is negative, do the right thing
    Floor = -((-Numerator) / Denominator);
    Mod   =   (-Numerator) % Denominator;
    if(Mod)
    {
      // there is a remainder
      Floor--; Mod = Denominator - Mod;
    }
  }
}

//-----------------------------------------------------------------------------
inline void
fxFloorDivMod(int32_t Numerator, int32_t Denominator, unsigned int shift, int32_t &Floor, int32_t &Mod)
{
  if(Numerator >= 0)
  {
    // positive case, C is okay
    Floor = (((int64_t)Numerator) << shift) / Denominator;
    Mod   = (((int64_t)Numerator) << shift) % Denominator;
  }
  else
  {
    // Numerator is negative, do the right thing
    Floor = -((-(((int64_t)Numerator) << shift)) / Denominator);
    Mod   =   (-(((int64_t)Numerator) << shift)) % Denominator;
    if(Mod)
    {
      // there is a remainder
      Floor--; Mod = Denominator - Mod;
    }
  }
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::_rasterTriangle(SVertexF & v0, SVertexF & v1, SVertexF & v2)
{
  // -------
  // Culling
  // -------
  if(cullFaceEnabled_ == true)
  {
    // Always invisible when culling both front and back
    if(cullFaceMode_ == GL_FRONT_AND_BACK)
      return;

    GLfloat vnz =
      (v0.vd.x - v2.vd.x) * (v1.vd.y - v2.vd.y) -
      (v0.vd.y - v2.vd.y) * (v1.vd.x - v2.vd.x);

    if(vnz == 0.0f)
      return;

    if((vnz < 0.0f) == bCullCW_)
      return;
  }

  SRasterVertex vtx0, vtx1, vtx2;

  vtx0.x   = (int32_t)((xA_ * v0.vd.x) + xB_);
  vtx0.y   = (int32_t)((yA_ * v0.vd.y) + yB_);
  vtx0.z   = (int32_t)((zA_ * v0.vd.z) + zB_);
  vtx0.c.r = (int32_t)(v0.cl.r * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx0.c.g = (int32_t)(v0.cl.g * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx0.c.b = (int32_t)(v0.cl.b * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx0.c.a = (int32_t)(v0.cl.a * (1<<SHIFT_COLOR)); // float   to 1.17.14
#ifdef CONFIG_GL_PERSPECTIVE_CORRECT_TEXTURES
  vtx0.t.u = v0.t[0] * (1 << pCurrentTex_->bitWidth_);
  vtx0.t.v = v0.t[1] * (1 << pCurrentTex_->bitHeight_);
  vtx0.t.w = v0.vd.w;
#else
  vtx0.t.u = (int32_t)(v0.t[0] * (1<<(SHIFT_TEX + pCurrentTex_->bitWidth_)));
  vtx0.t.v = (int32_t)(v0.t[1] * (1<<(SHIFT_TEX + pCurrentTex_->bitHeight_)));
#endif

  vtx1.x   = (int32_t)((xA_ * v1.vd.x) + xB_);
  vtx1.y   = (int32_t)((yA_ * v1.vd.y) + yB_);
  vtx1.z   = (int32_t)((zA_ * v1.vd.z) + zB_);
  vtx1.c.r = (int32_t)(v1.cl.r * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx1.c.g = (int32_t)(v1.cl.g * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx1.c.b = (int32_t)(v1.cl.b * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx1.c.a = (int32_t)(v1.cl.a * (1<<SHIFT_COLOR)); // float   to 1.17.14
#ifdef CONFIG_GL_PERSPECTIVE_CORRECT_TEXTURES
  vtx1.t.u = v1.t[0] * (1 << pCurrentTex_->bitWidth_);
  vtx1.t.v = v1.t[1] * (1 << pCurrentTex_->bitHeight_);
  vtx1.t.w = v1.vd.w;
#else
  vtx1.t.u = (int32_t)(v1.t[0] * (1<<(SHIFT_TEX + pCurrentTex_->bitWidth_)));
  vtx1.t.v = (int32_t)(v1.t[1] * (1<<(SHIFT_TEX + pCurrentTex_->bitHeight_)));
#endif

  vtx2.x   = (int32_t)((xA_ * v2.vd.x) + xB_);
  vtx2.y   = (int32_t)((yA_ * v2.vd.y) + yB_);
  vtx2.z   = (int32_t)((zA_ * v2.vd.z) + zB_);
  vtx2.c.r = (int32_t)(v2.cl.r * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx2.c.g = (int32_t)(v2.cl.g * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx2.c.b = (int32_t)(v2.cl.b * (1<<SHIFT_COLOR)); // float   to 1.17.14
  vtx2.c.a = (int32_t)(v2.cl.a * (1<<SHIFT_COLOR)); // float   to 1.17.14
#ifdef CONFIG_GL_PERSPECTIVE_CORRECT_TEXTURES
  vtx2.t.u = v2.t[0] * (1 << pCurrentTex_->bitWidth_);
  vtx2.t.v = v2.t[1] * (1 << pCurrentTex_->bitHeight_);
  vtx2.t.w = v2.vd.w;
#else
  vtx2.t.u = (int32_t)(v2.t[0] * (1<<(SHIFT_TEX + pCurrentTex_->bitWidth_)));
  vtx2.t.v = (int32_t)(v2.t[1] * (1<<(SHIFT_TEX + pCurrentTex_->bitHeight_)));
#endif

  // Calculate function code
  uint32_t function_code =
    (blendingEnabled_ << 3) |
    (texturesEnabled_ << 2) |
    (bSmoothShading_  << 1) |
    (depthTestEnabled_    );

  // We don't need smooth shading if all 3 colors are the same
  if((bSmoothShading_ == true) &&
     (vtx0.c.r == vtx1.c.r) && (vtx1.c.r == vtx2.c.r) &&
     (vtx0.c.g == vtx1.c.g) && (vtx1.c.g == vtx2.c.g) &&
     (vtx0.c.b == vtx1.c.b) && (vtx1.c.b == vtx2.c.b) &&
     (vtx0.c.a == vtx1.c.a) && (vtx1.c.a == vtx2.c.a))
  {
    // Disable smooth shading for this triangle
    function_code &= ~(bSmoothShading_ << 1);
  }

  // Bubble sort the 3 vertexes
  const SRasterVertex * vhi(&vtx0);
  const SRasterVertex * vmi(&vtx1);
  const SRasterVertex * vlo(&vtx2);
  {
    const SRasterVertex * vtemp;
    // Swap bottom with middle?
    if(vlo->y > vmi->y)
    {
      vtemp = vmi;
      vmi   = vlo;
      vlo   = vtemp;
    }
    // Swap middle with top?
    if(vmi->y > vhi->y)
    {
      vtemp = vhi;
      vhi   = vmi;
      vmi   = vtemp;
      // Swap bottom with middle again?
      if(vlo->y > vmi->y)
      {
        vtemp = vmi;
        vmi   = vlo;
        vlo   = vtemp;
      }
    }
  }

  switch(function_code)
  {
    case 0x00: raster    (vlo, vmi, vhi); break;
    case 0x01: rasterZ   (vlo, vmi, vhi); break;
    case 0x02: rasterC   (vlo, vmi, vhi); break;
    case 0x03: rasterCZ  (vlo, vmi, vhi); break;
    case 0x04: rasterT   (vlo, vmi, vhi); break;
    case 0x05: rasterTZ  (vlo, vmi, vhi); break;
    case 0x06: rasterTC  (vlo, vmi, vhi); break;
    case 0x07: rasterTCZ (vlo, vmi, vhi); break;
    case 0x08: rasterB   (vlo, vmi, vhi); break;
    case 0x09: rasterBZ  (vlo, vmi, vhi); break;
    case 0x0a: rasterBC  (vlo, vmi, vhi); break;
    case 0x0b: rasterBCZ (vlo, vmi, vhi); break;
    case 0x0c: rasterBT  (vlo, vmi, vhi); break;
    case 0x0d: rasterBTZ (vlo, vmi, vhi); break;
    case 0x0e: rasterBTC (vlo, vmi, vhi); break;
    case 0x0f: rasterBTCZ(vlo, vmi, vhi); break;

    default:
      rasterC(vlo, vmi, vhi);
  };
}

//-----------------------------------------------------------------------------
template <class T>
struct TInterpolation
{
  T current;
  T increment;
};

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::raster(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterC(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_SMOOTH_COLORS

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterCZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_SMOOTH_COLORS

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterT(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_TEXTURES

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterTZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_TEXTURES

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterTC(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_TEXTURES

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterTCZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_TEXTURES

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterB(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBC(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBCZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBT(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_TEXTURES
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBTZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_TEXTURES
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBTC(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_TEXTURES
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}

//-----------------------------------------------------------------------------
void
CSoftGLESFloat::rasterBTCZ(const SRasterVertex * vlo, const SRasterVertex * vmi, const SRasterVertex * vhi)
{
  #define RASTER_ENABLE_DEPTH_TEST
  #define RASTER_ENABLE_SMOOTH_COLORS
  #define RASTER_ENABLE_TEXTURES
  #define RASTER_ENABLE_BLENDING

  #include "raster.h"
}
