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


#include "kernel/debug.h"
#include "kernel/ibmPartitionDriver.h"


// -----------------------------------------------------------------------------
struct SPartitionTableRecord
{
  uint8_t  flag;
  uint8_t  legacy1;
  uint8_t  legacy2;
  uint8_t  legacy3;
  uint8_t  type;
  uint8_t  legacy4;
  uint8_t  legacy5;
  uint8_t  legacy6;
  uint32_t startSector;
  uint32_t sectorCount;
} __attribute__ ((__packed__));

// -----------------------------------------------------------------------------
struct SPartitionTable
{
  SPartitionTableRecord record[4];
} __attribute__ ((__packed__));

// -----------------------------------------------------------------------------
enum EPartitionType
{
  ptEMPTY      = 0x00,
  ptFAT12      = 0x01, // Up to 15MB
  ptFAT16      = 0x04, // Up to 32MB
  ptEXTENDED   = 0x05, // Within first 8GB of drive
  ptFAT16_B    = 0x06, // Up to 2GB
  ptNTFS       = 0x07,
  ptFAT32      = 0x0b, // Up to 2TB
  ptFAT32_B    = 0x0c, // Up to 2TB
  ptFAT16_C    = 0x0e, // Up to 2GB
  ptEXTENDED_B = 0x0f,
  ptSWAP       = 0x82,
  ptLINUX      = 0x83,
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
CIBMPartitionDriver::CIBMPartitionDriver()
{
}

// -----------------------------------------------------------------------------
CIBMPartitionDriver::~CIBMPartitionDriver()
{
}

// -----------------------------------------------------------------------------
bool
CIBMPartitionDriver::init(IBlockDevice * device)
{
  uint8_t * data = new uint8_t[512];
  int iPartitionCount(0);
  bool bRetVal(false);

  //printk("CIBMPartitionDriver::init\n");

  // Read MBR (Master Boot Record) from disk
  if(device->read(0, 1, data) == 1)
  {
    // Validate if disk uses IBM partitioning
    if((data[0x1FE] == 0x55) && (data[0x1FF] == 0xAA))
    {
      SPartitionTable * pTable = (SPartitionTable *)(&data[0x1BE]);

      // Check partition table (Max. 4 entries)
      for(int i(0); i < 4; i++)
      {
        // Validate flags (0x80 = bootable, 0x00 = not bootable)
        if(((pTable->record[i].flag == 0x80) || (pTable->record[i].flag == 0x00)) && (pTable->record[i].startSector > 0))
        {
          // We have a valid partition entry
          iPartitionCount++;
          bRetVal = true;

          // Add partition to file system
          printk("CIBMPartitionDriver::init: valid partition found\n");
          CFileSystem::addBlockDevice(new CPartition(device, pTable->record[i].startSector, pTable->record[i].sectorCount));
        }
      }
    }
  }
  else
    printk("CIBMPartitionDriver::init: ERROR: Unable to read boot sector\n");

  delete data;

  return bRetVal;
}
