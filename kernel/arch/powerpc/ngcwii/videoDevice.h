#ifndef ARCH_VIDEODEVICE_H
#define ARCH_VIDEODEVICE_H


#include "kernel/videoManager.h"
#include "kernel/2dRenderer.h"
#include "../../../../gl/softGLF.h"


//---------------------------------------------------------------------------
class CNGCVideoDevice;

//---------------------------------------------------------------------------
class CNGC2DRenderer
 : public C2DRenderer
{
public:
  CNGC2DRenderer(CNGCVideoDevice * dev);
  virtual ~CNGC2DRenderer();

  // Flush operations to surface
  virtual void flush();

private:
  CNGCVideoDevice * pDev_;
};

//-----------------------------------------------------------------------------
class CNGC3DRenderer
 : public CSoftGLESFloat
{
public:
  CNGC3DRenderer(CNGCVideoDevice * dev);
  virtual ~CNGC3DRenderer();

  // Flush operations to surface
  virtual void flush();
  virtual void glFlush(){flush();}

private:
  CNGCVideoDevice * pDev_;
};

//---------------------------------------------------------------------------
class CNGCVideoDevice
 : public CAVideoDevice
{
public:
  CNGCVideoDevice();
  virtual ~CNGCVideoDevice();

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

  // Flush RGB buffer to native screen
  void flush(CSurface * surface);

  void setHorizontal(uint16_t xoffset, uint16_t width);
  void setVertical  (uint16_t yoffset, uint16_t height);
  void setInterlaced(bool interlaced);

  // Screen positioning (in pixels)
  virtual bool     hasPositioning();
  virtual uint16_t getMaxHorizontalOffset();
  virtual uint16_t getHorizontalOffset();
  virtual void     setHorizontalOffset(uint16_t x);
  virtual uint16_t getMaxVerticalOffset();
  virtual uint16_t getVerticalOffset();
  virtual void     setVerticalOffset(uint16_t y);

private:
  // Surface we're currently displaying
  CSurface * pSurface_;

  // Current Native surface (RGB mode only)
  uint8_t * pSurfaceData_;
  void * pNativeSurface_;

  const SVideoMode * pCurrentMode_;

  vuint32_t iFrameCount_; // volatile, becouse the isr updates it
  uint16_t iCurrentHOffset_;
  uint16_t iCurrentVOffset_;
};


#endif
