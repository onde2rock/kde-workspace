#ifndef __NEXTCLIENT_H
#define __NEXTCLIENT_H

#include <qvariant.h>
#include <qbitmap.h>
#include <kpixmap.h>
#include <qlayout.h>
#include "../../client.h"
#include "../../kwinbutton.h"
class QLabel;
class QSpacerItem;

namespace KStep {

using namespace KWinInternal;

class NextButton : public KWinButton
{
public:
    NextButton(Client *parent=0, const char *name=0,
               const unsigned char *bitmap=NULL, int bw=0, int bh=0,
               const QString& tip=NULL);
    void setBitmap(const unsigned char *bitmap, int bw, int bh);
    void reset();
    ButtonState lastButton() { return last_button; }

protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    virtual void drawButton(QPainter *p);
    void drawButtonLabel(QPainter *){;}

    KPixmap aBackground, iBackground;
    QBitmap* deco;
    Client *client;
    ButtonState last_button;
};

class NextClient : public Client
{
    Q_OBJECT
public:
    NextClient( Workspace *ws, WId w, QWidget *parent=0, const char *name=0 );
    ~NextClient() {;}
protected:
    void resizeEvent( QResizeEvent* );
    void paintEvent( QPaintEvent* );
 
    void mouseDoubleClickEvent( QMouseEvent * );
    void init();
    void captionChange( const QString& name );
    void stickyChange(bool on);
    void activeChange(bool);

    MousePosition mousePosition(const QPoint &) const;

protected slots:
    void slotReset();
    void menuButtonPressed();
    void maximizeButtonClicked();

private:
    void initializeButtonsAndTitlebar(QBoxLayout* titleLayout);
    void addButtons(QBoxLayout* titleLayout, const QString& buttons);

    QSpacerItem* titlebar;

    // Helpful constants for buttons in array
    enum { CLOSE_IDX    = 0,
           HELP_IDX     = 1,
           ICONIFY_IDX  = 2,
           MAXIMIZE_IDX = 3,
           MENU_IDX     = 4,
           STICKY_IDX   = 5,
           MAX_NUM_BUTTONS = STICKY_IDX + 1 };

    // WARNING: button[i] may be null for any given i.  Make sure you
    // always check for null before doing button[i]->foo().
    NextButton* button[MAX_NUM_BUTTONS];
};                      

};

#endif
