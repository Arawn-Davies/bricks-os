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


#include "debugScreen.h"
#include "bios.h"
#include "dma.h"
#include "gif.h"
#include "string.h"


extern uint8_t getBitNr(uint32_t value);
extern SPS2VideoMode vmodes[];
extern uint32_t courier_new[];
extern uint16_t fixed_tc[];

uint64_t pdata[1000 * 2] __attribute__((aligned(64)));


//---------------------------------------------------------------------------
CPS2DebugScreen::CPS2DebugScreen()
 : pCurrentPS2Mode_(NULL)
 , freeMemAddr_(0)
 , iCurrentX_(0)
 , iCurrentY_(0)
 , g2_fontbuf_addr(0)
 , g2_fontbuf_w(256)
 , g2_fontbuf_h(128)
 , g2_font_tc(0)
 , packet_(pdata, 1000, DMAC::Channel::GIF)
{
  frameAddr_[0] = 0;
  frameAddr_[1] = 0;

  for(int y(0); y < TEXT_HEIGHT; y++)
    pBuffer_[y][0] = 0;
}

//---------------------------------------------------------------------------
CPS2DebugScreen::~CPS2DebugScreen()
{
  REG_GS_CSR = GS_CSR_RESET();
}

// -----------------------------------------------------------------------------
int
CPS2DebugScreen::init()
{
  // Select video mode
  //  0 -> NTSC, 640x224, 60Hz double-scan = 30Hz progressive
  //  2 -> PAL,  640x256, 50Hz double-scan = 25Hz progressive
  pCurrentPS2Mode_ = IS_PAL() ? &vmodes[1] : &vmodes[0];
  actualHeight_ = pCurrentPS2Mode_->height / 2;

  // Set video mode
  this->setMode(pCurrentPS2Mode_);

  // Clear screen
  cls();

  // Set font
  g2_font_tc = fixed_tc;
  ee_to_gsBitBlt(g2_fontbuf_addr, 0, 0, 0, g2_fontbuf_w, g2_fontbuf_h, (uint32_t)courier_new);

  return 0;
}

//---------------------------------------------------------------------------
ssize_t
CPS2DebugScreen::write(const void * data, size_t size, bool block)
{
  while((*((const char *)data) != 0) && (size > 0))
  {
    switch(*((const char *)data))
    {
      case '\n':
      {
        // Advance to next line
        iCurrentY_++;
        iCurrentX_ = 0;
        if(iCurrentY_ >= TEXT_HEIGHT)
        {
          // Scroll buffer up 1 line
          iCurrentY_ = TEXT_HEIGHT - 1;
          for(int y(0); y < TEXT_HEIGHT-1; y++)
            strcpy(pBuffer_[y], pBuffer_[y+1]);
          pBuffer_[iCurrentY_][0] = 0;
        }
        break;
      }
      default:
      {
        if(iCurrentX_ < (TEXT_WIDTH-1))
        {
          pBuffer_[iCurrentY_][iCurrentX_] = *((const char *)data);
          iCurrentX_++;
          pBuffer_[iCurrentY_][iCurrentX_] = 0;
        }
      }
    };
    ((const char *)data)++;
    size--;
  }

  // Clear screen
  cls();

  // Draw all lines
  printBegin();
  for(int y(0); y < TEXT_HEIGHT; y++)
    printLine(15, 15 * y,  pBuffer_[y]);
  printEnd();

  return 0;
}

//---------------------------------------------------------------------------
void
CPS2DebugScreen::cls()
{
  packet_.scTagOpenEnd();
    packet_.gifTagOpenPacked();
      packet_.gifAddPackedAD(GIF::REG::prim,  GIF::REG::PRIM(GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0));
      packet_.gifAddPackedAD(GIF::REG::rgbaq, GIF::REG::RGBAQ(0, 0, 0, 0x80, 0));
      packet_.gifAddPackedAD(GIF::REG::xyz2,  GIF::REG::XYZ2(GS_X_BASE<<4, GS_Y_BASE<<4, 0));
      packet_.gifAddPackedAD(GIF::REG::xyz2,  GIF::REG::XYZ2((GS_X_BASE+pCurrentPS2Mode_->width+1)<<4, (GS_Y_BASE+actualHeight_+1)<<4, 0));
    packet_.gifTagClose();
  packet_.scTagClose();
  packet_.send();
  packet_.reset();
}

