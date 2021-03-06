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


#include "dma.h"
#include "kernel/debug.h"
#include "registers.h"


#ifndef CONFIG_KERNEL_MODE
extern int _gp;
uint8_t CDMAC::ps2ThreadStack_[16*1024] __attribute__((aligned(64)));
int     CDMAC::ps2ThreadId_;
#endif


const char * sDMASource[] =
{
  "VIF0",
  "VIF1",
  "GIF",
  "fromIPU",
  "toIPU",
  "SIF0 (IOP to EE)",
  "SIF1 (EE to IOP)",
  "SIF2",
  "fromSPR",
  "toSPR"
};

struct SDMAChannel
{
  vuint32_t * chcr;
  vuint32_t * madr;
  vuint32_t * qwc;
  vuint32_t * tadr;
};

static SDMAChannel dmaChannel_[10] =
{
  {&REG_DMA_VIF0_CHCR,    &REG_DMA_VIF0_MADR,    &REG_DMA_VIF0_QWC,    &REG_DMA_VIF0_TADR},
  {&REG_DMA_VIF1_CHCR,    &REG_DMA_VIF1_MADR,    &REG_DMA_VIF1_QWC,    &REG_DMA_VIF1_TADR},
  {&REG_DMA_GIF_CHCR,     &REG_DMA_GIF_MADR,     &REG_DMA_GIF_QWC,     &REG_DMA_GIF_TADR},
  {&REG_DMA_fromIPU_CHCR, &REG_DMA_fromIPU_MADR, &REG_DMA_fromIPU_QWC, &REG_DMA_fromIPU_TADR},
  {&REG_DMA_toIPU_CHCR,   &REG_DMA_toIPU_MADR,   &REG_DMA_toIPU_QWC,   &REG_DMA_toIPU_TADR},
  {&REG_DMA_SIF0_CHCR,    &REG_DMA_SIF0_MADR,    &REG_DMA_SIF0_QWC,    &REG_DMA_SIF0_TADR},
  {&REG_DMA_SIF1_CHCR,    &REG_DMA_SIF1_MADR,    &REG_DMA_SIF1_QWC,    &REG_DMA_SIF1_TADR},
  {&REG_DMA_SIF2_CHCR,    &REG_DMA_SIF2_MADR,    &REG_DMA_SIF2_QWC,    &REG_DMA_SIF2_TADR},
  {&REG_DMA_fromSPR_CHCR, &REG_DMA_fromSPR_MADR, &REG_DMA_fromSPR_QWC, &REG_DMA_fromSPR_TADR},
  {&REG_DMA_toSPR_CHCR,   &REG_DMA_toSPR_MADR,   &REG_DMA_toSPR_QWC,   &REG_DMA_toSPR_TADR},
};


IDMAChannelHandler * CDMAC::handlers_[DMA_CHANNEL_COUNT];


#ifndef CONFIG_KERNEL_MODE
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Thread used to handle interrupts
static int32_t iChannelNr = -1;
int32_t
biosDMAHandler2()
{
  bool bDone = false;

  while(bDone == false)
  {
    // Wait for interrupt
    bios::SleepThread();

    if((iChannelNr != -1) && (iChannelNr < DMA_CHANNEL_COUNT))
    {
      // Handle
      if(CDMAC::handlers_[iChannelNr] != NULL)
        CDMAC::handlers_[iChannelNr]->dmaCallbackHandler(iChannelNr);

      iChannelNr = -1;
    }
  }

  bios::ExitDeleteThread();

  return 0;
}

// -----------------------------------------------------------------------------
int32_t
biosDMAHandler(int channel)
{
  if(channel < DMA_CHANNEL_COUNT)
  {
    // Wake up handler thread
    iChannelNr = channel;
    bios::iWakeupThread(CDMAC::ps2ThreadId_);
  }

  asm __volatile__("sync");
  asm __volatile__("ei");

  return 0;
}
#endif // CONFIG_KERNEL_MODE

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
CDMAC::CDMAC()
{
}

//-------------------------------------------------------------------------
CDMAC::~CDMAC()
{
}

