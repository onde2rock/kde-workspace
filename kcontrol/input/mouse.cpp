/*
 * mouse.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 * SC/DC/AutoSelect/ChangeCursor:
 * Copyright (c) 2000 David Faure <faure@kde.org>
 *
 * Double click interval, drag time & dist
 * Copyright (c) 2000 Bernd Gehrmann
 *
 * Large cursor support
 * Visual activation TODO: speed
 * Copyright (c) 2000 Rik Hemsley <rik@kde.org>
 *
 * White cursor support
 * TODO: give user the option to choose a certain cursor font
 * -> Theming
 *
 * General/Advanced tabs
 * Copyright (c) 2000 Brad Hughes <bhughes@trolltech.com>
 *
 * redesign for KDE 2.2
 * Copyright (c) 2001 Ralf Nolden <nolden@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qlayout.h>
#include <qcheckbox.h>
#undef Below
#undef Above
#include <qslider.h>
#include <qwhatsthis.h>
#include <qtabwidget.h>

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "mouse.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <kipc.h>

#undef Below

MouseConfig::MouseConfig (QWidget * parent, const char *name)
  : KCModule(parent, name)
{
    QString wtstr;

    QBoxLayout *top = new QVBoxLayout(this, 0, KDialog::spacingHint());

    tabwidget = new QTabWidget(this);
    top->addWidget(tabwidget);

    tab1 = new KMouseDlg(this);
    QButtonGroup *group = new QButtonGroup( tab1 );
    group->setExclusive( true );
    group->hide();
    group->insert( tab1->singleClick );
    group->insert( tab1->doubleClick );

    tabwidget->addTab(tab1, i18n("&General"));

    connect(tab1->handedBox, SIGNAL(clicked(int)), this, SLOT(changed()));
    connect(tab1->handedBox, SIGNAL(clicked(int)), this, SLOT(slotHandedChanged(int)));

    wtstr = i18n("If you are left-handed, you may prefer to swap the"
         " functions of the left and right buttons on your pointing device"
         " by choosing the 'left-handed' option. If your pointing device"
         " has more than two buttons, only those that function as the"
         " left and right buttons are affected. For example, if you have"
         " a three-button mouse, the middle button is unaffected.");
    QWhatsThis::add( tab1->handedBox, wtstr );

    connect(tab1->doubleClick, SIGNAL(clicked()), SLOT(changed()));

    wtstr = i18n("The default behavior in KDE is to select and activate"
         " icons with a single click of the left button on your pointing"
         " device. This behavior is consistent with what you would expect"
         " when you click links in most web browsers. If you would prefer"
         " to select with a single click, and activate with a double click,"
         " check this option.");
    QWhatsThis::add( tab1->doubleClick, wtstr );

    wtstr = i18n("Activates and opens a file or folder with a single click.");
    QWhatsThis::add( tab1->singleClick, wtstr );


    connect(tab1->cbAutoSelect, SIGNAL(clicked()), this, SLOT(changed()));

    wtstr = i18n("If you check this option, pausing the mouse pointer"
         " over an icon on the screen will automatically select that icon."
         " This may be useful when single clicks activate icons, and you"
         " want only to select the icon without activating it.");
    QWhatsThis::add( tab1->cbAutoSelect, wtstr );

//    slAutoSelect = new QSlider(0, 2000, 10, 0, QSlider::Horizontal, tab1);
    tab1->slAutoSelect->setSteps( 125, 125 );
    tab1->slAutoSelect->setTickmarks( QSlider::Below );
    tab1->slAutoSelect->setTickInterval( 250 );
    tab1->slAutoSelect->setTracking( true );

    wtstr = i18n("If you have checked the option to automatically select"
         " icons, this slider allows you to select how long the mouse pointer"
         " must be paused over the icon before it is selected.");
    QWhatsThis::add( tab1->slAutoSelect, wtstr );

    wtstr = i18n("Show feedback when clicking an icon");
    QWhatsThis::add( tab1->cbVisualActivate, wtstr );

    connect(tab1->slAutoSelect, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(tab1->cbVisualActivate, SIGNAL(clicked()), this, SLOT(changed()));

    connect(tab1->cb_pointershape, SIGNAL(clicked()), this, SLOT(changed()));

    connect(tab1->singleClick, SIGNAL(clicked()), this, SLOT(changed()));
    connect(tab1->singleClick, SIGNAL(clicked()), this, SLOT(slotClick()));

    connect( tab1->doubleClick, SIGNAL( clicked() ), this, SLOT( slotClick() ) );
    connect( tab1->cbAutoSelect, SIGNAL( clicked() ), this, SLOT( slotClick() ) );

    // Cursor theme tab
    themetab = new ThemePage(this);
    connect(themetab, SIGNAL(changed(bool)), SLOT(changed()));
    tabwidget->addTab(themetab, i18n("&Cursor Theme"));

    // Advanced tab
    tab2 = new QWidget(0, "Advanced Tab");
    tabwidget->addTab(tab2, i18n("Advanced"));

    QBoxLayout *lay = new QVBoxLayout(tab2, KDialog::marginHint(),
              KDialog::spacingHint());

    accel = new KDoubleNumInput(2, tab2);
    accel->setLabel(i18n("Pointer acceleration:"));
    accel->setPrecision(1);
    accel->setRange(1,20);
    accel->setSuffix("x");
    lay->addWidget(accel);
    connect(accel, SIGNAL(valueChanged(double)), this, SLOT(changed()));

    wtstr = i18n("This option allows you to change the relationship"
         " between the distance that the mouse pointer moves on the"
         " screen and the relative movement of the physical device"
         " itself (which may be a mouse, trackball, or some other"
         " pointing device.)<p>"
         " A high value for the acceleration will lead to large"
         " movements of the mouse pointer on the screen even when"
         " you only make a small movement with the physical device."
         " Selecting very high values may result in the mouse pointer"
         " flying across the screen, making it hard to control!");
    QWhatsThis::add( accel, wtstr );

    thresh = new KIntNumInput(accel, 20, tab2);
    thresh->setLabel(i18n("Pointer threshold:"));
    thresh->setRange(0,20,2);
    thresh->setSuffix(i18n(" pixels"));
    thresh->setSteps(1,1);
    lay->addWidget(thresh);
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("The threshold is the smallest distance that the"
         " mouse pointer must move on the screen before acceleration"
         " has any effect. If the movement is smaller than the threshold,"
         " the mouse pointer moves as if the acceleration was set to 1X.<p>"
         " Thus, when you make small movements with the physical device,"
         " there is no acceleration at all, giving you a greater degree"
         " of control over the mouse pointer. With larger movements of"
         " the physical device, you can move the mouse pointer"
         " rapidly to different areas on the screen.");
    QWhatsThis::add( thresh, wtstr );

    // It would be nice if the user had a test field.
    // Selecting such values in milliseconds is not intuitive
    doubleClickInterval = new KIntNumInput(thresh, 2000, tab2);
    doubleClickInterval->setLabel(i18n("Double click interval:"));
    doubleClickInterval->setRange(0, 2000, 100);
    doubleClickInterval->setSuffix(i18n(" msec"));
    doubleClickInterval->setSteps(100, 100);
    lay->addWidget(doubleClickInterval);
    connect(doubleClickInterval, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("The double click interval is the maximal time"
         " (in milliseconds) between two mouse clicks which"
         " turns them into a double click. If the second"
         " click happens later than this time interval after"
         " the first click, they are recognized as two"
         " separate clicks.");
    QWhatsThis::add( doubleClickInterval, wtstr );

    lay->addSpacing(15);

    dragStartTime = new KIntNumInput(doubleClickInterval, 2000, tab2);
    dragStartTime->setLabel(i18n("Drag start time:"));
    dragStartTime->setRange(0, 2000, 100);
    dragStartTime->setSuffix(i18n(" msec"));
    dragStartTime->setSteps(100, 100);
    lay->addWidget(dragStartTime);
    connect(dragStartTime, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("If you click with the mouse (e.g. in a multi-line"
         " editor) and begin to move the mouse within the"
         " drag start time, a drag operation will be initiated.");
    QWhatsThis::add( dragStartTime, wtstr );

    dragStartDist = new KIntNumInput(dragStartTime, 20, tab2);
    dragStartDist->setLabel(i18n("Drag start distance:"));
    dragStartDist->setRange(1, 20, 2);
    dragStartDist->setSuffix(i18n(" pixels"));
    dragStartDist->setSteps(1,1);
    lay->addWidget(dragStartDist);
    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("If you click with the mouse and begin to move the"
         " mouse at least the drag start distance, a drag"
         " operation will be initiated.");
    QWhatsThis::add( dragStartDist, wtstr);

    wheelScrollLines = new KIntNumInput(dragStartDist, 3, tab2);
    wheelScrollLines->setLabel(i18n("Mouse wheel scrolls by:"));
    wheelScrollLines->setRange(1, 12, 2);
    wheelScrollLines->setSuffix(i18n(" lines"));
    wheelScrollLines->setSteps(1,1);
    lay->addWidget(wheelScrollLines);
    connect(wheelScrollLines, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("If you use the wheel of a mouse, this value determines the number of lines to scroll for each wheel movement. Note that if this number exceeds the number of visible lines, it will be ignored and the wheel movement will be handled as a page up/down movement.");
    QWhatsThis::add( wheelScrollLines, wtstr);
    lay->addStretch();

{
  QWidget *mouse = new QWidget(this, "Mouse Navigation");
  tabwidget->addTab(mouse, i18n("Mouse Navigation"));


  QBoxLayout *vbox = new QVBoxLayout(mouse, KDialog::marginHint(),
    KDialog::spacingHint());

  QGroupBox *grp = new QGroupBox(i18n("Mouse Navigation"), mouse);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  QVBoxLayout *vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  mouseKeys = new QCheckBox(i18n("&Move pointer with keyboard (using the num pad)"), grp);
  vvbox->addWidget(mouseKeys);

  QBoxLayout *hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  mk_delay = new KIntNumInput(grp);
  mk_delay->setLabel(i18n("&Acceleration delay:"), AlignVCenter);
  mk_delay->setSuffix(i18n(" msec"));
  mk_delay->setRange(1, 1000, 50);
  hbox->addWidget(mk_delay);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  mk_interval = new KIntNumInput(mk_delay, 0, grp);
  mk_interval->setLabel(i18n("R&epeat interval:"), AlignVCenter);
  mk_interval->setSuffix(i18n(" msec"));
  mk_interval->setRange(1, 1000, 10);
  hbox->addWidget(mk_interval);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  mk_time_to_max = new KIntNumInput(mk_interval, 0, grp);
  mk_time_to_max->setLabel(i18n("Acceleration &time:"), AlignVCenter);
  mk_time_to_max->setRange(1, 5000, 250);
  hbox->addWidget(mk_time_to_max);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  mk_max_speed = new KIntNumInput(mk_time_to_max, 0, grp);
  mk_max_speed->setLabel(i18n("Ma&ximum speed:"), AlignVCenter);
  mk_max_speed->setRange(1, 1000, 10);
  hbox->addWidget(mk_max_speed);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  mk_curve = new KIntNumInput(mk_max_speed, 0, grp);
  mk_curve->setLabel(i18n("Acceleration &profile:"), AlignVCenter);
  mk_curve->setRange(-1000, 1000, 100);
  hbox->addWidget(mk_curve);

  connect(mouseKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(mouseKeys, SIGNAL(clicked()), this, SLOT(changed()));
  connect(mk_delay, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_interval, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_time_to_max, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_max_speed, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_curve, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  vbox->addStretch();
}


    config = new KConfig("kcminputrc");
    settings = new MouseSettings;
    load();
}


void MouseConfig::checkAccess()
{
  mk_delay->setEnabled(mouseKeys->isChecked());
  mk_interval->setEnabled(mouseKeys->isChecked());
  mk_time_to_max->setEnabled(mouseKeys->isChecked());
  mk_max_speed->setEnabled(mouseKeys->isChecked());
  mk_curve->setEnabled(mouseKeys->isChecked());
}


MouseConfig::~MouseConfig()
{
    delete config;
    delete settings;
}

double MouseConfig::getAccel()
{
  return accel->value();
}

void MouseConfig::setAccel(double val)
{
  accel->setValue(val);
}

int MouseConfig::getThreshold()
{
  return thresh->value();
}

void MouseConfig::setThreshold(int val)
{
  thresh->setValue(val);
}


int MouseConfig::getHandedness()
{
  if (tab1->rightHanded->isChecked())
    return RIGHT_HANDED;
  else
    return LEFT_HANDED;
}

void MouseConfig::setHandedness(int val)
{
  tab1->rightHanded->setChecked(false);
  tab1->leftHanded->setChecked(false);
  if (val == RIGHT_HANDED){
    tab1->rightHanded->setChecked(true);
    tab1->mousePix->setPixmap(locate("data", "kcminput/pics/mouse_rh.png"));
  }
  else{
    tab1->leftHanded->setChecked(true);
    tab1->mousePix->setPixmap(locate("data", "kcminput/pics/mouse_lh.png"));
  }
}

void MouseConfig::load()
{
  settings->load(config);

  tab1->rightHanded->setEnabled(settings->handedEnabled);
  tab1->leftHanded->setEnabled(settings->handedEnabled);

  setAccel(settings->accelRate);
  setThreshold(settings->thresholdMove);
  setHandedness(settings->handed);

  doubleClickInterval->setValue(settings->doubleClickInterval);
  dragStartTime->setValue(settings->dragStartTime);
  dragStartDist->setValue(settings->dragStartDist);
  wheelScrollLines->setValue(settings->wheelScrollLines);

  tab1->singleClick->setChecked( settings->singleClick );
  tab1->doubleClick->setChecked(!settings->singleClick);
  tab1->cb_pointershape->setChecked(settings->changeCursor);
  tab1->cbAutoSelect->setChecked( settings->autoSelectDelay >= 0 );
  if ( settings->autoSelectDelay < 0 )
     tab1->slAutoSelect->setValue( 0 );
  else
     tab1->slAutoSelect->setValue( settings->autoSelectDelay );
  tab1->cbVisualActivate->setChecked( settings->visualActivate );
  slotClick();


  KConfig ac("kaccessrc", true);

  ac.setGroup("Mouse");
  mouseKeys->setChecked(ac.readBoolEntry("MouseKeys", false));
  mk_delay->setValue(ac.readNumEntry("MKDelay", 160));
  mk_interval->setValue(ac.readNumEntry("MKInterval", 5));
  mk_time_to_max->setValue(ac.readNumEntry("MKTimeToMax", 1000));
  mk_max_speed->setValue(ac.readNumEntry("MKMaxSpeed", 500));
  mk_curve->setValue(ac.readNumEntry("MKCurve", 0));

  themetab->load();

  checkAccess();
  changed();
}

void MouseConfig::save()
{
  settings->accelRate = getAccel();
  settings->thresholdMove = getThreshold();
  settings->handed = getHandedness();

  settings->doubleClickInterval = doubleClickInterval->value();
  settings->dragStartTime = dragStartTime->value();
  settings->dragStartDist = dragStartDist->value();
  settings->wheelScrollLines = wheelScrollLines->value();
  settings->singleClick = !tab1->doubleClick->isChecked();
  settings->autoSelectDelay = tab1->cbAutoSelect->isChecked()? tab1->slAutoSelect->value():-1;
  settings->visualActivate = tab1->cbVisualActivate->isChecked();
//  settings->changeCursor = tab1->singleClick->isChecked();
  settings->changeCursor = tab1->cb_pointershape->isChecked();

  settings->apply();
  settings->save(config);

  KConfig ac("kaccessrc", false);

  ac.setGroup("Mouse");

  ac.writeEntry("MouseKeys", mouseKeys->isChecked());
  ac.writeEntry("MKDelay", mk_delay->value());
  ac.writeEntry("MKInterval", mk_interval->value());
  ac.writeEntry("MKTimeToMax", mk_time_to_max->value());
  ac.writeEntry("MKMaxSpeed", mk_max_speed->value());
  ac.writeEntry("MKCurve", mk_curve->value());

  config->sync();

  themetab->save();

  // restart kaccess
  kapp->startServiceByDesktopName("kaccess");

  KCModule::changed(false);

}

void MouseConfig::defaults()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(RIGHT_HANDED);
    doubleClickInterval->setValue(400);
    dragStartTime->setValue(500);
    dragStartDist->setValue(4);
    wheelScrollLines->setValue(3);
    tab1->doubleClick->setChecked( !KDE_DEFAULT_SINGLECLICK );
    tab1->cbAutoSelect->setChecked( KDE_DEFAULT_AUTOSELECTDELAY != -1 );
    tab1->slAutoSelect->setValue( KDE_DEFAULT_AUTOSELECTDELAY == -1 ? 50 : KDE_DEFAULT_AUTOSELECTDELAY );
    tab1->singleClick->setChecked( KDE_DEFAULT_SINGLECLICK );
    tab1->cbVisualActivate->setChecked( KDE_DEFAULT_VISUAL_ACTIVATE );
    tab1->cb_pointershape->setChecked(KDE_DEFAULT_CHANGECURSOR);
    slotClick();

  mouseKeys->setChecked(false);
  mk_delay->setValue(160);
  mk_interval->setValue(5);
  mk_time_to_max->setValue(1000);
  mk_max_speed->setValue(500);
  mk_curve->setValue(0);

  checkAccess();

  changed();
}


QString MouseConfig::quickHelp() const
{
  return i18n("<h1>Mouse</h1> This module allows you to choose various"
     " options for the way in which your pointing device works. Your"
     " pointing device may be a mouse, trackball, or some other hardware"
     " that performs a similar function.");
}


void MouseConfig::slotClick()
{
  // Autoselect has a meaning only in single-click mode
  tab1->cbAutoSelect->setEnabled(!tab1->doubleClick->isChecked() || tab1->singleClick->isChecked());
  // Delay has a meaning only for autoselect
  bool bDelay = tab1->cbAutoSelect->isChecked() && ! tab1->doubleClick->isChecked();
   tab1->slAutoSelect->setEnabled( bDelay );
   tab1->lDelay->setEnabled( bDelay );
   tab1->lb_short->setEnabled( bDelay );
   tab1->lb_long->setEnabled( bDelay );

}

void MouseConfig::changed()
{
  emit KCModule::changed(true);
}

/** No descriptions */
void MouseConfig::slotHandedChanged(int val){
  if(val==0)
    tab1->mousePix->setPixmap(locate("data", "kcminput/pics/mouse_rh.png"));
  else
    tab1->mousePix->setPixmap(locate("data", "kcminput/pics/mouse_lh.png"));
  settings->m_handedNeedsApply = true;
}

