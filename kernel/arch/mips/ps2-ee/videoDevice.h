#ifndef PS2_VIDEODEVICE_H
#define PS2_VIDEODEVICE_H


#include "kernel/videoManager.h"
#include "kernel/fs.h"
#include "gif.h"


//---------------------------------------------------------------------------
#define GS_X_BASE 1024
#define GS_Y_BASE 1024


struct SPS2CRTCMode
{
  uint8_t biosMode;
  uint8_t vck;
};

//---------------------------------------------------------------------------
struct SPS2VideoMode
{
  uint16_t width;
  uint16_t height;
  uint16_t bpp;
  uint16_t psm;
  uint16_t interlace;
  uint16_t field;
  const SPS2CRTCMode * crtcMode;
  uint16_t xoffset;
  uint16_t yoffset;
};

//---------------------------------------------------------------------------
class CPS2Surface
 : public CSurface
{
public:
  CPS2Surface();
  virtual ~CPS2Surface();

public:
  uint16_t psm_;
};

//---------------------------------------------------------------------------
class CAPS2Renderer
 : public virtual IRenderer
{
public:
  CAPS2Renderer(CSurface * surf = 0);
  ~CAPS2Renderer();

  // Surfaces
  virtual void       setSurface(CSurface * surface);
  virtual CSurface * getSurface();

  // Flush operations to surface
  virtual void       flush();

protected:
  // Surface we're currently rendering on
  CPS2Surface * pSurface_;

  // GIF packet for DMA transmission to GS
  CGIFPacket    packet_;

  // Data for DMA transfer to GS
  bool bDataWaiting_;
};

//---------------------------------------------------------------------------
class CPS22DRenderer
 : public I2DRenderer
 , public CAPS2Renderer
{
public:
  CPS22DRenderer(CSurface * surf = 0);
  virtual ~CPS22DRenderer();

  // Surfaces
  virtual void       setSurface(CSurface * surface){CAPS2Renderer::setSurface(surface);}
  virtual CSurface * getSurface()                  {return CAPS2Renderer::getSurface();}

  // Flush operations to surface
  virtual void       flush()                       {CAPS2Renderer::flush();}

  // Color
  virtual void       setColor(color_t rgb); // cfA8R8G8B8 format color
  virtual void       setColor(uint8_t r, uint8_t g, uint8_t b);

  // Drawing
  virtual void       setPixel(int x, int y);
  virtual void       fill();
  virtual void       fillRect(int x, int y, unsigned int width, unsigned int height);
  virtual void       drawLine(int x1, int y1, int x2, int y2);
  virtual void       drawHLine(int x, int y, unsigned int width);
  virtual void       drawVLine(int x, int y, unsigned int height);
  virtual void       drawRect(int x, int y, unsigned int width, unsigned int height);

private:
  // Current drawing color
  SColor color_;
};

//---------------------------------------------------------------------------
class CPS2VideoDevice
 : public CAVideoDevice
{
public:
  CPS2VideoDevice();
  virtual ~CPS2VideoDevice();

  virtual void listModes(const SVideoMode ** modes, int * modeCount);
  virtual void getCurrentMode(const SVideoMode ** mode);
  virtual void getDefaultMode(const SVideoMode ** mode);
  virtual void setMode(const SVideoMode * mode);

  virtual void getSurface(CSurface ** surface, int width, int height);
  virtual void get2DRenderer(I2DRenderer ** renderer);
  virtual void get3DRenderer(I3DRenderer ** renderer);

  virtual uint32_t getFrameNr(); // Returns current frame/field number
  virtual uint32_t waitVSync(); // Returns current frame/field number, after vsync
  virtual void displaySurface(CSurface * surface);

  virtual void bitBlt(CSurface * dest, int dx, int dy, int w, int h, CSurface * source, int sx, int sy);

private:
  // Surface we're currently displaying
  CPS2Surface * pSurface_;

  // GIF packet for DMA transmission to GS
  CGIFPacket    packet_;

  const SVideoMode    * pCurrentMode_;
  const SPS2VideoMode * pCurrentPS2Mode_;

  vuint32_t iFrameCount_; // volatile, becouse the isr updates it
};


#endif