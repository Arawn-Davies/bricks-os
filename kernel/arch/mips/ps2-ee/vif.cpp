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


#include "vif.h"


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
CVIFPacket::CVIFPacket(uint32_t qwSize, EDMAChannel channelId)
 : CSCDMAPacket(qwSize, channelId)
{
}

//-------------------------------------------------------------------------
CVIFPacket::CVIFPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId)
 : CSCDMAPacket(buffer, qwSize, channelId)
{
}

//-------------------------------------------------------------------------
CVIFPacket::~CVIFPacket()
{
}

//-------------------------------------------------------------------------
void
CVIFPacket::vifAddSendCode(const void * code, uint32_t size)
{
  int32_t size64 = size / 8;

  while(size64 > 0)
  {
    // Total send size
    uint32_t sendSize = (size64 > 256) ? 256 : size64;

    // Add send code command (VIF_CMD_MPG)
    this->scTagOpenCnt();
    {
      this->vifAddCommand(VIF_CMD_NOP,    0, 0);
      this->vifAddCommand(VIF_CMD_NOP,    0, 0);
      this->vifAddCommand(VIF_CMD_NOP,    0, 0);
      this->vifAddCommand(VIF_CMD_MPG,    (sendSize == 256) ? 0 : sendSize, 0);
    }
    this->scTagClose();

    // If not 128bit aligned, add 2 NOP commands after code
    if(sendSize & 1)
    {
      ((uint32_t *)code)[sendSize * 2 + 0] = VIF_CODE(0, 0, VIF_CMD_NOP);
      ((uint32_t *)code)[sendSize * 2 + 1] = VIF_CODE(0, 0, VIF_CMD_NOP);
    }

    // Add code to DMA packet
    this->scTagRef(code, (sendSize + 1) / 2);

    (uint64_t *)code += sendSize;
    size64 -= sendSize;
  }
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void
vu1_wait()
{
  //2 nops before coprocessor 2 wait?
  asm __volatile__
  (
    "  nop\n"
    "  nop\n"
    "0:\n"
    "  bc2t 0b\n"
    "  nop\n"
    "  nop\n"
  );
}

//-------------------------------------------------------------------------
void
vu_upload(void * dst, const void * src, unsigned int size)
{
  for(unsigned int i(0); i < (size + 8-1) / 8; i++)
  {
    *((uint64_t *)dst)++ = *((uint64_t *)src)++;
  }
}

//-------------------------------------------------------------------------
void
vu1_run()
{
  asm __volatile__
  (
    "ctc2.i $0,$31"
  );
}
