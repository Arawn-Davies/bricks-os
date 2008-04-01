#include "kernel/2dRenderer.h"
#include "string.h"
#include "stdlib.h"


//---------------------------------------------------------------------------
#define SET_PIXEL(x,y,c) \
{ \
  switch(pSurface_->mode.bpp) \
  { \
    case 8: \
      ((uint8_t  *)pSurface_->p)[y * pSurface_->mode.xpitch + x] = c; \
      break; \
    case 16: \
      ((uint16_t *)pSurface_->p)[y * pSurface_->mode.xpitch + x] = c; \
      break; \
    case 32: \
      ((uint32_t *)pSurface_->p)[y * pSurface_->mode.xpitch + x] = c; \
      break; \
    default: \
      ; \
  }; \
}

//---------------------------------------------------------------------------
#define VISIBLE_POINT(x,y) ((x >= 0) && (x < (int)pSurface_->mode.width) && (y >= 0) && (y < (int)pSurface_->mode.height))

//---------------------------------------------------------------------------
#define VISIBLE_RECT(x,y,w,h) ((x < (int)pSurface_->mode.width) && ((x + w - 1) >= 0) && (y < (int)pSurface_->mode.height) && ((y + h - 1) >= 0))

//---------------------------------------------------------------------------
#define CLIP_POINT(x,y) \
{ \
  if(x < 0) x = 0; \
  if(y < 0) y = 0; \
  else if((unsigned int)x >= pSurface_->mode.width)  x = pSurface_->mode.width - 1; \
  else if((unsigned int)y >= pSurface_->mode.height) y = pSurface_->mode.height -1; \
}

//---------------------------------------------------------------------------
// Make sure the rect is visible first, using 'VISIBLE(x,y,w,h);'
// Otherwise this macro makes no sense
#define CLIP_RECT(x,y,w,h) \
{ \
  if(x < 0) {w -= -x; x = 0;} \
  if(y < 0) {h -= -y; y = 0;} \
  if((x + w) > pSurface_->mode.width)  {w = pSurface_->mode.width  - x;} \
  if((y + h) > pSurface_->mode.height) {h = pSurface_->mode.height - y;} \
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
C2DRenderer::C2DRenderer(CSurface * surf)
 : pSurface_(surf)
{
  setColor(0, 0, 0);
}

//---------------------------------------------------------------------------
C2DRenderer::~C2DRenderer()
{
}

//---------------------------------------------------------------------------
void
C2DRenderer::setSurface(CSurface * surface)
{
  pSurface_ = surface;

  setColor(color_.r, color_.g, color_.b);
}

//---------------------------------------------------------------------------
CSurface *
C2DRenderer::getSurface()
{
  return pSurface_;
}

//---------------------------------------------------------------------------
void
C2DRenderer::flush()
{
}

//---------------------------------------------------------------------------
void
C2DRenderer::setColor(color_t rgb)
{
  color_.r = BxColorFormat_GetR(cfA8R8G8B8, rgb);
  color_.g = BxColorFormat_GetG(cfA8R8G8B8, rgb);
  color_.b = BxColorFormat_GetB(cfA8R8G8B8, rgb);
  color_.a = BxColorFormat_GetA(cfA8R8G8B8, rgb);

  if(pSurface_ != NULL)
    fmtColor_ = BxColorFormat_FromRGB(pSurface_->mode.format, color_.r, color_.g, color_.b);
}

//---------------------------------------------------------------------------
void
C2DRenderer::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  color_.r = r;
  color_.g = g;
  color_.b = b;
  color_.a = 255;

  if(pSurface_ != NULL)
    fmtColor_ = BxColorFormat_FromRGB(pSurface_->mode.format, color_.r, color_.g, color_.b);
}

