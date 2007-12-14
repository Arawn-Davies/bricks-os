#include "video.h"
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>


static const SVideoMode videoModes[] =
{
  {512, 272, 480, 272, 32, cfA8B8G8R8},
};
static const int videoModeCount(sizeof(videoModes) / sizeof(SVideoMode));


// -----------------------------------------------------------------------------
CPSPVideo::CPSPVideo()
{
  // Don't use constructor, use init function instead
}

// -----------------------------------------------------------------------------
CPSPVideo::~CPSPVideo()
{
}

// -----------------------------------------------------------------------------
int
CPSPVideo::init()
{
  pspDebugScreenInit();

  return 0;
}

// -----------------------------------------------------------------------------
ssize_t
CPSPVideo::write(const void * buffer, size_t size, loff_t *)
{
  for(size_t i(0); i < size; i++)
    pspDebugScreenPrintf("%c", ((const char *)buffer)[i]);

  return 0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CPSPVideoDevice::CPSPVideoDevice()
 : CAVideoDevice()
 , pSurface_(NULL)
 , pCurrentMode_(NULL)
{
}

//---------------------------------------------------------------------------
CPSPVideoDevice::~CPSPVideoDevice()
{
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::listModes(const SVideoMode ** modes, int * modeCount)
{
  *modes = videoModes;
  *modeCount = videoModeCount;
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::getMode(SVideoMode ** mode)
{
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::setMode(const SVideoMode * mode)
{
  pCurrentMode_ = mode;
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::getSurface(CSurface ** surface, ESurfaceType type)
{
  CSurface * pSurface = 0;

  switch(type)
  {
    case stSCREEN:
    {
      pSurface = new CSurface;
      pSurface->mode = *pCurrentMode_;
      pSurface->p = (uint16_t *)0x04000000;
      *surface = pSurface;
      break;
    }
    default:
      ;
  };

  *surface = pSurface;
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::getRenderer(I2DRenderer ** renderer)
{
  *renderer = new C2DRenderer;
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::waitVSync()
{
  sceDisplayWaitVblankStart();
}

//---------------------------------------------------------------------------
void
CPSPVideoDevice::displaySurface(CSurface * surface)
{
  if(vSync_ == true)
  {
    sceDisplayWaitVblankStart();
  }

  // Set new surface
  if(surface != NULL)
  {
    pSurface_ = surface;
  }
}