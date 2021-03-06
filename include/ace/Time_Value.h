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
 *
 * Based on the Adaptive Communication Environment (ACE)
 */


#ifndef ACE_TIME_VALUE_H
#define ACE_TIME_VALUE_H


#include "sys/time.h"


class ACE_Time_Value
{
public:
  ACE_Time_Value();
  ACE_Time_Value(long sec, long usec = 0);
  ACE_Time_Value(const struct timeval & t);

  void set(long sec, long usec);
  void set(const timeval & t);

  unsigned long msec (void) const;
  void msec (long);

private:
  void normalize();

  timeval tv_;
};


#endif
