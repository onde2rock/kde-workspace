/*
  RISC OS KWin client
  
  Copyright 2000
    Rik Hemsley <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include "MaximiseButton.h"
#include "Static.h"
#include "Manager.h"

namespace RiscOS
{

MaximiseButton::MaximiseButton(QWidget * parent, Manager * client)
  : Button(parent, client, Max)
{
}

  void
MaximiseButton::setOn(bool on)
{
  if (on)
    setType(Unmax);
  else
    setType(Max);
}

  void
MaximiseButton::mouseReleaseEvent(QMouseEvent * e)
{
  Button::mouseReleaseEvent(e);

  if (!rect().contains(e->pos()))
    return;

  switch (e->button())
  {
    case RightButton:
      client()->maximizeNoRaise();
      break;

    case MidButton:
      client()->maximizeVertically();
      break;

    case LeftButton:
    default:
      client()->maximizeAndRaise();
      break;
  }
}

} // End namespace

// vim:ts=2:sw=2:tw=78
