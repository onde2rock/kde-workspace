/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef CLOCK_H
#define CLOCK_H

#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QGraphicsItem>
#include <QColor>

#include <Plasma/Containment>
#include <Plasma/DataEngine>

#include <libplasmaclock/clockapplet.h>
#include "ui_clockConfig.h"

class QTimer;

namespace Plasma
{
    class Svg;
    class Dialog;
}

class Clock : public ClockApplet
{
    Q_OBJECT
    public:
        Clock(QObject *parent, const QVariantList &args);
        ~Clock();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void setPath(const QString&);
        void constraintsEvent(Plasma::Constraints constraints);
        QPainterPath shape() const;

    public slots:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

    protected:
        void createClockConfigurationInterface(KConfigDialog *parent);
        void changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone);

    protected slots:
        void clockConfigAccepted();
        void moveSecondHand();

    private:
        void drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName);
        void connectToEngine();

        bool m_showSecondHand;
        bool m_fancyHands;
        bool m_showTimezoneString;
        Plasma::Svg* m_theme;
        QTime m_time;
        QTime m_lastTimeSeen;
        QTimer *m_secondHandUpdateTimer;
        int m_animationStart;
        /// Designer Config file
        Ui::clockConfig ui;
};

K_EXPORT_PLASMA_APPLET(clock, Clock)

#endif
