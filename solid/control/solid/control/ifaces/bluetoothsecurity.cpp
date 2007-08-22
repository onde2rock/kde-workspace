/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2007 Juan González <jaguilera@opsiland.info>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "bluetoothsecurity.h"

Solid::Control::Ifaces::BluetoothSecurity::BluetoothSecurity(QObject *parent)
        : QObject(parent)
{}

Solid::Control::Ifaces::BluetoothSecurity::BluetoothSecurity(const QString & interface, QObject * parent)
    : QObject(parent)
{
    Q_UNUSED(interface)
}

Solid::Control::Ifaces::BluetoothSecurity::~BluetoothSecurity()
{}


#include "bluetoothsecurity.moc"
