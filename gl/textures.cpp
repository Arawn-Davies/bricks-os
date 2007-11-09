#include "textures.h"
#include "unistd.h"


//-----------------------------------------------------------------------------
CAGLESTextures::CAGLESTextures()
 : pCurrentTex_(0)
{
  // Initialize all texture structs
  for(GLuint idxTex(0); idxTex < MAX_TEXTURE_COUNT; idxTex++)
  {
    textures_[idxTex].used = false;
    textures_[idxTex].data = NULL;
  }
}

//-----------------------------------------------------------------------------
CAGLESTextures::~CAGLESTextures()
{
  // Clear all texture structs
  for(GLsizei idxTex(0); idxTex < MAX_TEXTURE_COUNT; idxTex++)
  {
    textures_[idxTex].used = false;
    if(textures_[idxTex].data != NULL)
      delete ((uint8_t *)textures_[idxTex].data);
  }
}

//-----------------------------------------------------------------------------
void
CAGLESTextures::glBindTexture(GLenum target, GLuint texture)
{
  if((target == GL_TEXTURE_2D) && (texture < MAX_TEXTURE_COUNT))
    pCurrentTex_ = &textures_[texture];
}

//-----------------------------------------------------------------------------
void
CAGLESTextures::glDeleteTextures(GLsizei n, const GLuint *textures)
{
  for(GLsizei idxNr(0); idxNr < n; idxNr++)
  {
    if(textures[idxNr] < MAX_TEXTURE_COUNT)
    {
      textures_[textures[idxNr]].used = false;
      if(textures_[textures[idxNr]].data != NULL)
        delete ((uint8_t *)textures_[textures[idxNr]].data);
    }
  }
}

//-----------------------------------------------------------------------------
void
CAGLESTextures::glGenTextures(GLsizei n, GLuint *textures)
{
  GLuint idxTex(0);

  for(GLsizei idxNr(0); idxNr < n; idxNr++)
  {
    bool bFound(false);

    for(; idxTex < MAX_TEXTURE_COUNT; idxTex++)
    {
      if(textures_[idxTex].used == false)
      {
        textures_[idxTex].used = true;
        textures[idxNr] = idxTex;
        bFound = true;
        break;
      }
    }

    if(bFound = false)
      break;
  }
}

//-----------------------------------------------------------------------------
void
CAGLESTextures::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  if(target == GL_TEXTURE_2D)
  {
    if((pCurrentTex_ != 0) && (pCurrentTex_->used == true))
    {
      switch(width)
      {
        case    8: pCurrentTex_->maskWidth = 0x00000007; break;
        case   16: pCurrentTex_->maskWidth = 0x0000000f; break;
        case   32: pCurrentTex_->maskWidth = 0x0000001f; break;
        case   64: pCurrentTex_->maskWidth = 0x0000003f; break;
        case  128: pCurrentTex_->maskWidth = 0x0000007f; break;
        case  256: pCurrentTex_->maskWidth = 0x000000ff; break;
        case  512: pCurrentTex_->maskWidth = 0x000001ff; break;
        case 1024: pCurrentTex_->maskWidth = 0x000003ff; break;
        default:
          return; // ERROR, invalid width
      };
      switch(height)
      {
        case    8: pCurrentTex_->maskHeight = 0x00000007; break;
        case   16: pCurrentTex_->maskHeight = 0x0000000f; break;
        case   32: pCurrentTex_->maskHeight = 0x0000001f; break;
        case   64: pCurrentTex_->maskHeight = 0x0000003f; break;
        case  128: pCurrentTex_->maskHeight = 0x0000007f; break;
        case  256: pCurrentTex_->maskHeight = 0x000000ff; break;
        case  512: pCurrentTex_->maskHeight = 0x000001ff; break;
        case 1024: pCurrentTex_->maskHeight = 0x000003ff; break;
        default:
          return; // ERROR, invalid height
      };

      pCurrentTex_->width  = width;
      pCurrentTex_->height = height;

      EColorFormat fmtTo = renderSurface->format_;
      EColorFormat fmtFrom;
      switch(type)
      {
        case GL_UNSIGNED_BYTE:          fmtFrom = cfA8B8G8R8; break;
        case GL_UNSIGNED_SHORT_5_6_5:   fmtFrom = cfR5G6B5;   break;
        case GL_UNSIGNED_SHORT_4_4_4_4: fmtFrom = cfA4B4G4R4; break;
        case GL_UNSIGNED_SHORT_5_5_5_1: fmtFrom = cfA1B5G5R5; break;
        default:
          return; // ERROR, invalid type
      };

      // Delete old texture buffer if present
      if(pCurrentTex_->data != NULL)
        delete ((uint8_t *)pCurrentTex_->data);
      // Allocate texture buffer
      switch(renderSurface->bpp_)
      {
        case 8:  pCurrentTex_->data = new uint8_t[width*height];  break;
        case 16: pCurrentTex_->data = new uint16_t[width*height]; break;
        case 32: pCurrentTex_->data = new uint32_t[width*height]; break;
        default:
          return; // ERROR, invalid render surface
      };

      // Copy texture, change to surface pixel format
      for(int i(0); i < (width*height); i++)
      {
        uint32_t px(0);

        // Get pixel
        switch(type)
        {
          case GL_UNSIGNED_BYTE:          px = ((uint32_t *)pixels)[i]; break;
          case GL_UNSIGNED_SHORT_5_6_5:   px = ((uint16_t *)pixels)[i]; break;
          case GL_UNSIGNED_SHORT_4_4_4_4: px = ((uint16_t *)pixels)[i]; break;
          case GL_UNSIGNED_SHORT_5_5_5_1: px = ((uint16_t *)pixels)[i]; break;
        };

        // FIXME: Convert pixel
        if(fmtTo != fmtFrom)
          px = px;

        // Put pixel
        switch(renderSurface->bpp_)
        {
          case 8:  ((uint8_t  *)pCurrentTex_->data)[i] = px; break;
          case 16: ((uint16_t *)pCurrentTex_->data)[i] = px; break;
          case 32: ((uint32_t *)pCurrentTex_->data)[i] = px; break;
        };
      }
    }
  }
}
