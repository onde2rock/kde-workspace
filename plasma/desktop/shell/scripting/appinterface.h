/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef APPINTERFACE
#define APPINTERFACE

#include <QObject>
#include <QRectF>

namespace Plasma
{
    class Applet;
    class Containment;
    class Corona;
} // namespace Plasma

class AppInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool locked READ coronaLocked WRITE lockCorona)
    Q_PROPERTY(bool hasBattery READ hasBattery)
    Q_PROPERTY(int screenCount READ screenCount)
    Q_PROPERTY(QList<int> activityIds READ activityIds)
    Q_PROPERTY(QList<int> panelIds READ panelIds)

public:
    AppInterface(Plasma::Corona *corona, QObject *parent = 0);

    bool hasBattery() const;
    int screenCount() const;
    QList<int> activityIds() const;
    QList<int> panelIds() const;
    bool coronaLocked() const;

public Q_SLOTS:
    QRectF screenGeometry(int screen) const;
    void lockCorona(bool locked);
    void sleep(int ms);

Q_SIGNALS:
    void print(const QString &string);

private:
    Plasma::Corona *m_corona;
};

#endif
