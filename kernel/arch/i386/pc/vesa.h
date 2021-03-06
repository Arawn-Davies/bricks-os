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


#ifndef I386_VESA_H
#define I386_VESA_H


#include "asm/task.h"
#include "kernel/videoManager.h"
#include "kernel/2dRenderer.h"
#include "stdint.h"


//---------------------------------------------------------------------------
struct SVBEInfo
{
  char     sig[4];
  uint8_t  ver_minor;
  uint8_t  ver_major;
  uint16_t oem_name_off;
  uint16_t oem_name_seg;
  uint32_t capabilities; // b1=1 for non-VGA board
  uint16_t mode_list_off;
  uint16_t mode_list_seg;
  uint16_t vid_mem_size; // in units of 64K
  uint8_t  reserved[492];
} __attribute__ ((__packed__));

//---------------------------------------------------------------------------
struct SVBEMode
{
  // All VBE versions
  uint16_t modeAttributes;
  uint8_t  winAAttributes;
  uint8_t  winBAttributes;
  uint16_t winGranularity;
  uint16_t winSize;
  uint16_t winASegment;
  uint16_t winBSegment;
  uint32_t winFuncPtr;
  uint16_t bytesPerScanLine;
  // VBE 1.2 and above
  uint16_t xResolution;
  uint16_t yResolution;
  uint8_t  xCharSize;
  uint8_t  yCharSize;
  uint8_t  numberOfPlanes;
  uint8_t  bitsPerPixel;
  uint8_t  numberOfBanks;
  uint8_t  memoryModel;
  uint8_t  bankSize;
  uint8_t  numberOfImagePlanes;
  uint8_t  reserved1[1];
  // Direct Color fields
  uint8_t  reserved2[9];
  // VBE 2.0 and above
  uint32_t physBasePtr;
  uint8_t  reserved3[6];
  // VBE 3.0 and above
  uint8_t  reserved4[16+189];
} __attribute__ ((__packed__));

//---------------------------------------------------------------------------
class CVesaVideoDevice
 : public CAVideoDevice
{
public:
  CVesaVideoDevice();
  virtual ~CVesaVideoDevice();

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

private:
  bool vbeCall(uint8_t function);

  // Surface we're currently displaying
  CSurface * pSurface_;
  vuint32_t iFrameCount_; // volatile, becouse the isr updates it
  const SVideoMode * pCurrentMode_;
  const SVideoMode * pDefaultMode_;
  CV86Thread v86thr_;
  SVBEInfo * pInfo_;
  int16_t  * pModeNrs_;
  SVideoMode * pMode_;
  uint32_t iModeCount_;
  SVBEMode * pCurrentVBEMode_;
};


#endif
