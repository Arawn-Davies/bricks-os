#ifndef PS2_GIF_H
#define PS2_GIF_H


#include "inttypes.h"
#include "dma.h"


namespace GIF
{
  namespace DataFormat
  {
    enum EGIFDataFormat {packed, reglist, image, disable};
  };

  namespace REG
  {
    enum EGIFREG  {prim, rgbaq, st, uv, xyzf2, xyz2, tex0_1, tex0_2, clamp_1, clamp_2, fog, reserved, xyzf3, xyz3, ad, nop};
    enum EGIFREG2 {
      tex1_1     = 0x14,
      tex1_2     = 0x15,
      tex2_1     = 0x16,
      tex2_2     = 0x17,
      xyoffset_1 = 0x18, // Mapping from Primitive to Window coordinate system (Context 1)
      xyoffset_2 = 0x19, // Mapping from Primitive to Window coordinate system (Context 2)
      prmodecont = 0x1a,
      prmode     = 0x1b,
      texclut    = 0x1c,
      scanmsk    = 0x22,
      miptbp1_1  = 0x34,
      miptbp1_2  = 0x35,
      miptbp2_1  = 0x36,
      miptbp2_2  = 0x37,
      texa       = 0x3b,
      fogcol     = 0x3d,
      texflush   = 0x3f, // Write to this register before using newly loaded texture
      scissor_1  = 0x40, // Setup clipping rectangle (Context 1)
      scissor_2  = 0x41, // Setup clipping rectangle (Context 2)
      alpha_1    = 0x42, // Setup Alpha Blending Parameters (Context 1)
      alpha_2    = 0x43, // Setup Alpha Blending Parameters (Context 2)
      dimx       = 0x44,
      dthe       = 0x45,
      colclamp   = 0x46,
      test_1     = 0x47,
      test_2     = 0x48,
      pabe       = 0x49,
      fba_1      = 0x4a,
      fba_2      = 0x4b,
      frame_1    = 0x4c, // Frame buffer settings (Context 1)
      frame_2    = 0x4d, // Frame buffer settings (Context 2)
      zbuf_1     = 0x4e,
      zbuf_2     = 0x4f,
      bitbltbuf  = 0x50, // Setup Image Transfer Between EE and GS
      trxpos     = 0x51, // Setup Image Transfer Coordinates
      trxreg     = 0x52, // Setup Image Transfer Size
      trxdir     = 0x53, // Set Image Transfer Directon + Start Transfer
      hwreg      = 0x54,
      signal     = 0x60,
      finish     = 0x61,
      label      = 0x62
    };
  };
};


//-------------------------------------------------------------------------
#define DMA_GIF_TAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) \
  ((uint64_t)(NLOOP) <<  0) | \
  ((uint64_t)(EOP)   << 15) | \
  ((uint64_t)(PRE)   << 46) | \
  ((uint64_t)(PRIM)  << 47) | \
  ((uint64_t)(FLG)   << 58) | \
  ((uint64_t)(NREG)  << 60)


//-------------------------------------------------------------------------
class CGIFPacket
 : public CSCDMAPacket
{
public:
  CGIFPacket(uint32_t qwSize, EDMAChannel channelId);
  CGIFPacket(void * buffer, uint32_t qwSize, EDMAChannel channelId);
  ~CGIFPacket();

  // Reset packet
  inline void reset();

  // Open GIF Tag
  void gifTagOpenPacked(
    uint8_t regCount = 1,
    uint8_t reg1  = GIF::REG::ad,
    uint8_t reg2  = GIF::REG::nop,
    uint8_t reg3  = GIF::REG::nop,
    uint8_t reg4  = GIF::REG::nop,
    uint8_t reg5  = GIF::REG::nop,
    uint8_t reg6  = GIF::REG::nop,
    uint8_t reg7  = GIF::REG::nop,
    uint8_t reg8  = GIF::REG::nop,
    uint8_t reg9  = GIF::REG::nop,
    uint8_t reg10 = GIF::REG::nop,
    uint8_t reg11 = GIF::REG::nop,
    uint8_t reg12 = GIF::REG::nop,
    uint8_t reg13 = GIF::REG::nop,
    uint8_t reg14 = GIF::REG::nop,
    uint8_t reg15 = GIF::REG::nop,
    uint8_t reg16 = GIF::REG::nop);
  void gifTagOpenReglist(
    uint8_t regCount,
    uint8_t reg1,
    uint8_t reg2  = GIF::REG::nop,
    uint8_t reg3  = GIF::REG::nop,
    uint8_t reg4  = GIF::REG::nop,
    uint8_t reg5  = GIF::REG::nop,
    uint8_t reg6  = GIF::REG::nop,
    uint8_t reg7  = GIF::REG::nop,
    uint8_t reg8  = GIF::REG::nop,
    uint8_t reg9  = GIF::REG::nop,
    uint8_t reg10 = GIF::REG::nop,
    uint8_t reg11 = GIF::REG::nop,
    uint8_t reg12 = GIF::REG::nop,
    uint8_t reg13 = GIF::REG::nop,
    uint8_t reg14 = GIF::REG::nop,
    uint8_t reg15 = GIF::REG::nop,
    uint8_t reg16 = GIF::REG::nop);

  // Close GIF Tag
  void gifTagClose();

  inline void gifAddPackedAD(uint64_t reg, uint64_t data);
  void gifAddSendImage(const void * source, uint32_t size);

  // Pad to 128bit (for faster operation)
  inline void pad128();

protected:
  void * pGIFTag_;
  uint32_t nloopDiv_;
};


#include "gif.inl"


#endif // PS2_GIF_H
