#include "dma.h"
#include "bios.h"
#include "stddef.h"


struct SDMAChannel
{
  vuint32_t * qwc;
  vuint32_t * madr;
  vuint32_t * tadr;
  vuint32_t * chcr;
  int32_t     handler;
  bool        initialized;
};

static SDMAChannel dmaChannel_[10] =
{
  {(vuint32_t *)0x10008020, (vuint32_t *)0x10008010, (vuint32_t *)0x10008030, (vuint32_t *)0x10008000, 0, false},
  {(vuint32_t *)0x10009020, (vuint32_t *)0x10009010, (vuint32_t *)0x10009030, (vuint32_t *)0x10009000, 0, false},
  {(vuint32_t *)0x1000a020, (vuint32_t *)0x1000a010, (vuint32_t *)0x1000a030, (vuint32_t *)0x1000a000, 0, false},
  {(vuint32_t *)0x1000b020, (vuint32_t *)0x1000b010, (vuint32_t *)0x1000b030, (vuint32_t *)0x1000b000, 0, false},
  {(vuint32_t *)0x1000b420, (vuint32_t *)0x1000b410, (vuint32_t *)0x1000b430, (vuint32_t *)0x1000b400, 0, false},
  {(vuint32_t *)0x1000c020, (vuint32_t *)0x1000c010, (vuint32_t *)0x1000c030, (vuint32_t *)0x1000c000, 0, false},
  {(vuint32_t *)0x1000c420, (vuint32_t *)0x1000c410, (vuint32_t *)0x1000c430, (vuint32_t *)0x1000c400, 0, false},
  {(vuint32_t *)0x1000c820, (vuint32_t *)0x1000c810, (vuint32_t *)0x1000c830, (vuint32_t *)0x1000c800, 0, false},
  {(vuint32_t *)0x1000d020, (vuint32_t *)0x1000d010, (vuint32_t *)0x1000d030, (vuint32_t *)0x1000d000, 0, false},
  {(vuint32_t *)0x1000d420, (vuint32_t *)0x1000d410, (vuint32_t *)0x1000d430, (vuint32_t *)0x1000d400, 0, false}
};


// -----------------------------------------------------------------------------
void
dmaInitialize()
{
  // Reset the dma controller.
  bios::ResetEE(0x01);
}

// -----------------------------------------------------------------------------
void
dmaInitialize(EDMAChannel channel, dmaCallBack handler)
{
  if(dmaChannel_[channel].initialized == false)
  {
    // Set callback handler
    if(handler != NULL)
      dmaChannel_[channel].handler = bios::AddDmacHandler(channel, handler, 0);

    // Enable DMA channel
    bios::_EnableDmac(channel);

    dmaChannel_[channel].initialized = true;
  }
}

// -----------------------------------------------------------------------------
void
dmaShutdown(EDMAChannel channel)
{
  if(dmaChannel_[channel].initialized == true)
  {
    // Disable DMA channel
    bios::_DisableDmac(channel);

    // Remove callback handler.
    if(dmaChannel_[channel].handler != 0)
    {
      bios::RemoveDmacHandler(channel, dmaChannel_[channel].handler);
      dmaChannel_[channel].handler = 0;
    }

    dmaChannel_[channel].initialized = false;
  }
}

// -----------------------------------------------------------------------------
void
dmaSend(EDMAChannel channel, void * data, int size, bool chain)
{
  bios::_SyncDCache(data, (uint8_t *)data + size);

  if(chain == false)
  {
    *dmaChannel_[channel].qwc  = DMA_QWC(size);
    *dmaChannel_[channel].madr = DMA_MADR(data, 0);
    *dmaChannel_[channel].tadr = DMA_TADR(0, 0);
    *dmaChannel_[channel].chcr = DMA_CHCR(DMAC::Channel::fromMemory, DMAC::Channel::normal, 0, 0, 0, 1, 0);
  }
  else
  {
    *dmaChannel_[channel].qwc  = DMA_QWC(0);
    *dmaChannel_[channel].madr = DMA_MADR(0, 0);
    *dmaChannel_[channel].tadr = DMA_TADR(data, 0);
    *dmaChannel_[channel].chcr = DMA_CHCR(DMAC::Channel::fromMemory, DMAC::Channel::chain,  0, 0, 0, 1, 0);
  }
}

// -----------------------------------------------------------------------------
void
dmaWait(EDMAChannel channel)
{
  while(*dmaChannel_[channel].chcr & (1<<8));
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
inline void
CDMAPacket::reset()
{
  pCurrent_ = pData_;
}

//-------------------------------------------------------------------------
#define ADD_DATA(__TYPE, __DATA) *((__TYPE *)pCurrent_) = __DATA; pCurrent_ += sizeof(__TYPE);

//-------------------------------------------------------------------------
template<class T> inline void
CDMAPacket::add(const T data)
{
  ADD_DATA(T, data);
}
