/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ABSTRACTICON_H
#define ABSTRACTICON_H

#include <QGraphicsWidget>

class AbstractIcon : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit AbstractIcon(QGraphicsItem *parent = 0);
        ~AbstractIcon();

        void setIconSize(int height);
        int iconSize() const;

        void setSelected(bool selected);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        void setName(const QString &name);
        QString name() const;

        void expand();
        void collapse();

        /**
         * return the background image
         */
        virtual QPixmap pixmap(const QSize &size) = 0;
        /**
         * return the mime data for d&d
         */
        virtual QMimeData* mimeData() = 0;

        static const int DEFAULT_ICON_SIZE = 16;

    Q_SIGNALS:
        void hoverEnter(AbstractIcon *applet);
        void hoverLeave(AbstractIcon *applet);
        void selected(AbstractIcon *applet);
        void doubleClicked(AbstractIcon *applet);

    protected:
        //listen to events and emit signals
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void resizeEvent(QGraphicsSceneResizeEvent *);

    private:
        QString m_name;
        int m_iconHeight;
        QSizeF m_maxSize;
        bool m_selected : 1;
        bool m_hovered : 1;
};

#endif //APPLETICON_H