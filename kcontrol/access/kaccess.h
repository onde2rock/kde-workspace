#ifndef __K_ACCESS_H__
#define __K_ACCESS_H__


#include <QWidget>
#include <QColor>
//Added by qt3to4:
#include <QLabel>
#include <QPaintEvent>


#include <kuniqueapplication.h>
#include <kurl.h>
#include <kwinmodule.h>

#include <phonon/simpleplayer.h>

#include <X11/Xlib.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit

class KDialog;
class QLabel;
class KComboBox;

class KAccessApp : public KUniqueApplication
{
  Q_OBJECT

public:

  KAccessApp(bool allowStyles=true, bool GUIenabled=true);

  bool x11EventFilter(XEvent *event);

  int newInstance();


protected:

  void readSettings();

  void xkbStateNotify();
  void xkbBellNotify(XkbBellNotifyEvent *event);
  void xkbControlsNotify(XkbControlsNotifyEvent *event);


private Q_SLOTS:

  void activeWindowChanged(WId wid);
  void slotArtsBellTimeout();
  void notifyChanges();
  void applyChanges();
  void yesClicked();
  void noClicked();
  void dialogClosed();


private:
   void  createDialogContents();
   void  initMasks();

  int xkb_opcode;
  unsigned int features;
  unsigned int requestedFeatures;

  Phonon::SimplePlayer _player;
  bool    _systemBell, _artsBell, _visibleBell, _visibleBellInvert;
  bool    _artsBellBlocked;
  KUrl    _artsBellFile;
  QColor  _visibleBellColor;
  int     _visibleBellPause;

  bool    _gestures, _gestureConfirmation;
  bool    _kNotifyModifiers, _kNotifyAccessX;

  QWidget *overlay;

  QTimer *artsBellTimer;

  KWinModule wm;

  WId _activeWindow;

  KDialog *dialog;
  QLabel *featuresLabel;
  KComboBox *showModeCombobox;

  int keys[8];
  int state;
};


class VisualBell : public QWidget
{
  Q_OBJECT

public:

  VisualBell(int pause)
    : QWidget(( QWidget* )0, Qt::WX11BypassWM), _pause(pause)
    {}


protected:

  void paintEvent(QPaintEvent *);


private:

  int _pause;

};




#endif
