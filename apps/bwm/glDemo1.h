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


#ifndef GLDEMO1_H
#define GLDEMO1_H


#include "glWidget.h"
#include "vhl/fixedPoint.h"


class CGLDemo1
 : public twl::CGLWidget
{
public:
  CGLDemo1(CWidget * parent = 0);
  virtual ~CGLDemo1();

  virtual void initializeGL();
  virtual void resizeGL(int w, int h);
  virtual void drawGL();

private:
  GLfixed yrot_;
};


#endif
