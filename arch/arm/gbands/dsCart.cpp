#include "dsCart.h"
#include "iostream"


// -----------------------------------------------------------------------------
CDSCart::CDSCart()
{
}

// -----------------------------------------------------------------------------
CDSCart::~CDSCart()
{
  CInterruptManager::detach(13, this);
}

// -----------------------------------------------------------------------------
int
CDSCart::init()
{
  CInterruptManager::attach(13, this);

  return 0;
}

// -----------------------------------------------------------------------------
int
CDSCart::isr(int irq)
{
  std::cout<<"CDSCart::isr: gba cart ejected"<<std::endl;

  return 0;
}