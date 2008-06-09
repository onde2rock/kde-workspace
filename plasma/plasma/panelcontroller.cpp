 /*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "panelcontroller.h"

#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>

#include <KIcon>
#include <KColorUtils>

#include <plasma/theme.h>
#include <plasma/containment.h>

#include "positioningruler.h"

class PanelController::ToolButton: public QToolButton
{
public:
    ToolButton(QWidget *parent)
       : QToolButton(parent)
    {
         
    }
    
    ~ToolButton()
    {}

    void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.translate(0.5, 0.5);

        QStyleOptionToolButton buttonOpt;
        initStyleOption(&buttonOpt);

        QColor backgroundColor;
        if ((buttonOpt.state & QStyle::State_MouseOver) || isChecked()) {
            backgroundColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        } else {
            backgroundColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        }

        backgroundColor.setAlphaF(0.4);
        QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        
        buttonOpt.palette.setColor(QPalette::Foreground, textColor);
        buttonOpt.palette.setColor(QPalette::ButtonText, textColor);

        textColor.setAlphaF(0.4);
        painter.setPen(textColor);
        painter.setBrush(backgroundColor);
        painter.drawPath(Plasma::roundedRectangle(event->rect().adjusted(1,1,-1,-1), 4));

        style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, &painter, this);
    }
};

class PanelController::ButtonGroup: public QFrame
{
public:
    ButtonGroup(QWidget *parent)
       : QFrame(parent)
    {
         
    }
    
    ~ButtonGroup()
    {}

    void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.translate(0.5, 0.5);

        QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);


        int x;
        int y;
        if (event->rect().height() > event->rect().width()) {
            x = event->rect().left();
            y = event->rect().bottom();
        } else {
            x = event->rect().right();
            y = event->rect().top();
        }

        QLinearGradient gradient(x, y, event->rect().right(), event->rect().bottom());
        textColor.setAlphaF(0);
        gradient.setColorAt(0.1, textColor);
        textColor.setAlphaF(0.4);
        gradient.setColorAt(1, textColor);

        painter.setBrush(Qt::NoBrush);
        QPen pen;
        pen.setBrush(gradient);
        painter.setPen(pen);
        painter.drawPath(Plasma::roundedRectangle(event->rect().adjusted(1,1,-1,-1), 4));
    }
};

class PanelController::ResizeHandle: public QWidget
{
public:
    ResizeHandle(QWidget *parent)
       : QWidget(parent),
         m_mouseOver(false)
    {
        setCursor(Qt::SizeVerCursor);
    }

    QSize sizeHint() const
    {
        return QSize(4, 4);
    }

    void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        QColor backColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);

        if (m_mouseOver) {
            backColor.setAlphaF(0.50);
        } else {
            backColor.setAlphaF(0.30);
        }

        painter.fillRect(event->rect(), backColor);

        // draw 3 dots to resemble other resize handles
        int diameter = qMin(width(), height());
        QRect dotRect(QPoint(0,0), QSize(diameter, diameter));
        dotRect.moveCenter(mapFromParent(geometry().center()));

        painter.setRenderHint(QPainter::Antialiasing, true);

        paintDot(&painter, dotRect);

        //other two dots
        if (size().width() > size().height()) {
            dotRect.translate(-diameter*2, 0);
            paintDot(&painter, dotRect);
            dotRect.translate(diameter*4, 0);
            paintDot(&painter, dotRect);
        } else {
            dotRect.translate(0, -diameter*2);
            paintDot(&painter, dotRect);
            dotRect.translate(0, diameter*4);
            paintDot(&painter, dotRect);
        }
    }

protected:
    void enterEvent(QEvent * event)
    {
        m_mouseOver = true;
        update();
    }

    void leaveEvent(QEvent * event)
    {
        m_mouseOver = false;
        update();
    }

private:
    void paintDot(QPainter *painter, QRect dotRect)
    {
        QLinearGradient gradient(dotRect.left(), dotRect.top(), dotRect.left(), dotRect.bottom());
        QColor firstColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        firstColor.setAlphaF(0.6);
        QColor secondColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        secondColor.setAlphaF(0.6);
        gradient.setColorAt(0, firstColor);
        gradient.setColorAt(1, secondColor);

        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(dotRect);
    }

    bool m_mouseOver;
};

class PanelController::Private
{
public:
    Private(PanelController *panelControl)
       : q(panelControl),
         containment(0),
         orientation(Qt::Horizontal),
         location(Plasma::BottomEdge),
         extLayout(0),
         layout(0),
         dragging(NoElement),
         startDragPos(0,0),
         leftAlignTool(0),
         centerAlignTool(0),
         rightAlignTool(0)
    {
    }

    ToolButton *addTool(const QString icon, const QString iconText, QWidget *parent,  Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon, bool checkButton = false)
    {
        //TODO take advantage of setDefaultAction using the containment's actions if possible
        ToolButton *tool = new ToolButton(parent);

        tool->setIcon(KIcon(icon));
        tool->setText(iconText);
        tool->setToolButtonStyle(style);

        if (style == Qt::ToolButtonIconOnly) {
            tool->setToolTip(iconText);
        }

        tool->setCheckable(checkButton);
        tool->setAutoExclusive(checkButton);

        return tool;
    }

    void resizePanelHeight(const int newHeight)
    {
        if (!containment) {
            return;
        }

        switch (location) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            containment->resize(QSize(newHeight, (int)containment->size().height()));
            containment->setMinimumSize(QSize(newHeight, (int)containment->minimumSize().height()));
            containment->setMaximumSize(QSize(newHeight, (int)containment->maximumSize().height()));
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
        default:
            containment->resize(QSize((int)containment->size().width(), newHeight));
            containment->setMinimumSize(QSize((int)containment->minimumSize().width(), newHeight));
            containment->setMaximumSize(QSize((int)containment->maximumSize().width(), newHeight));
            break;
       }
    }

    void rulersMoved(int offset, int minLength, int maxLength)
    {
         if (!containment) {
            return;
         }

         QSize preferredSize(containment->size().toSize());

         switch (location) {
         case Plasma::LeftEdge:
         case Plasma::RightEdge:
             containment->resize(QSize((int)containment->size().width(), qBound(minLength, preferredSize.height(), maxLength)));
             containment->setMinimumSize(QSize((int)containment->minimumSize().width(), minLength));
             containment->setMaximumSize(QSize((int)containment->maximumSize().width(), maxLength));
             break;
         case Plasma::TopEdge:
         case Plasma::BottomEdge:
         default:
             containment->resize(QSize(qBound(minLength, preferredSize.width(), maxLength), (int)containment->size().height()));
             containment->setMinimumSize(QSize(minLength, (int)containment->minimumSize().height()));
             containment->setMaximumSize(QSize(maxLength, (int)containment->maximumSize().height()));
             break;
        }

        emit q->offsetChanged(offset);
    }

    void alignToggled(bool toggle)
    {
        if (!toggle) {
            return;
        }

        if (q->sender() == leftAlignTool) {
            emit q->alignmentChanged(Qt::AlignLeft);
            ruler->setAlignment(Qt::AlignLeft);
        } else if (q->sender() == centerAlignTool) {
            emit q->alignmentChanged(Qt::AlignCenter);
            ruler->setAlignment(Qt::AlignCenter);
        } else if (q->sender() == rightAlignTool) {
            emit q->alignmentChanged(Qt::AlignRight);
            ruler->setAlignment(Qt::AlignRight);
        }

        emit q->offsetChanged(0);
        ruler->setOffset(0);
    }

     enum DragElement { NoElement = 0,
                        ResizeHandleElement,
                        PanelControllerElement
                      };

    PanelController *q;
    Plasma::Containment *containment;
    Qt::Orientation orientation;
    Plasma::Location location;
    QBoxLayout *extLayout;
    QBoxLayout *layout;
    QBoxLayout *alignLayout;
    DragElement dragging;
    QPoint startDragPos;

    //Alignment buttons
    ToolButton *leftAlignTool;
    ToolButton *centerAlignTool;
    ToolButton *rightAlignTool;

    ResizeHandle *panelHeightHandle;
    PositioningRuler *ruler;

    static const int minimumHeight = 10;
};

PanelController::PanelController(QWidget* parent)
   : QWidget(parent),
     d(new Private(this))
{
    setWindowFlags(Qt::Popup);

    //Resize handles
    d->panelHeightHandle = new ResizeHandle(this);

    //layout setup
    d->extLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    d->extLayout->setContentsMargins(0, 1, 0, 0);
    setLayout(d->extLayout);
    d->extLayout->addWidget(d->panelHeightHandle);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    d->layout->setContentsMargins(4, 4, 4, 4);
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        d->layout->setDirection(QBoxLayout::RightToLeft);
    } else {
        d->layout->setDirection(QBoxLayout::LeftToRight);
    }
    d->layout->setSpacing(4);
    d->layout->addStretch();
    d->extLayout->addItem(d->layout);

    //Add buttons

    //alignment
    //first the container
    QFrame *alignFrame = new ButtonGroup(this);
    d->alignLayout = new QBoxLayout(d->layout->direction(), this);
    alignFrame->setLayout(d->alignLayout);
    d->layout->addWidget(alignFrame);
    
    d->leftAlignTool = d->addTool("format-justify-left", i18n("Align panel to left"), alignFrame,  Qt::ToolButtonIconOnly, true);
    d->alignLayout->addWidget(d->leftAlignTool);
    d->leftAlignTool->setChecked(true);
    connect(d->leftAlignTool, SIGNAL(toggled(bool)), this, SLOT(alignToggled(bool)));

    d->centerAlignTool = d->addTool("format-justify-center", i18n("Align panel to center"), alignFrame,  Qt::ToolButtonIconOnly, true);
    d->alignLayout->addWidget(d->centerAlignTool);
    connect(d->centerAlignTool, SIGNAL(clicked(bool)), this, SLOT(alignToggled(bool)));

    d->rightAlignTool = d->addTool("format-justify-right", i18n("Align panel to right"), alignFrame,  Qt::ToolButtonIconOnly, true);
    d->alignLayout->addWidget(d->rightAlignTool);
    connect(d->rightAlignTool, SIGNAL(clicked(bool)), this, SLOT(alignToggled(bool)));

    d->layout->addStretch();

    //other buttons
    ToolButton *addWidgetTool = d->addTool("list-add", i18n("Add Widgets"), this);
    d->layout->addWidget(addWidgetTool);
    connect(addWidgetTool, SIGNAL(clicked()), this, SIGNAL(showAddWidgets()));
    connect(addWidgetTool, SIGNAL(clicked()), this, SLOT(hideController()));

    ToolButton *removePanelTool = d->addTool("list-remove", i18n("Remove this panel"), this);
    d->layout->addWidget(removePanelTool);
    connect(removePanelTool, SIGNAL(clicked()), this, SIGNAL(removePanel()));
    connect(removePanelTool, SIGNAL(clicked()), this, SLOT(hideController()));

    d->layout->addSpacing(20);
    ToolButton *closeControllerTool = d->addTool("window-close", i18n("Close this configuration window"), this, Qt::ToolButtonIconOnly, false);
    d->layout->addWidget(closeControllerTool);
    connect(closeControllerTool, SIGNAL(clicked()), this, SLOT(hideController()));

    d->ruler = new PositioningRuler(this);
    connect(d->ruler, SIGNAL(rulersMoved(int, int, int)), this, SLOT(rulersMoved(int, int, int)));
    d->extLayout->addWidget(d->ruler);
}

PanelController::~PanelController()
{
    delete d;
}

void PanelController::setContainment( Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    d->containment = containment;

    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        d->ruler->setAvailableLength(screenGeom.height());
        d->ruler->setMaxLength(qMin((int)containment->maximumSize().height(), screenGeom.height()));
        d->ruler->setMinLength(containment->minimumSize().height());
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        d->ruler->setAvailableLength(screenGeom.width());
        d->ruler->setMaxLength(qMin((int)containment->maximumSize().width(), screenGeom.width()));
        d->ruler->setMinLength(containment->minimumSize().width());
        break;
    }

}

QSize PanelController::sizeHint() const
{
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        return QSize(QWidget::sizeHint().width(), screenGeom.height());
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        return QSize(screenGeom.width(), QWidget::sizeHint().height());
        break;
    }
}

QPoint PanelController::positionForPanelGeometry(const QRect &panelGeom) const
{
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (d->location) {
    case Plasma::LeftEdge:
        return QPoint(panelGeom.right(), screenGeom.top());
        break;
    case Plasma::RightEdge:
        return QPoint(panelGeom.left() - width(), screenGeom.top());
        break;
    case Plasma::TopEdge:
        return QPoint(screenGeom.left(), panelGeom.bottom());
        break;
    case Plasma::BottomEdge:
    default:
        return QPoint(screenGeom.left(), panelGeom.top() - height());
        break;
    }
}

void PanelController::setLocation(const Plasma::Location &loc)
{
    if (d->location == loc) {
        return;
    }

    d->location = loc;
    d->ruler->setLocation(loc);
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (loc) {
    case Plasma::LeftEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        //The external layout gwts auto flipped when QApplication::layoutDirection() changes
        //and it shouldn't, the internal one no and it should, so i must manually invert both
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->extLayout->setDirection(QBoxLayout::LeftToRight);
        } else {
            d->extLayout->setDirection(QBoxLayout::RightToLeft);
        }
        d->extLayout->setContentsMargins(1, 0, 0, 0);
        d->panelHeightHandle->setCursor(Qt::SizeHorCursor);

        d->ruler->setAvailableLength(screenGeom.height());
        break;
    case Plasma::RightEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->extLayout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->extLayout->setDirection(QBoxLayout::LeftToRight);
        }
        d->extLayout->setContentsMargins(1, 0, 0, 0);
        d->panelHeightHandle->setCursor(Qt::SizeHorCursor);

        d->ruler->setAvailableLength(screenGeom.height());
        break;
    case Plasma::TopEdge:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->layout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->layout->setDirection(QBoxLayout::LeftToRight);
        }
        d->extLayout->setDirection(QBoxLayout::BottomToTop);
        d->extLayout->setContentsMargins(0, 0, 0, 1);
        d->panelHeightHandle->setCursor(Qt::SizeVerCursor);

        d->ruler->setAvailableLength(screenGeom.width());
        break;
    case Plasma::BottomEdge:
    default:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->layout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->layout->setDirection(QBoxLayout::LeftToRight);
        }
        d->extLayout->setDirection(QBoxLayout::TopToBottom);
        d->extLayout->setContentsMargins(0, 1, 0, 0);
        d->panelHeightHandle->setCursor(Qt::SizeVerCursor);

        d->ruler->setAvailableLength(screenGeom.width());
        break;
    }

    d->alignLayout->setDirection(d->layout->direction());
    if (d->alignLayout->parentWidget()) {
        d->alignLayout->parentWidget()->setMaximumSize(d->alignLayout->sizeHint());
    }

    d->ruler->setMaximumSize(d->ruler->sizeHint());
}

Plasma::Location PanelController::location() const
{
    return d->location;
}

void PanelController::setOffset(int newOffset)
{
    if (newOffset != d->ruler->offset()) {
        d->ruler->setOffset(newOffset);
    }
}

int PanelController::offset()
{
    return d->ruler->offset();
}

void PanelController::setAlignment(const Qt::Alignment &newAlignment)
{
    if (newAlignment != d->ruler->alignment()) {
        if (newAlignment == Qt::AlignLeft) {
            d->leftAlignTool->setChecked(true);
        } else if (newAlignment == Qt::AlignCenter) {
            d->centerAlignTool->setChecked(true);
        } else if (newAlignment == Qt::AlignRight) {
            d->rightAlignTool->setChecked(true);
        }

        d->ruler->setAlignment(newAlignment);
    }
}

int PanelController::alignment()
{
    return d->ruler->alignment();
}

void PanelController::hideController()
{
    hide();
}

void PanelController::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );
    QColor backColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    backColor.setAlphaF(0.75);
    painter.fillRect(event->rect(), backColor);
}

void PanelController::mousePressEvent(QMouseEvent *event)
{
    if (d->panelHeightHandle->geometry().contains(event->pos()) ) {
        d->startDragPos = event->pos();
        d->dragging = Private::ResizeHandleElement;
    } else if (QRect(QPoint(0, 0), size()).contains(event->pos()) && !d->ruler->geometry().contains(event->pos()) ) {
        d->dragging = Private::PanelControllerElement;
        setCursor(Qt::SizeAllCursor);
    }

    QWidget::mousePressEvent(event);
}

void PanelController::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    //FIXME: for now resizes here instead of on mouse move, for a serious performance problem, maybe in Qt
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());
    if (d->dragging == Private::ResizeHandleElement) {
        switch (location()) {
        case Plasma::LeftEdge:
            d->resizePanelHeight(geometry().left() - screenGeom.left());
            break;
        case Plasma::RightEdge:
            d->resizePanelHeight(screenGeom.right() - geometry().right());
            break;
        case Plasma::TopEdge:
            d->resizePanelHeight(geometry().top() - screenGeom.top());
            break;
        case Plasma::BottomEdge:
        default:
            d->resizePanelHeight(screenGeom.bottom() - geometry().bottom());
            break;
        }
    }

    //resets properties saved during the drag
    d->startDragPos = QPoint(0, 0);
    d->dragging = Private::NoElement;
    setCursor(Qt::ArrowCursor);
}

void PanelController::mouseMoveEvent(QMouseEvent *event)
{
    if (d->dragging == Private::NoElement || !d->containment) {
        return;
    }

    QDesktopWidget *desktop = QApplication::desktop();
    QRect screenGeom = desktop->screenGeometry(d->containment->screen());

    if (d->dragging == Private::PanelControllerElement) {
        //only move when the mouse cursor is out of the controller to avoid an endless reposition cycle
        if (geometry().contains(event->globalPos())) {
            return;
        }

        if (!screenGeom.contains(event->globalPos())) {
            //move panel to new screen if dragged there
            int targetScreen = desktop->screenNumber(event->globalPos());
            kDebug() << "Moving panel from screen" << d->containment->screen() << "to screen" << targetScreen;
            d->containment->setScreen(targetScreen);
            return;
        }

        //create a dead zone so you can go across the middle without having it hop to one side
        float dzFactor = 0.35;
        QPoint offset = QPoint::QPoint(screenGeom.width()*dzFactor,screenGeom.height()*dzFactor);
        QRect deadzone = QRect::QRect(screenGeom.topLeft()+offset, screenGeom.bottomRight()-offset);
        if (deadzone.contains(event->globalPos())) {
            //kDebug() << "In the deadzone:" << deadzone;
            return;
        }

        Plasma::FormFactor oldFormFactor = d->containment->formFactor();
        Plasma::FormFactor newFormFactor = d->containment->formFactor();
        const Plasma::Location oldLocation = d->containment->location();
        Plasma::Location newLocation = d->containment->location();
        float screenAspect = float(screenGeom.height()/screenGeom.width());

        /* Use diagonal lines so we get predictable behavior when moving the panel
         * y=topleft.y+(x-topleft.x)*aspectratio   topright < bottomleft
         * y=bottomleft.y-(x-topleft.x)*aspectratio   topleft < bottomright
         */
        if (event->globalY() < screenGeom.y()+(event->globalX()-screenGeom.x())*screenAspect) {
            if (event->globalY() < screenGeom.bottomLeft().y()-(event->globalX()-screenGeom.x())*screenAspect) {
                if (d->containment->location() == Plasma::TopEdge) {
                    return;
                } else {
                    newFormFactor = Plasma::Horizontal;
                    newLocation = Plasma::TopEdge;
                }
            } else if (d->containment->location() == Plasma::RightEdge) {
                    return;
            } else {
                newFormFactor = Plasma::Vertical;
                newLocation = Plasma::RightEdge;
            }
        } else {
            if (event->globalY() < screenGeom.bottomLeft().y()-(event->globalX()-screenGeom.x())*screenAspect) {
                if (d->containment->location() == Plasma::LeftEdge) {
                    return;
                } else {
                    newFormFactor = Plasma::Vertical;
                    newLocation = Plasma::LeftEdge;
                }
            } else if(d->containment->location() == Plasma::BottomEdge) {
                    return;
            } else {
                newFormFactor = Plasma::Horizontal;
                newLocation = Plasma::BottomEdge;
            }
        }


        //If the orientation changed swap width and height
        if (oldLocation != newLocation) {
            int panelHeight;

            if (oldFormFactor == Plasma::Vertical) {
                panelHeight = d->containment->size().width();
            } else {
                panelHeight = d->containment->size().height();
            }

            //FIXME: to achieve a reliable resize it seems that it is necessary to resize, set min and max sizes and resize again, a little weird

            emit beginLocationChange();
            d->containment->setFormFactor(newFormFactor);
            d->containment->setLocation(newLocation);

            if (newFormFactor == Plasma::Vertical) {
                d->containment->resize(panelHeight, d->containment->preferredSize().height());
                d->containment->setMaximumSize(panelHeight, qMin(screenGeom.height(), d->ruler->maxLength()));
                d->containment->setMinimumSize(panelHeight, qMin(screenGeom.height(), d->ruler->minLength()));
                d->containment->resize(panelHeight, d->containment->preferredSize().height());
            } else {
                d->containment->resize(d->containment->preferredSize().width(), panelHeight);
                d->containment->setMaximumSize(qMin(screenGeom.width(), d->ruler->maxLength()), panelHeight);
                d->containment->setMinimumSize(qMin(screenGeom.width(), d->ruler->minLength()), panelHeight);
                d->containment->resize(d->containment->preferredSize().width(), panelHeight);
            }

            emit commitLocationChange();
        }

        return;
    } 

    //Resize handle moved
    switch (location()) {
    case Plasma::LeftEdge:
        if (mapToGlobal(event->pos()).x() -
            d->startDragPos.x() - d->minimumHeight >
            screenGeom.left()) {
            move(mapToGlobal(event->pos()).x() - d->startDragPos.x(), pos().y());
            //FIXME: Panel resize deferred, should be here
        }
        break;
    case Plasma::RightEdge:
        if (mapToGlobal(event->pos()).x() -
            d->startDragPos.x() + width() + d->minimumHeight <
            screenGeom.right()) {
            move(mapToGlobal(event->pos()).x() - d->startDragPos.x(), pos().y());
        }
        break;
    case Plasma::TopEdge:
        if (mapToGlobal(event->pos()).y() -
            d->startDragPos.y() - d->minimumHeight >
            screenGeom.top()) {
            move(pos().x(), mapToGlobal(event->pos()).y() - d->startDragPos.y());
        }
        break;
    case Plasma::BottomEdge:
    default:
        if (mapToGlobal(event->pos()).y() -
            d->startDragPos.y() + height() + d->minimumHeight <
            screenGeom.bottom()) {
            move(pos().x(), mapToGlobal(event->pos()).y() - d->startDragPos.y());
        }
        break;
    }
}

#include "panelcontroller.moc"