//-------------------------------------------------------------------------
int
CDMAC::init()
{
#ifdef CONFIG_KERNEL_MODE
  iDMAMask_ = 0;

  // Clear DMA mask/status
  //  0.. 9 == status (writing 1 clears status)
  // 16..25 == mask   (writing 1 xors mask)
  REG_DMA_STAT = REG_DMA_STAT;

  // DMA controller is connected to MIPS interrupt PIN1
  setInterruptHandler(MIPS_INTERRUPT_1, *this);
#else
  for(int i(0); i < DMA_CHANNEL_COUNT; i++)
    handle_[i] = 0;

  // Create handler thread
  ee_thread_t ps2Thread;
  ps2Thread.func = (void *)biosDMAHandler2;
  ps2Thread.stack = ps2ThreadStack_;
  ps2Thread.stack_size = sizeof(ps2ThreadStack_);
  ps2Thread.gp_reg = &_gp;
  ps2Thread.initial_priority = 20;

  int ret = bios::CreateThread(&ps2Thread);
  if(ret < 0)
    return -1;

  ps2ThreadId_ = ret;

  ret = bios::StartThread(ps2ThreadId_, 0);
  if(ret < 0)
  {
    bios::DeleteThread(ps2ThreadId_);
    ps2ThreadId_ = 0;
    return -1;
  }
#endif // CONFIG_KERNEL_MODE

  for(int i(0); i < DMA_CHANNEL_COUNT; i++)
    handlers_[i] = NULL;

  return 0;
}

//-------------------------------------------------------------------------
void
CDMAC::attach(unsigned int channel, IDMAChannelHandler * handler)
{
  if(channel < DMA_CHANNEL_COUNT)
  {
    handlers_[channel] = handler;
    enable(channel);
  }
}

//-------------------------------------------------------------------------
void
CDMAC::detach(unsigned int channel, IDMAChannelHandler * handler)
{
  if(channel < DMA_CHANNEL_COUNT)
  {
    if(handlers_[channel] == handler)
    {
      handlers_[channel] = NULL;
      disable(channel);
    }
  }
}

//-------------------------------------------------------------------------
void
CDMAC::enable(unsigned int channel)
{
  if(channel < DMA_CHANNEL_COUNT)
  {
#ifdef CONFIG_KERNEL_MODE
    // Enable if disabled
    if((iDMAMask_ & (0x00010000 << channel)) == 0)
    {
      iDMAMask_    |= (0x00010000 << channel);
      REG_DMA_STAT  = (0x00010000 << channel);
    }
#else
    if(handle_[channel] == 0)
    {
      handle_[channel] = bios::AddDmacHandler(channel, ::biosDMAHandler, 0);
      bios::EnableDmac(channel);
    }
#endif // CONFIG_KERNEL_MODE
  }
}

//-------------------------------------------------------------------------
void
CDMAC::disable(unsigned int channel)
{
  if(channel < DMA_CHANNEL_COUNT)
  {
#ifdef CONFIG_KERNEL_MODE
    // Disable if enabled
    if((iDMAMask_ & (0x00010000 << channel)) != 0)
    {
      iDMAMask_    &= ~(0x00010000 << channel);
      REG_DMA_STAT  =  (0x00010000 << channel);
    }
#else
    if(handle_[channel] != 0)
    {
      bios::DisableDmac(channel);
      bios::RemoveDmacHandler(channel, handle_[channel]);
      handle_[channel] = 0;
    }
#endif // CONFIG_KERNEL_MODE
  }
}

#ifdef CONFIG_KERNEL_MODE
//-------------------------------------------------------------------------
void
CDMAC::isr(unsigned int irq, pt_regs * regs)
{
  uint32_t status = REG_DMA_STAT;

  // DMA controller
  //printk("interrupt from DMAC(0x%x)\n", status);

  for(uint32_t i(0); i < DMA_CHANNEL_COUNT; i++)
  {
    if((status & ((uint32_t)0x00010001 << i)) == ((uint32_t)0x00010001 << i))
    {
      //printk(" - %s\n", sDMASource[i]);

      // Clear status
      REG_DMA_STAT = (1 << i);

      if(handlers_[i] != NULL)
        handlers_[i]->dmaCallbackHandler(i);
    }
  }
}
#endif // CONFIG_KERNEL_MODE

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
CDMAPacket::CDMAPacket(uint32_t qwSize, EDMAChannel channelId)
 : bHasOwnership_(true)
 , eChannelId_(channelId)
{
  // Clip qwSize to DMA_MAX_QWTRANSFER
  if(qwSize > DMA_MAX_QWTRANSFER)
    qwSize = DMA_MAX_QWTRANSFER;
  iQWSize_ = qwSize;

  // Allocate data (+63 bytes for alignment)
  pRawData_ = new uint8_t[(qwSize * 16) + (64 - 1)];

  // Align to 64 byte boundry
  pData_    = (uint8_t *)((((uint32_t)pRawData_) + (64 - 1)) & (~(64 - 1)));
  pCurrent_ = pData_;
  pEnd_     = pData_ + (qwSize * 16);
}

