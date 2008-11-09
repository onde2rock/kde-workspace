/***************************************************************************
 *   x11embedpainter.h                                                     *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "x11embedpainter.h"

#include <QtCore/QSet>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <KDebug>
#include <KGlobal>


#define MAX_PAINTS_PER_SEC 20
#define MIN_TIME_BETWEEN_PAINTS (1000 / MAX_PAINTS_PER_SEC)


namespace SystemTray
{
namespace FDO
{


class X11EmbedPainter::Singleton
{
public:
    X11EmbedPainter instance;
};

K_GLOBAL_STATIC(X11EmbedPainter::Singleton, singleton)


X11EmbedPainter* X11EmbedPainter::self()
{
    return &singleton->instance;
}


class X11EmbedPainter::Private
{
public:
    Private(X11EmbedPainter *parent)
        : q(parent)
    {
        lastPaintTime = QTime::currentTime();
        lastPaintTime.addMSecs(-MIN_TIME_BETWEEN_PAINTS);

        delayedPaintTimer.setSingleShot(true);
        connect(&delayedPaintTimer, SIGNAL(timeout()),
                q, SLOT(performUpdates()));
    }

    X11EmbedPainter *q;
    QSet<X11EmbedContainer*> containers;
    QTime lastPaintTime;
    QTimer delayedPaintTimer;
};


X11EmbedPainter::X11EmbedPainter()
    : d(new Private(this))
{
}


X11EmbedPainter::~X11EmbedPainter()
{
    delete d;
}


void X11EmbedPainter::updateContainer(X11EmbedContainer *container)
{
    d->containers.insert(container);

    if (!d->delayedPaintTimer.isActive()) {
        int msecsToNextPaint = MIN_TIME_BETWEEN_PAINTS - d->lastPaintTime.elapsed();
        if (msecsToNextPaint > 0 && msecsToNextPaint < MIN_TIME_BETWEEN_PAINTS) {
            kDebug() << "Delaying paint by" << msecsToNextPaint << "msecs";
            d->delayedPaintTimer.start(msecsToNextPaint);
        } else {
            d->delayedPaintTimer.start(0);
        }
    }
}


void X11EmbedPainter::performUpdates()
{
    QMultiHash<QWidget*, X11EmbedContainer*> containersByParent;

    foreach (X11EmbedContainer *container, d->containers) {
        QWidget *topWidget = container;
        while (topWidget->parentWidget()) {
            topWidget = topWidget->parentWidget();
        }
        containersByParent.insert(topWidget, container);
        container->setUpdatesEnabled(false);
    }

    foreach (QWidget *parent, containersByParent.uniqueKeys()) {
        QList<X11EmbedContainer*> containers = containersByParent.values(parent);
        containersByParent.remove(parent);

        QRegion paintRegion;
        foreach (X11EmbedContainer *container, containers) {
            QRect rect = QRect(container->mapTo(parent, QPoint(0, 0)), container->size());
            paintRegion = paintRegion.united(rect);
        }

        QPixmap background = QPixmap(parent->size());
        parent->render(&background, paintRegion.boundingRect().topLeft(), paintRegion);

        foreach (X11EmbedContainer *container, containers) {
            QRect rect = QRect(container->mapTo(parent, QPoint(0, 0)), container->size());
            container->setBackgroundPixmap(background.copy(rect));
        }
    }

    foreach (X11EmbedContainer *container, d->containers) {
        container->setUpdatesEnabled(true);
    }

    d->containers.clear();
    d->lastPaintTime.start();
}


}
}

#include "x11embedpainter.moc"
