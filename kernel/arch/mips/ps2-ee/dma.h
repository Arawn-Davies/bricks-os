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


#ifndef PS2_DMA_H
#define PS2_DMA_H


#include "stdint.h"
#include "asm/irq.h"
#include "asm/arch/config.h"
#ifdef CONFIG_KERNEL_MODE
#include "r5900.h"
#else
#include "bios.h"
#endif


//---------------------------------------------------------------------------
// DMA Register Constructors
//---------------------------------------------------------------------------
// CHCR Register - Channel Control Register
#define DMA_CHCR(DIR,MOD,ASP,TTE,TIE,STR,TAG) \
  ((uint32_t)(DIR) << 0) | \
  ((uint32_t)(MOD) << 2) | \
  ((uint32_t)(ASP) << 4) | \
  ((uint32_t)(TTE) << 6) | \
  ((uint32_t)(TIE) << 7) | \
  ((uint32_t)(STR) << 8) | \
  ((uint32_t)(TAG) << 16)

// MADR Register - Transfer Address Register
#define DMA_MADR(ADDR,SPR) \
  (((uint32_t)(ADDR) << 0) | \
   ((uint32_t)(SPR)  << 31))

// TADR Register - Tag Address Register
#define DMA_TADR(ADDR,SPR) \
  (((uint32_t)(ADDR) << 0) | \
   ((uint32_t)(SPR)  << 31))

// QWC Register - Transfer Data Size Register
#define DMA_QWC(SIZE) ((uint32_t)(SIZE))

#define DMA_CHANNEL_COUNT   10
#define DMA_MAX_QWTRANSFER  (16 * 1024)                  // Max amount of qwords (16 bytes / 128 bits) to transfer  (16K)
#define DMA_MAX_TRANSFER    (DMA_MAX_QWTRANSFER * 16)    // Max amount of bytes to transfer                        (256KiB)


namespace DMAC
{
  namespace Channel
  {
    enum Id
    {
      VIF0    = 0,
      VIF1    = 1,
      GIF     = 2,
      fromIPU = 3,
      toIPU   = 4,
      SIF0    = 5,
      SIF1    = 6,
      SIF2    = 7,
      fromSPR = 8,
      toSPR   = 9
    };
    enum Direction
    {
      toMemory    = 0,
      fromMemory  = 1,
    };
    enum Mode
    {
      normal      = 0,
      chain       = 1,
      interactive = 2
    };
  };

  namespace Tag
  {
    enum Id
    {
      refe = 0,
      cnt  = 1,
      next = 2,
      ref  = 3,
      refs = 4,
      call = 5,
      ret  = 6,
      end  = 7
    };
  };
};
typedef DMAC::Channel::Id EDMAChannel;

typedef struct
{
  uint64_t QWC  :16; // Quad Word Count
  uint64_t pad  :10; // Not used
  uint64_t PCE  : 2; // Priority Control Enable
  uint64_t ID   : 3; // ID: tag type
  uint64_t IRQ  : 1; // Interrupt on end of packet
  uint64_t ADDR :31; // Address
  uint64_t SPR  : 1; // Scratchpad/Main Memory
  uint32_t opt1;
  uint32_t opt2;
} SDMATag __attribute__ ((aligned(16)));


//---------------------------------------------------------------------------
class IDMAChannelHandler
{
public:
  virtual ~IDMAChannelHandler(){}

  virtual int dmaCallbackHandler(int channel) = 0;
};

// -----------------------------------------------------------------------------
// DMA Controller class
class CDMAC
#ifdef CONFIG_KERNEL_MODE
 : public IMIPSInterruptHandler
#endif
{
#ifndef CONFIG_KERNEL_MODE
  friend int32_t biosDMAHandler2();
  friend int32_t biosDMAHandler(int32_t channel);
#endif
public:
  CDMAC();
  virtual ~CDMAC();

  int init();

  void attach(unsigned int channel, IDMAChannelHandler * handler);
  void detach(unsigned int channel, IDMAChannelHandler * handler);

  void enable(unsigned int channel);
  void disable(unsigned int channel);

#ifdef CONFIG_KERNEL_MODE
  void isr(unsigned int irq, pt_regs * regs);
#endif

private:
#ifdef CONFIG_KERNEL_MODE
  uint32_t iDMAMask_;
#else
  int32_t handle_[DMA_CHANNEL_COUNT];
  static int     ps2ThreadId_;
  static uint8_t ps2ThreadStack_[16*1024];
#endif

  static IDMAChannelHandler * handlers_[DMA_CHANNEL_COUNT];
};

// -----------------------------------------------------------------------------
class CDMAPacket
{
public:
  CDMAPacket(uint32_t qwSize, EDMAChannel channelId);
  CDMAPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId);
  virtual ~CDMAPacket();

  // Reset packet
  inline void reset();

  // Add data to packet
  template<class T> inline void add(const T data);
  inline void add8  (const uint8_t   data){this->add(data);}
  inline void add16 (const uint16_t  data){this->add(data);}
  inline void add32 (const uint32_t  data){this->add(data);}
  inline void add64 (const uint64_t  data){this->add(data);}
//  inline void add128(const uint128_t data){this->add(data)}
  inline void addFloat (float f){this->add(f);}
  inline void addFloat4(float f1, float f2, float f3, float f4){this->add(f1);this->add(f2);this->add(f3);this->add(f4);}

  // Send data to dma channel
  virtual void send(bool waitComplete = true);

  // Pad to 128bit (for faster operation)
  inline void pad128(uint32_t data);

protected:
  uint8_t   * pRawData_;      // Not aligned data
  uint8_t   * pData_;         // Aligned data
  uint8_t   * pCurrent_;      // Pointer to next free position
  uint8_t   * pEnd_;          // Pointer to end of data
  uint32_t    iQWSize_;       // Size of data in QWords
  bool        bHasOwnership_; // Delete buffer in destructor if we own the buffer
  EDMAChannel eChannelId_;    // DMA channel ID
};

// -----------------------------------------------------------------------------
// Source Chain DMA packets
class CSCDMAPacket
 : public CDMAPacket
{
public:
  CSCDMAPacket(uint32_t qwSize, EDMAChannel channelId);
  CSCDMAPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId);
  virtual ~CSCDMAPacket();

  // Reset packet
  inline void reset();

  // Send data to dma channel
  virtual void send(bool waitComplete = true);

  // DMA Tags in Source Chain mode
  inline CSCDMAPacket & scTagOpenCnt(                                 bool irq = false);
//  inline CSCDMAPacket & scTagNext(const SDMATag * nextTag);
  inline CSCDMAPacket & scTagRef(const void * data, uint32_t qwSize,  bool irq = false);
//  inline CSCDMAPacket & scTagRefs(const void * data, uint32_t qwSize);
  inline CSCDMAPacket & scTagRefe(const void * data, uint32_t qwSize, bool irq = false);
//  inline CSCDMAPacket & scTagCall(const SDMATag * nextTag);
//  inline CSCDMAPacket & scTagCall(const CSCDMAPacket & packet);
//  inline CSCDMAPacket & scTagRet();
  inline CSCDMAPacket & scTagOpenEnd(                                 bool irq = false);

  // Close DMA Tag in Source Chain mode (sets data size in tag)
  inline CSCDMAPacket & scTagClose();

protected:
  SDMATag * pSCTag_;
};


#include "dma.inl"


#endif // PS2_DMA_H