//---------------------------------------------------------------------------
void
CPS2DebugScreen::setMode(SPS2VideoMode * mode)
{
  pCurrentPS2Mode_ = mode;

  // Allocate first frame
  frameAddr_[0] = freeMemAddr_;
  freeMemAddr_ += pCurrentPS2Mode_->width * actualHeight_ * 4;

  // No double buffering (second frame points to first one)
  frameAddr_[1] = frameAddr_[0];

  // Allocate (font) texture
  g2_fontbuf_addr = freeMemAddr_;
  freeMemAddr_ += g2_fontbuf_w * g2_fontbuf_h * 4;

  // Reset the GS
  REG_GS_CSR = GS_CSR_RESET();

  // Sync
  __asm__("sync.p\nnop");

  // Setup CRTC for video mode
  bios::SetGsCrt(GS_NON_INTERLACED, pCurrentPS2Mode_->crtcMode->biosMode, 0);

  // Enable read circuit 1
  REG_GS_PMODE = GS_PMODE(
    1,        // ReadCircuit1 ON
    0,        // ReadCircuit2 OFF
    1,        // Use ALP register for Alpha Blending
    1,        // Alpha Value of ReadCircuit2 for output selection
    0,        // Blend Alpha with the output of ReadCircuit2
    0xff      // Alpha Value = 1.0
  );

  // Setup read circuit 1
  REG_GS_DISPLAY1 = GS_DISPLAY_CREATE(pCurrentPS2Mode_->crtcMode->vck,
                                      pCurrentPS2Mode_->xoffset,
                                      pCurrentPS2Mode_->yoffset >> 1,
                                      1 * pCurrentPS2Mode_->crtcMode->vck, // MAGH
                                      1,                                   // MAGV
                                      pCurrentPS2Mode_->width,
                                      actualHeight_);

  // Setup read circuit 2
  //REG_GS_DISPLAY2 = ...;

  packet_.scTagOpenEnd();
    packet_.gifTagOpenPacked();
      // Use drawing parameters from PRIM register
      packet_.gifAddPackedAD(GIF::REG::prmodecont, 1);
      // Setup frame buffers. Point to 0 initially.
      packet_.gifAddPackedAD(GIF::REG::frame_1,    GIF::REG::FRAME(frameAddr_[0] >> 13, pCurrentPS2Mode_->width >> 6, GS_PSM_32, 0));
      // Displacement between Primitive and Window coordinate systems.
      packet_.gifAddPackedAD(GIF::REG::xyoffset_1, GIF::REG::XYOFFSET(GS_X_BASE<<4, GS_Y_BASE<<4));
      // Clip to frame buffer.
      packet_.gifAddPackedAD(GIF::REG::scissor_1,  GIF::REG::SCISSOR(0, pCurrentPS2Mode_->width, 0, actualHeight_));
    packet_.gifTagClose();
  packet_.scTagClose();
  packet_.send();
  packet_.reset();

  // Display buffer
  REG_GS_DISPFB1  = GS_DISPFB(frameAddr_[0] >> 13, pCurrentPS2Mode_->width >> 6, GS_PSM_32, 0, 0);
}

//---------------------------------------------------------------------------
void
CPS2DebugScreen::printBegin()
{
  packet_.scTagOpenEnd();
    packet_.gifTagOpenPacked();

      // Draw a sprite with current character mapped onto it
      packet_.gifAddPackedAD(GIF::REG::tex0_1,
        GIF::REG::TEX0(
          g2_fontbuf_addr/256,            // base pointer
          (g2_fontbuf_w)/64,              // width
          0,                              // 32bit RGBA
          getBitNr(g2_fontbuf_w),         // width
          getBitNr(g2_fontbuf_w),         // height
          1,                              // RGBA
          GS_TEX0_DECAL,                  // just overwrite existing pixels
          0,0,0,0,0));
/*
      packet_.gifAddPackedAD(tex1_1,
        GIF::REG::TEX1(
          0, 0,
          GS_TEX1_LINEAR,
          GS_TEX1_LINEAR,
          0, 0, 0));

      packet_.gifAddPackedAD(GIF::REG::clamp_1, 0x05);
*/
      packet_.gifAddPackedAD(GIF::REG::prim,
        GIF::REG::PRIM(GS_PRIM_SPRITE,
          0,                              // flat shading
          1,                              // texture mapping ON
          0, 1, 0,                        // no fog or antialiasing, but use alpha
          1,                              // use UV register for coordinates.
          0,
          0));
}

//---------------------------------------------------------------------------
void
CPS2DebugScreen::printLine(uint16_t x, uint16_t y, char * str)
{
  uint16_t * tc;            // current texture coordinates [4]
  uint16_t x0, y0, x1, y1;  // rectangle for current character
  uint16_t w, h;            // width and height of above rectangle

  x += GS_X_BASE;
  y += GS_Y_BASE;

  while(*str)
  {
    // Read the texture coordinates for current character
    tc = &g2_font_tc[(*str)*4];
    x0 = *tc++;
    y0 = *tc++;
    x1 = *tc++;
    y1 = *tc++;
    w  = x1-x0+1;
    h  = y1-y0+1;

    packet_.gifAddPackedAD(GIF::REG::uv,    GIF::REG::UV(x0<<4, y0<<4));
    packet_.gifAddPackedAD(GIF::REG::xyz2,  GIF::REG::XYZ2(x<<4, y<<4, 0));
    packet_.gifAddPackedAD(GIF::REG::uv,    GIF::REG::UV((x1+1)<<4, (y1+1)<<4));
    packet_.gifAddPackedAD(GIF::REG::xyz2,  GIF::REG::XYZ2((x+w)<<4, (y+h)<<4, 0));

    // Advance drawing position
    x += w;

    // Get next character
    str++;
  }
}

//---------------------------------------------------------------------------
void
CPS2DebugScreen::printEnd()
{
    packet_.gifTagClose();
  packet_.scTagClose();
  packet_.send();
  packet_.reset();
}
