/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "desktopscriptengine.h"

#include "containment.h"
#include "panel.h"

DesktopScriptEngine::DesktopScriptEngine(Plasma::Corona *corona, QObject *parent)
    : ScriptEngine(corona, parent)
{
}

QScriptValue DesktopScriptEngine::wrap(Plasma::Containment *c, QScriptEngine *engine)
{
    Containment *wrapper = isPanel(c) ? new Panel(c) : new Containment(c);
    return wrap(wrapper, engine);
}

QScriptValue DesktopScriptEngine::wrap(Containment *c, QScriptEngine *engine)
{
    return ScriptEngine::wrap(c, engine);
}

#include "desktopscriptengine.moc"
