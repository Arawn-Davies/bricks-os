#include "desktop.h"
#include "frameBuffer.h"


namespace twl
{


//---------------------------------------------------------------------------
CDesktop::CDesktop()
 : CFrame(0, CFrame::ftNone)
 , taskBar_(this, CFrame::ftRaised)
 , startButton_(&taskBar_, "Start")
{
  int iWidth(pFB->rect().width());
  int iHeight(pFB->rect().height());

  frame(false);
  rect(0, 0, iWidth, iHeight);

  taskBar_.rect(-1, iHeight - 13, iWidth + 3, 15);
  startButton_.rect(2, 2, 24, 10);
}

//---------------------------------------------------------------------------
CDesktop::~CDesktop()
{
}

//---------------------------------------------------------------------------
bool
CDesktop::eventRedraw(const CEventRedraw & event)
{
  pWindowImpl_->fill(clDesktop);
  pWindowImpl_->drawText(0, 0, "Bricks-OS", clWhite);

  return true;
}


};