//-------------------------------------------------------------------------
CDMAPacket::CDMAPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId)
 : pRawData_((uint8_t *)buffer)
 , bHasOwnership_(true)
 , eChannelId_(channelId)
{
  // Clip qwSize to DMA_MAX_QWTRANSFER
  if(qwSize > DMA_MAX_QWTRANSFER)
    qwSize = DMA_MAX_QWTRANSFER;
  iQWSize_ = qwSize;

  // Align to 64 byte boundry
  pData_    = (uint8_t *)((((uint32_t)pRawData_) + (64 - 1)) & (~(64 - 1)));
  pCurrent_ = pData_;
  pEnd_     = pData_ + (qwSize * 16);
}

//-------------------------------------------------------------------------
CDMAPacket::~CDMAPacket()
{
  if((bHasOwnership_ == true) && (pRawData_ != NULL))
    delete pRawData_;
}

//-------------------------------------------------------------------------
void
CDMAPacket::send(bool waitComplete)
{
  uint32_t qwc = ((uint32_t)pCurrent_ - (uint32_t)pData_) / 16;

  // Flush caches before transfer
  bios::FlushCache(0);
  bios::_SyncDCache(pData_, (uint128_t *)pData_ + qwc);

  // Send
  *dmaChannel_[eChannelId_].qwc  = DMA_QWC(qwc);
  *dmaChannel_[eChannelId_].madr = DMA_MADR((uint32_t)ADDR_TO_PHYS(pData_), 0);
  *dmaChannel_[eChannelId_].tadr = DMA_TADR(0, 0);
  *dmaChannel_[eChannelId_].chcr = DMA_CHCR(DMAC::Channel::fromMemory, DMAC::Channel::normal, 0, 0, 0, 1, 0);

  // Wait for completion
  if(waitComplete == true)
  {
    while(*dmaChannel_[eChannelId_].chcr & (1<<8));
  }
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
CSCDMAPacket::CSCDMAPacket(uint32_t qwSize, EDMAChannel channelId)
 : CDMAPacket(qwSize, channelId)
 , pSCTag_(NULL)
{
}

//-------------------------------------------------------------------------
CSCDMAPacket::CSCDMAPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId)
 : CDMAPacket(buffer, qwSize, channelId)
 , pSCTag_(NULL)
{
}

//-------------------------------------------------------------------------
CSCDMAPacket::~CSCDMAPacket()
{
}

//-------------------------------------------------------------------------
void
CSCDMAPacket::send(bool waitComplete)
{
  uint32_t qwc = ((uint32_t)pCurrent_ - (uint32_t)pData_) / 16;

  // Flush caches before transfer
  bios::FlushCache(0);
  bios::_SyncDCache(pData_, (uint128_t *)pData_ + qwc);

  // Send
  *dmaChannel_[eChannelId_].qwc  = DMA_QWC(0);
  *dmaChannel_[eChannelId_].madr = DMA_MADR(0, 0);
  *dmaChannel_[eChannelId_].tadr = DMA_TADR((uint32_t)ADDR_TO_PHYS(pData_), 0);
  *dmaChannel_[eChannelId_].chcr = DMA_CHCR(DMAC::Channel::fromMemory, DMAC::Channel::chain, 0, 0, 1, 1, 0);

  // Wait for completion
  if(waitComplete == true)
  {
    while(*dmaChannel_[eChannelId_].chcr & (1<<8));
  }
}
