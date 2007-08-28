#include "kernel/debug.h"
#include "kernel/fatDriver.h"


//#define LE16(x)  (((x<<8)&0xff00)|((x>>8)&0x00ff))
//#define LE32(x)  (((x<<24)&0xff000000)|((x<<8)&0x00ff0000)|((x>>8)&0x0000ff00)|((x>>24)&0x000000ff))
#define LE16(x)  (x)
#define LE32(x)  (x)


struct SBPB
{
  uint8_t  BS_jmpBoot[3];
  char     BS_OEMName[8];
  uint16_t BPB_BytsPerSec;
  uint8_t  BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t  BPB_NumFATs;
  uint16_t BPB_RootEntCnt;
  uint16_t BPB_TotSec16;
  uint8_t  BPB_Media;
  uint16_t BPB_FATSz16;
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;
} __attribute__ ((__packed__));

struct SBPB16
{
  uint8_t  BS_DrvNum;
  uint8_t  BS_Reserved1;
  uint8_t  BS_BootSig;
  uint32_t BS_VolID;
  char     BS_VolLab[11];
  char     BS_FilSysType[8];
} __attribute__ ((__packed__));

struct SBPB32
{
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t  BPB_Reserved[12];
  uint8_t  BS_DrvNum;
  uint8_t  BS_Reserved1;
  uint8_t  BS_BootSig;
  uint32_t BS_VolID;
  char     BS_VolLab[11];
  char     BS_FilSysType[8];
} __attribute__ ((__packed__));

/*
// -----------------------------------------------------------------------------
CFAT::CFAT(IBlockDevice * pPartition)
 : pPartition_(pPartition)
{
}

// -----------------------------------------------------------------------------
CFAT::~CFAT()
{
}

// -----------------------------------------------------------------------------
int
CFAT::init()
{
  uint8_t data[512];

  if(pPartition_->read(0, 1, data) == true)
  {
    SBPB   * pBPB   = (SBPB   *)(&data[0]);
    SBPB16 * pBPB16 = (SBPB16 *)(&data[36]);
    SBPB32 * pBPB32 = (SBPB32 *)(&data[36]);

    // Validate boot signature
    if((data[0x1FE] == 0x55) && (data[0x1FF] == 0xAA))
    {
      // Validate "FAT" string
      if(((pBPB16->BS_FilSysType[0] == 'F') && (pBPB16->BS_FilSysType[1] == 'A') && (pBPB16->BS_FilSysType[2] == 'T')) ||
         ((pBPB32->BS_FilSysType[0] == 'F') && (pBPB32->BS_FilSysType[1] == 'A') && (pBPB32->BS_FilSysType[2] == 'T')))
      {
        uint32_t iClusterCount;
        uint32_t iRootDirSectors;
        uint32_t iDataSectors;
        uint32_t iFATSz;
        uint32_t iTotSec;

        iRootDirSectors = ((pBPB->BPB_RootEntCnt * 32) + (pBPB->BPB_BytsPerSec - 1)) / pBPB->BPB_BytsPerSec;
        if(pBPB->BPB_FATSz16 != 0)
          iFATSz = pBPB->BPB_FATSz16;
        else
          iFATSz = LE32(pBPB32->BPB_FATSz32);
        if(pBPB->BPB_TotSec16 != 0)
          iTotSec = pBPB->BPB_TotSec16;
        else
          iTotSec = pBPB->BPB_TotSec32;
        iDataSectors = iTotSec - (pBPB->BPB_RsvdSecCnt + (pBPB->BPB_NumFATs * iFATSz) + iRootDirSectors);
        iClusterCount = iDataSectors / pBPB->BPB_SecPerClus;

        if(iClusterCount < 4085)
        {
          printk(" - FAT12 Detected\n");
          pBPB->BS_OEMName[7] = 0;
          pBPB16->BS_VolLab[10] = 0;
          pBPB16->BS_FilSysType[7] = 0;
          printk(" - OEM Name:       %s\n", pBPB->BS_OEMName);
          printk(" - Volume Label:   %s\n", pBPB16->BS_VolLab);
          printk(" - FS Type:        %s\n", pBPB16->BS_FilSysType);
        }
        else if(iClusterCount < 65525)
        {
          printk(" - FAT16 Detected\n");
          pBPB->BS_OEMName[7] = 0;
          pBPB16->BS_VolLab[10] = 0;
          pBPB16->BS_FilSysType[7] = 0;
          printk(" - OEM Name:       %s\n", pBPB->BS_OEMName);
          printk(" - Volume Label:   %s\n", pBPB16->BS_VolLab);
          printk(" - FS Type:        %s\n", pBPB16->BS_FilSysType);
        }
        else
        {
          printk(" - FAT32 Detected\n");
          pBPB->BS_OEMName[7] = 0;
          pBPB32->BS_VolLab[10] = 0;
          pBPB32->BS_FilSysType[7] = 0;
          printk(" - OEM Name:       %s\n", pBPB->BS_OEMName);
          printk(" - Volume Label:   %s\n", pBPB32->BS_VolLab);
          printk(" - FS Type:        %s\n", pBPB32->BS_FilSysType);
        }
      }
      else
        printk(" - ERROR: Unable to locate boot sector\n");
    }
    else
      printk(" - ERROR: Invalid boot signature\n");
  }
  else
    printk(" - ERROR: Unable to read sector\n");


  return 0;
}
*/

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
CFATDriver::CFATDriver()
{
}

// -----------------------------------------------------------------------------
CFATDriver::~CFATDriver()
{
}

// -----------------------------------------------------------------------------
bool
CFATDriver::init(IBlockDevice * device)
{
  uint8_t * data = new uint8_t[512];
  bool bRetVal(false);

  printk("CFATDriver::init\n");

  if(device->read(0, 1, data) == true)
  {
//    SBPB   * pBPB   = (SBPB   *)(&data[0]);
    SBPB16 * pBPB16 = (SBPB16 *)(&data[36]);
    SBPB32 * pBPB32 = (SBPB32 *)(&data[36]);

    // Validate boot signature
    if((data[0x1FE] == 0x55) && (data[0x1FF] == 0xAA))
    {
      // Validate "FAT" string
      if(((pBPB16->BS_FilSysType[0] == 'F') && (pBPB16->BS_FilSysType[1] == 'A') && (pBPB16->BS_FilSysType[2] == 'T')) ||
         ((pBPB32->BS_FilSysType[0] == 'F') && (pBPB32->BS_FilSysType[1] == 'A') && (pBPB32->BS_FilSysType[2] == 'T')))
      {
        bRetVal = true;
      }
      else
        printk(" - ERROR: \"FAT\" string not found\n");
    }
    else
      printk(" - ERROR: Invalid boot signature\n");
  }
  else
    printk(" - ERROR: Unable to read sector\n");


  return bRetVal;
}
