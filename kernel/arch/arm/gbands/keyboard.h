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


#ifndef ARCH_KEYBOARD_H
#define ARCH_KEYBOARD_H


#include "kernel/fs.h"
#include "kernel/interrupt.h"
#include "kernel/inputManager.h"
#include "asm/arch/memory.h"


class CGBAKeyboard
 : public IInterruptServiceRoutine
 , public CAFileIOBufferedRead
 , public CAControllerDevice
{
public:
  CGBAKeyboard();
  virtual ~CGBAKeyboard();

  virtual int init();

  // Inherited from IInterruptServiceRoutine
  virtual int isr(int irq) INTERRUPT_CODE;

  // Inherited from CAControllerDevice
  uint32_t getButtonState();

private:
  uint16_t iKeys_;
#ifdef NDS7
  uint16_t iKeysXY_;
#endif
};


#endif
