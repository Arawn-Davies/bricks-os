#include "kernel/bricks.h"
#include "kernel/task.h"
#include "kernel/elf.h"
#include "kernel/syscall.h"
#include "kernel/srr.h"
#include "asm/irq.h"
#include "asm/cpu.h"
#include "video.h"
#include "keyboard.h"
#include "descriptor.h"
#include "mmap.h"
#include "multiboot.h"
#include "task.h"
#include "string.h"
#include "iostream"


extern char       start_text;
extern char       end_bss;
CIRQ              cIRQ;
CI386Video        cVideo;
CI386Keyboard     cKeyboard;
bool              bPAEEnabled;
CPCTask           taskTest;


// -----------------------------------------------------------------------------
int
loadELF32(void * file, CPCTask & task)
{
  static const char magic[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  Elf32_Ehdr * hdr = (Elf32_Ehdr *)file;
  
  // Check header magic
  for(int i(0); i < 16; i++)
    if(magic[i] != hdr->e_ident[i])
      return -1;
     
  // Load Sections
  for(int i(0); i < hdr->e_phnum; i++)
  {
    Elf32_Phdr * seg = (Elf32_Phdr *)((char *)file + hdr->e_phoff + hdr->e_phentsize * i);
    switch(seg->p_type)
    {
      case SHT_PROGBITS:
        std::cout<<" - "
                 <<"vaddr: "<<seg->p_vaddr
        //         <<", paddr: "<<seg->p_paddr
                 <<", foff: "<<seg->p_offset
                 <<", fsize: "<<seg->p_filesz
                 <<", msize: "<<seg->p_memsz
                 <<std::endl;
        task.aspace().addSection((char *)seg->p_vaddr, (char *)file + seg->p_offset, seg->p_filesz);
        break;
      default:
        std::cout<<"Unable to load segment type: "<<seg->p_type<<std::endl;
        return -1;
    };    
  }
  
  // Set task entry point
  task.entry((void *)hdr->e_entry);
  task.eState_ = TS_READY;
  CTaskManager::addTask(&task);
  
  return 0;
}

#define PAE_AUTO      0x00
#define PAE_FORCE_ON  0x01
#define PAE_FORCE_OFF 0x02
// -----------------------------------------------------------------------------
int
main(unsigned long magic, multiboot_info_t * mbi)
{
  int iRetVal(0);
  uint32_t iCPUFeatures(0);
  unsigned int iPAEPolicy(PAE_AUTO);
  unsigned char * pFirstFreeByte = 0;
  uint64_t iMemFree(0);
  uint64_t iMemTop(0);
  uint64_t iMemReserved(0);
  uint64_t iMemKernel(0);
  unsigned int iMemPageCount(0);

  if(cIRQ.init() == -1)
    iRetVal = -1;
  if(cVideo.init() == -1)
    iRetVal = -1;
  if(cKeyboard.init() == -1)
    iRetVal = -1;

  CTaskManager::setStandardOutput(&cVideo);
  CTaskManager::setStandardInput(&cKeyboard);

  if(magic == MULTIBOOT_BOOTLOADER_MAGIC)
  {
    if((mbi->flags & (1<<6)) == 0)
    {
      std::cout<<"ERROR: No multiboot memory map present"<<std::endl;
      CCPU::halt();
    }
  }
  else
  {
    std::cout<<"ERROR: Multiboot loader information not present"<<std::endl;
    CCPU::halt();
  }

  // --------------------------------
  // Setup Physical Memory Management
  // --------------------------------
  // Memory Map Type:
  // 0x01 memory, available to OS
  // 0x02 reserved, not available (e.g. system ROM, memory-mapped device)
  // 0x03 ACPI Reclaim Memory (usable by OS after reading ACPI tables) (60 KiB ?)
  // 0x04 ACPI NVS Memory (OS is required to save this memory between NVS sessions) (4 KiB ?)
  //std::cout<<"Memory Map:"<<std::endl;
  for(memory_map_t * mmap = (memory_map_t *) mbi->mmap_addr; (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length; mmap = (memory_map_t *) ((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
  {
    //std::cout<<" - Type: "<<mmap->type<<", Addr: "<<mmap->base_addr_low<<", Size: "<<mmap->length_low<<std::endl;
    // Determine top of memory
    // Determine free memory
    if(mmap->type == 1)
    {
      if(iMemTop < (mmap->base_addr_low + mmap->length_low))
        iMemTop = mmap->base_addr_low + mmap->length_low;
      iMemFree += mmap->length_low;
    }
  }

  if(iMemFree < (2 * 1024 * 1024))
  {
    std::cout<<"ERROR: "<<iMemFree/1024<<"KiB free memory, need at least 2048KiB"<<std::endl;
    CCPU::halt();
  }

  iMemReserved = iMemTop - iMemFree;
  iMemPageCount = iMemTop / 4096;
  
  // Kernel location and size
  //std::cout<<"Kernel: start: "<<(unsigned int)&start_text<<", size: "<<(unsigned int)&end_bss - (unsigned int)&start_text<<std::endl;
  if((unsigned char *)&end_bss > pFirstFreeByte)
    pFirstFreeByte = (unsigned char *)&end_bss;
  // Module location and size
  if((mbi->flags & (1<<3)) && ((int)mbi->mods_count > 0))
  {
    module_t * mod = (module_t *) mbi->mods_addr;
    for(unsigned int i(0); i < mbi->mods_count; i++, mod++)
    {
      //std::cout<<"Module: start: "<<mod->mod_start<<", size: "<<mod->mod_end - mod->mod_start<<std::endl;
      if((unsigned char *)mod->mod_end > pFirstFreeByte)
        pFirstFreeByte = (unsigned char *)mod->mod_end;
    }
  }

  // Create physical memory map, all pages will be initially marked used
  uint8_t * pMemoryMap = pFirstFreeByte;
  init_mmap(pMemoryMap, iMemPageCount);
  //std::cout<<"MemMap: start: "<<(unsigned int)pFirstFreeByte<<", size: "<<iMemPageCount<<std::endl;
  pFirstFreeByte += iMemPageCount;
  
  // Free memory that multiboot/bios reports available
  for(memory_map_t * mmap = (memory_map_t *) mbi->mmap_addr; (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length; mmap = (memory_map_t *) ((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
    if(mmap->type == 1)
      physFreeRange(mmap->base_addr_low, mmap->length_low);

  // Allocate kernel stack (start: 1MiB, size: 4KiB)
  physAllocRange((uint64_t)0x00100000, 0x00001000);
  // Allocate kernel
  physAllocRange((uint64_t)&start_text, (unsigned int)&end_bss - (unsigned int)&start_text);
  // Allocate modules
  if((mbi->flags & (1<<3)) && ((int)mbi->mods_count > 0))
  {
    module_t * mod = (module_t *) mbi->mods_addr;
    for(unsigned int i(0); i < mbi->mods_count; i++, mod++)
      physAllocRange((uint64_t)mod->mod_start, mod->mod_end - mod->mod_start);
  }
  // Allocate memory map
  physAllocRange((uint64_t)pMemoryMap, iMemPageCount);
  
  // ----------------
  // Setup Interrupts
  // ----------------
  // Allocate & Create IDT (Max.: 8-Byte *  256 =  2-KiB)
  physAllocRange((uint64_t)pFirstFreeByte, sizeof(SDescriptor) *  256);
  //std::cout<<"IDT   : start: "<<(unsigned int)pFirstFreeByte<<", size: "<<sizeof(SDescriptor) * 256<<std::endl;
  init_idt((SDescriptor *)pFirstFreeByte,  256);
  pFirstFreeByte += sizeof(SDescriptor) *  256;

  // ---------
  // Setup GDT
  // ---------
  // Align to page boundries (4KiB)
  pFirstFreeByte = (unsigned char *)PAGE_ALIGN_UP_4K(pFirstFreeByte);
  // Allocate & Create GDT (Max.: 8-Byte * 8192 = 64-KiB)
  physAllocRange((uint64_t)pFirstFreeByte, sizeof(SDescriptor) * 8192);
  //std::cout<<"GDT   : start: "<<(unsigned int)pFirstFreeByte<<", size: "<<sizeof(SDescriptor) * 8192<<std::endl;
  init_gdt((SDescriptor *)pFirstFreeByte, 8192);
  pFirstFreeByte += sizeof(SDescriptor) * 8192;
  
  // ---------
  // Setup CPU
  // ---------
  if(cpuidPresent() == true)
  {
    uint32_t iCPUIDMax;
    char vendorID[13];
    
    // Get CPU information
    cpuid(0, &iCPUIDMax, (uint32_t *)&vendorID[0], (uint32_t *)&vendorID[8], (uint32_t *)&vendorID[4]);
    vendorID[12] = 0;
    
    // Display information
    //std::cout<<"CPU:"<<std::endl;
    //std::cout<<" - Vendor ID: "<<vendorID<<std::endl;
    
    if(iCPUIDMax >= 1)
    {
      //static const char * sType[] = {
      //    "Original OEM Processor"
      //  , "Intel OverDrive Processor"
      //  , "Dual Processor"
      //  , "Intel Reserved"
      //};
      uint32_t iDummy;
      uint32_t iVersion;
      
      //unsigned int iCPUType;
      //unsigned int iCPUFamily;
      //unsigned int iCPUModel;
      //unsigned int iCPUStepping;

      // Get CPU information
      cpuid(1, &iVersion, &iDummy, &iDummy, &iCPUFeatures);
      //iCPUType     = (iVersion & 0x00003000) >> 12;
      //iCPUFamily   = (iVersion & 0x00000f00) >>  8;
      //iCPUModel    = (iVersion & 0x000000f0) >>  4;
      //iCPUStepping = (iVersion & 0x0000000f);

      // Display information
      //std::cout<<" - Type:      "<<sType[iCPUType]<<std::endl;
      //std::cout<<" - Family:    "<<iCPUFamily<<std::endl;
      //std::cout<<" - Model:     "<<iCPUModel<<std::endl;
      //std::cout<<" - Stepping:  "<<iCPUStepping<<std::endl;
    }
  }
  else
  {
    // CPUID instuction not available, has to be a 386 or an early 486
    //std::cout<<"CPU: 386/486"<<std::endl;
  }
  
  // Use PAE?
  if(iCPUFeatures & CPUID_FT_PAE)
  {
    // PAE can be enabled/disabled on the command line (if PAE is present)
    switch(iPAEPolicy)
    {
      case PAE_AUTO:
        // PAE will only be enabled if physical memory is located above the 4GiB limit
        // Note: Do we need PAE for page level data execution prevention?
        //       Why not use segment level data execution prevention?
        if(iMemTop > (uint64_t(4 * 1024 * 1024) * 1024))
          bPAEEnabled = true;
        else
          bPAEEnabled = false;
        break;
      case PAE_FORCE_ON:
        bPAEEnabled = true;
        break;
      case PAE_FORCE_OFF:
        bPAEEnabled = false;
        break;
    };
    
    // Display PAE status
    if(bPAEEnabled == true)
      std::cout<<"PAE enabled"<<std::endl;
    else
      std::cout<<"PAE disabled"<<std::endl;
  }
  else
  {
    bPAEEnabled = false;
  }
  
  // ---------------------
  // Setup Paging
  // Setup Task Management
  // ---------------------
  init_task();
  std::cout<<"Paging Enabled!"<<std::endl;

  /*
  // Boot device
  if(mbi->flags & (1<<1))
  {
    //printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
  }
    
  // Command line
  if(mbi->flags & (1<<2))
  {
    std::cout<<"Command line: "<<(char *)mbi->cmdline<<std::endl;
  }
  */

  // ------------
  // Load modules
  // ------------
  if((mbi->flags & (1<<3)) && ((int)mbi->mods_count > 0))
  {
    //std::cout<<"Found Modules: "<<(int)mbi->mods_count<<std::endl;
    module_t * mod = (module_t *) mbi->mods_addr;
    for(unsigned int i(0); i < mbi->mods_count; i++, mod++)
    {
      std::cout<<"Loading: "<<(char *)mod->string<<std::endl;
        
      // Try to load as elf32 file
      taskTest.init();
      taskTest.aspace().addRange(taskMain.aspace(), 0, 0x00400000);  // Bottom 4MiB
      if(loadELF32((void *)mod->mod_start, taskTest) == 0)
        std::cout<<" - Done"<<std::endl;
      else
        std::cout<<" - Unknown File"<<std::endl;
    }
  }
  
  // Enable Timer IRQ
  cIRQ.enable(0x20);
  
  iMemKernel = iMemTop - iMemReserved - (freePageCount() * 4096);
  std::cout<<"Memory size:     "<<iMemTop/1024<<"KiB"<<std::endl;
  std::cout<<"Memory reserved: "<<iMemReserved/1024<<"KiB"<<std::endl;
  std::cout<<"Memory kernel:   "<<iMemKernel/1024<<"KiB"<<std::endl;
  std::cout<<"Memory free:     "<<freePageCount() * 4<<"KiB"<<std::endl;

  return bricks_main();
}

