    /*

    Shutdown dialog. Class KDMShutdown
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000, 2001 Oswald Buddenhagen <ossi@kde.org>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#ifndef KDMSHUTDOWN_H
#define KDMSHUTDOWN_H

#include <qglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <kpassdlg.h>
#include "kfdialog.h"

#include <sys/param.h>	// for BSD
#include <stdlib.h>

class KDMShutdown : public FDialog {
    Q_OBJECT

public:
    KDMShutdown( QWidget* _parent=0);

private slots:
    void bye_bye();
    void target_changed(int);
    void when_changed(int);
    void timerDone();

private:
    QLabel		*label;
    QLabel 		*plb;
    QButtonGroup	*howGroup, *whenGroup;
    QPushButton		*okButton;
    QPushButton		*cancelButton;
    KPasswordEdit	*pswdEdit;
    QRadioButton	*restart_rb, *force_rb, *try_rb;
    QTimer		*timer;
    bool		needRoot;
#if defined(__linux__) && (__i386__)
    int			liloTarget;
#endif

    void set_min( QWidget* w) { w->setMinimumSize( w->sizeHint()); }
    void set_fixed( QWidget* w) { w->setFixedSize( w->sizeHint()); }

};

#endif /* KDMSHUTDOWN_H */
