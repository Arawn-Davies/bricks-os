#ifndef SUPERCARD_H
#define SUPERCARD_H


#include "kernel/fileManager.h"
#include "inttypes.h"


struct CF_REGISTERS
{
  vuint16_t * data;
  vuint16_t * status;
  vuint16_t * command;
  vuint16_t * error;
  vuint16_t * sectorCount;
  vuint16_t * lba1;
  vuint16_t * lba2;
  vuint16_t * lba3;
  vuint16_t * lba4;
};


class CSuperCard
 : public IBlockDevice
{
public:
  CSuperCard();
  virtual ~CSuperCard();

  virtual int init();
  virtual bool removable();
  virtual bool inserted();

  virtual int read (uint32_t startSector, uint32_t sectorCount,       void * data);
  virtual int write(uint32_t startSector, uint32_t sectorCount, const void * data);

private:
  void changeMode(uint8_t mode);

  CF_REGISTERS cfRegisters_;
};


#endif
