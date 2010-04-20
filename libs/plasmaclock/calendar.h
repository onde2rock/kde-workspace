/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
 *   Copyright 2009 John Layt <john@layt.net>
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

#ifndef PLASMA_CALENDAR_H
#define PLASMA_CALENDAR_H

#include <QtGui/QGraphicsWidget>

#include "plasmaclock_export.h"

#include "calendartable.h"

class KCalendarSystem;
class KConfigDialog;
class KConfigGroup;

namespace Plasma
{

class CalendarTable;
class CalendarPrivate;
class DataEngine;

class PLASMACLOCK_EXPORT Calendar : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit Calendar(QGraphicsWidget *parent = 0);
    explicit Calendar(const QDate &, QGraphicsWidget *parent = 0);
    explicit Calendar(CalendarTable *calendarTable, QGraphicsWidget *parent = 0);
    ~Calendar();

    CalendarTable *calendarTable() const;

    void setCalendar(const QString &newCalendarType = "locale");
    void setCalendar(const KCalendarSystem *calendar);
    const KCalendarSystem *calendar () const;

    void setDate(const QDate &date);
    const QDate& date() const;

    void setDataEngine(Plasma::DataEngine *dataEngine);
    const Plasma::DataEngine *dataEngine() const;

    void setDisplayHolidays(bool showHolidays);
    bool displayHolidays();

    void setHolidaysRegion(const QString &region);
    QString holidaysRegion() const;

    bool dateHasDetails(const QDate &date) const;
    QString dateDetails(const QDate &date) const;

    void applyConfiguration(KConfigGroup cg);
    void writeConfiguration(KConfigGroup cg);
    void createConfigurationInterface(KConfigDialog *parent);
    void applyConfigurationInterface();
    void configAccepted(KConfigGroup cg);

Q_SIGNALS:
    void dateChanged(const QDate &newDate, const QDate &oldDate);
    void dateChanged(const QDate &newDate);
    void dateHovered(const QDate &date);

private Q_SLOTS:
    void prevMonth();
    void nextMonth();
    void prevYear();
    void nextYear();
    void dateUpdated(const QDate &newDate);
    void showTip(const QDate &date);
    void goToToday();
    void goToWeek(int week);
    void manualDateChange();
    void monthsPopup();
    void monthTriggered();
    void showYearSpinBox();
    void hideYearSpinBox();

private:
    void init(CalendarTable *calendarTable);
    void refreshWidgets();
    CalendarPrivate* const d;
};

}

#endif