//---------------------------------------------------------------------------
void
C2DRenderer::setPixel(int x, int y)
{
  if(pSurface_ != NULL)
  {
    if(VISIBLE_POINT(x, y))
      setPixel_i(x, y);
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::fill()
{
  if(pSurface_ != NULL)
  {
    fill_i();
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::fillRect(int x, int y, unsigned int width, unsigned int height)
{
  if(pSurface_ != NULL)
  {
    if(VISIBLE_RECT(x, y, width, height))
    {
      CLIP_RECT(x, y, width, height);
      fillRect_i(x, y, width, height);
    }
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawLine(int x1, int y1, int x2, int y2)
{
  if(pSurface_ != NULL)
  {
    // FIXME: Are we visible?
    CLIP_POINT(x1, y1);
    CLIP_POINT(x2, y2);

    // order: Smallest x first
    if(x1 <= x2)
      drawLine_i(x1, y1, x2, y2);
    else
      drawLine_i(x2, y2, x1, y1);
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawHLine(int x, int y, unsigned int width)
{
  if(pSurface_ != NULL)
  {
    // FIXME: Are we visible?
    CLIP_POINT(x, y);

    drawHLine_i(x, y, width);
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawVLine(int x, int y, unsigned int height)
{
  if(pSurface_ != NULL)
  {
    // FIXME: Are we visible?
    CLIP_POINT(x, y);

    drawVLine_i(x, y, height);
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawRect(int x, int y, unsigned int width, unsigned int height)
{
  if(pSurface_ != NULL)
  {
    if(VISIBLE_RECT(x, y, width, height))
    {
      CLIP_RECT(x, y, width, height);
      drawRect_i(x, y, width, height);
    }
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::setPixel_i(int x, int y)
{
  SET_PIXEL(x, y, fmtColor_);
}

//---------------------------------------------------------------------------
void
C2DRenderer::fill_i()
{
  fillRect_i(0, 0, pSurface_->mode.width, pSurface_->mode.height);
}

//---------------------------------------------------------------------------
void
C2DRenderer::fillRect_i(int x, int y, unsigned int width, unsigned int height)
{
  for(uint32_t i(0); i < height; i++)
    drawHLine_i(x, y + i, width);
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawLine_i(int x1, int y1, int x2, int y2)
{
  if(y1 == y2)
  {
    drawHLine_i(x1, y1, x2 - x1 + 1);
  }
  else if(x1 == x2)
  {
    drawVLine_i(x1, y1, y2 - y1 + 1);
  }
  else
  {
    float dx = (x2-x1);
    float dy = (y2-y1);

    if(dx >= abs(dy))
    {
      float currenty = y1 + 0.5f;
      float slopey   = dy / dx;

      for(; x1 <= x2; x1++)
      {
        SET_PIXEL(x1, (int)currenty, fmtColor_);
        currenty += slopey;
      }
    }
    else
    {
      float currentx = x1 + 0.5f;
      float slopex   = dx / dy;
      int ystart = (y1 <= y2) ? y1 : y2;
      int yend   = (y1 <= y2) ? y2 : y1;

      for(; ystart <= yend; ystart++)
      {
        SET_PIXEL((int)currentx, ystart, fmtColor_);
        currentx += slopex;
      }
    }
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawHLine_i(int x, int y, unsigned int width)
{
  while(width > 0)
  {
    SET_PIXEL(x, y, fmtColor_);
    x++;
    width--;
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawVLine_i(int x, int y, unsigned int height)
{
  while(height > 0)
  {
    SET_PIXEL(x, y, fmtColor_);
    y++;
    height--;
  }
}

//---------------------------------------------------------------------------
void
C2DRenderer::drawRect_i(int x, int y, unsigned int width, unsigned int height)
{
  if((width < 3) || (height < 3))
  {
    if((width == 0) || (height == 0))
      return;
    else if(width == 1)
      drawVLine_i(x, y, height);
    else if(height == 1)
      drawHLine_i(x, y, width);
    else if(width == 2)
    {
      drawVLine_i(x,     y, height);
      drawVLine_i(x + 1, y, height);
    }
    else if(height == 2)
    {
      drawHLine_i(x, y,     width);
      drawHLine_i(x, y + 1, width);
    }
  }
  else
  {
    drawHLine_i(x, y, width);                      // Top
    drawHLine_i(x, y + height - 1, width);         // Bottom
    drawVLine_i(x, y + 1, height - 2);             // Left
    drawVLine_i(x + width - 1, y + 1, height - 2); // Right
  }
}