void MouseSettings::load(KConfig *config)
{
  int accel_num, accel_den, threshold;
  double accel;
  XGetPointerControl( kapp->getDisplay(),
              &accel_num, &accel_den, &threshold );
  accel = float(accel_num) / float(accel_den);

  // get settings from X server
  int h = RIGHT_HANDED;
  unsigned char map[5];
  num_buttons = XGetPointerMapping(kapp->getDisplay(), map, 5);

  handedEnabled = true;

  // ## keep this in sync with KGlobalSettings::mouseSettings
  switch (num_buttons)
    {
    case 1:
      /* disable button remapping */
      handedEnabled = false;
      break;
    case 2:
      if ( (int)map[0] == 1 && (int)map[1] == 2 )
        h = RIGHT_HANDED;
      else if ( (int)map[0] == 2 && (int)map[1] == 1 )
        h = LEFT_HANDED;
      else
        /* custom button setup: disable button remapping */
        handedEnabled = false;
      break;
    case 3:
    case 5:
      middle_button = (int)map[1];
      if ( (int)map[0] == 1 && (int)map[2] == 3 )
    h = RIGHT_HANDED;
      else if ( (int)map[0] == 3 && (int)map[2] == 1 )
    h = LEFT_HANDED;
      else
    {
      /* custom button setup: disable button remapping */
      handedEnabled = false;
    }
      break;
    default:
      /* custom setup with > 3 buttons: disable button remapping */
      handedEnabled = false;
      break;
    }

  config->setGroup("Mouse");
  double a = config->readDoubleNumEntry("Acceleration",-1);
  if (a == -1)
    accelRate = accel;
  else
    accelRate = a;

  int t = config->readNumEntry("Threshold",-1);
  if (t == -1)
    thresholdMove = threshold;
  else
    thresholdMove = t;

  QString key = config->readEntry("MouseButtonMapping");
  if (key == "RightHanded")
    handed = RIGHT_HANDED;
  else if (key == "LeftHanded")
    handed = LEFT_HANDED;
  else if (key == NULL)
    handed = h;
  m_handedNeedsApply = (handed != h);

  // SC/DC/AutoSelect/ChangeCursor
  config->setGroup("KDE");
  doubleClickInterval = config->readNumEntry("DoubleClickInterval", 400);
  dragStartTime = config->readNumEntry("StartDragTime", 500);
  dragStartDist = config->readNumEntry("StartDragDist", 4);
  wheelScrollLines = config->readNumEntry("WheelScrollLines", 3);

  singleClick = config->readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
  autoSelectDelay = config->readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
  visualActivate = config->readBoolEntry("VisualActivate", KDE_DEFAULT_VISUAL_ACTIVATE);
  changeCursor = config->readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

void MouseSettings::apply()
{
  XChangePointerControl( kapp->getDisplay(),
                         true, true, int(qRound(accelRate*10)), 10, thresholdMove);

  unsigned char map[5];
  int remap=1;
  if (handedEnabled && m_handedNeedsApply) {
      switch (num_buttons) {
      case 1:
          map[0] = (unsigned char) 1;
          break;
      case 2:
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) 3;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) 1;
          }
          break;
      case 3:
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 3;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 1;
          }
          break;
      case 5:
          // Intellimouse case, where buttons 1-3 are left, middle, and
          // right, and 4-5 are up/down
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) 2;
              map[2] = (unsigned char) 3;
              map[3] = (unsigned char) 4;
              map[4] = (unsigned char) 5;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) 2;
              map[2] = (unsigned char) 1;
              map[3] = (unsigned char) 4;
              map[4] = (unsigned char) 5;
          }
          break;
      default: {
              //catch-all for mice with four or more than five buttons
              //Without this, XSetPointerMapping is called with a undefined value
              //for map
              remap=0;  //don't remap
          }
          break;
      }
      int retval;
      if (remap)
          while ((retval=XSetPointerMapping(kapp->getDisplay(), map,
                                            num_buttons)) == MappingBusy)
              /* keep trying until the pointer is free */
          { };
      m_handedNeedsApply = false;
  }

}

void MouseSettings::save(KConfig *config)
{
  config->setGroup("Mouse");
  config->writeEntry("Acceleration",accelRate);
  config->writeEntry("Threshold",thresholdMove);
  if (handed == RIGHT_HANDED)
      config->writeEntry("MouseButtonMapping",QString("RightHanded"));
  else
      config->writeEntry("MouseButtonMapping",QString("LeftHanded"));

  config->setGroup("KDE");
  config->writeEntry("DoubleClickInterval", doubleClickInterval, true, true);
  config->writeEntry("StartDragTime", dragStartTime, true, true);
  config->writeEntry("StartDragDist", dragStartDist, true, true);
  config->writeEntry("WheelScrollLines", wheelScrollLines, true, true);
  config->writeEntry("SingleClick", singleClick, true, true);
  config->writeEntry("AutoSelectDelay", autoSelectDelay, true, true );
  config->writeEntry("VisualActivate", visualActivate, true, true);

  // the following need to be in the local config file so they are read
  // by kcminit (which only reads local config for speed up)
  config->writeEntry("ChangeCursor", changeCursor );
  config->sync();
  KIPC::sendMessageAll(KIPC::SettingsChanged, KApplication::SETTINGS_MOUSE);
}

#include "mouse.moc"
