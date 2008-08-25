#include "GL/glut.h"
#include "GLES/gl.h"
#include "kernel/debug.h"
#include "kernel/videoManager.h"
#include "stdlib.h"


extern void glMakeCurrent(I3DRenderer * ctx);


// Selected device
CAVideoDevice * pGlutDevice = NULL;

// Selected mode
const SVideoMode * pGlutMode = NULL;

// 2 surfaces for double buffering
CSurface * pGlutSurfaceA = NULL;
CSurface * pGlutSurfaceB = NULL;
bool bGlutActiveDisplay = false;

// 3D rendering device
I3DRenderer * pGlutRenderer;


typedef void (* glutDisplayFunction)(void);
typedef void (* glutReshapeFunction)(int, int);
typedef void (* glutKeyboardFunction)(unsigned char, int, int);
typedef void (* glutSpecialFunction)(int, int, int);
glutDisplayFunction  fpGlutDisplay  = NULL;
glutReshapeFunction  fpGlutReshape  = NULL;
glutKeyboardFunction fpGlutKeyboard = NULL;
glutSpecialFunction  fpGlutSpecial  = NULL;


// -----------------------------------------------------------------------------
void
glutInit(int * pargc, char ** argv)
{
  CAVideoDevice ** devices;
  int iDeviceCount;

  videoManager.listDevices(&devices, &iDeviceCount);
  if(iDeviceCount > 0)
  {
    pGlutDevice = devices[0];

    pGlutDevice->getDefaultMode(&pGlutMode);
    if(pGlutMode != NULL)
    {
      // Set default video mode
      pGlutDevice->setMode(pGlutMode);

      // Allocate two surfaces for double buffering
      pGlutDevice->getSurface(&pGlutSurfaceA, pGlutMode->width, pGlutMode->height);
      pGlutDevice->getSurface(&pGlutSurfaceB, pGlutMode->width, pGlutMode->height);

      // Allocate renderer and activate it
      pGlutDevice->get3DRenderer(&pGlutRenderer);
      glMakeCurrent(pGlutRenderer);

      pGlutDevice->setVSync(false);
      pGlutRenderer->setSurface(pGlutSurfaceA);
      pGlutDevice->displaySurface(pGlutSurfaceA);
    }
    else
      printk("ERROR: Device has no modes!\n");
  }
  else
    printk("ERROR: No video devices!\n");
}

// -----------------------------------------------------------------------------
void
glutDisplayFunc(void (* callback)(void))
{
  fpGlutDisplay = callback;
}

// -----------------------------------------------------------------------------
void
glutReshapeFunc(void (* callback)(int, int))
{
  fpGlutReshape = callback;
}

// -----------------------------------------------------------------------------
void
glutKeyboardFunc(void (* callback)(unsigned char, int, int))
{
  fpGlutKeyboard = callback;
}

// -----------------------------------------------------------------------------
void
glutSpecialFunc(void (* callback)(int, int, int))
{
  fpGlutSpecial = callback;
}

// -----------------------------------------------------------------------------
void
glutProcessKeys()
{
  bool bUp(false), bDown(false), bLeft(false), bRight(false);

#ifdef GBA
  uint16_t data;
  data   = ~REG_KEYS;
  bUp    = (data & KEY_UP);
  bDown  = (data & KEY_DOWN);
  bLeft  = (data & KEY_LEFT);
  bRight = (data & KEY_RIGHT);
#endif
#ifdef NGC
  uint32_t datah, datal;
  datah  = REG_SI_CHANNEL0_INBUFH;
  datal  = REG_SI_CHANNEL0_INBUFL;
  bUp    = (datah & KEY_UP);
  bDown  = (datah & KEY_DOWN);
  bLeft  = (datah & KEY_LEFT);
  bRight = (datah & KEY_RIGHT);
#endif

  if(bUp)
    fpGlutSpecial(GLUT_KEY_UP, 0, 0);
  if(bDown)
    fpGlutSpecial(GLUT_KEY_DOWN, 0, 0);
  if(bLeft)
    fpGlutSpecial(GLUT_KEY_LEFT, 0, 0);
  if(bRight)
    fpGlutSpecial(GLUT_KEY_RIGHT, 0, 0);
}

// -----------------------------------------------------------------------------
void
glutMainLoop(void)
{
  if((fpGlutDisplay == NULL) || (fpGlutReshape == NULL))
    return;

  fpGlutReshape(pGlutMode->width, pGlutMode->height);

  while(true)
  {
    if(fpGlutSpecial != NULL)
      glutProcessKeys();

    fpGlutDisplay();
  }
}

// -----------------------------------------------------------------------------
void
glutSwapBuffers(void)
{
  pGlutRenderer->setSurface(bGlutActiveDisplay ? pGlutSurfaceA : pGlutSurfaceB);
  pGlutDevice->displaySurface(bGlutActiveDisplay ? pGlutSurfaceB : pGlutSurfaceA);
  bGlutActiveDisplay = !bGlutActiveDisplay;
}
