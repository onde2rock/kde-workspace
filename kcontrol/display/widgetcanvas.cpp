//
// A special widget which draws a sample of KDE widgets
// It is used to preview color schemes
//
// Copyright (c)  Mark Donohoe 1998
//

#include <kcolordrag.h>
#include <kpixmapeffect.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <qbitmap.h>

#include "widgetcanvas.h"
#include "widgetcanvas.moc"
#include "stdclient_bitmaps.h"

static QPixmap* close_pix = 0;
static QPixmap* maximize_pix = 0;
static QPixmap* minimize_pix = 0;
static QPixmap* normalize_pix = 0;
static QPixmap* pinup_pix = 0;
static QPixmap* pindown_pix = 0;
static QPixmap* menu_pix = 0;

static QPixmap* dis_close_pix = 0;
static QPixmap* dis_maximize_pix = 0;
static QPixmap* dis_minimize_pix = 0;
static QPixmap* dis_normalize_pix = 0;
static QPixmap* dis_pinup_pix = 0;
static QPixmap* dis_pindown_pix = 0;
static QPixmap* dis_menu_pix = 0;


WidgetCanvas::WidgetCanvas( QWidget *parent, const char *name )
	: QWidget( parent, name  )
{
    setBackgroundMode( NoBackground );
    setAcceptDrops( true);
    setMinimumSize(200, 100);
}

void WidgetCanvas::paintEvent(QPaintEvent *)
{
    bitBlt( this, 0, 0, &smplw );
}

void WidgetCanvas::mousePressEvent( QMouseEvent *me )
{
    for ( int i = 0; i < MAX_HOTSPOTS; i++ )
	if ( hotspots[i].rect.contains( me->pos() ) ) {
	    emit widgetSelected( hotspots[i].number );
	    return;
	}
}

void WidgetCanvas::dropEvent( QDropEvent *e)
{
    QColor c;
    if (KColorDrag::decode( e, c)) {
	for ( int i = 0; i < MAX_HOTSPOTS; i++ )
	    if ( hotspots[i].rect.contains( e->pos() ) ) {
		emit colorDropped( hotspots[i].number, c);
		return;
	    }
    }
}


void WidgetCanvas::dragEnterEvent( QDragEnterEvent *e)
{
        e->accept( KColorDrag::canDecode( e));
}

void WidgetCanvas::paletteChange(const QPalette &)
{
	drawSampleWidgets();
}

void WidgetCanvas::resizeEvent(QResizeEvent *)
{
    drawSampleWidgets();
}

/*
 * This is necessary because otherwise the scrollbar in drawSampleWidgets()
 * doesn't show the first time.
 */
void WidgetCanvas::showEvent(QShowEvent *)
{
    drawSampleWidgets();
}

void WidgetCanvas::resetTitlebarPixmaps(const QColor &actMed,
                                        const QColor &disMed)
{
    if(close_pix) delete close_pix;
    if(maximize_pix) delete maximize_pix;
    if(minimize_pix) delete minimize_pix;
    if(normalize_pix) delete normalize_pix;
    if(pinup_pix) delete pinup_pix;
    if(pindown_pix) delete pindown_pix;
    if(menu_pix) delete menu_pix;

    if(dis_close_pix) delete dis_close_pix;
    if(dis_maximize_pix) delete dis_maximize_pix;
    if(dis_minimize_pix) delete dis_minimize_pix;
    if(dis_normalize_pix) delete dis_normalize_pix;
    if(dis_pinup_pix) delete dis_pinup_pix;
    if(dis_pindown_pix) delete dis_pindown_pix;
    if(dis_menu_pix) delete dis_menu_pix;
    
    QPainter pact, pdis;
    QBitmap bitmap;
    QColor actHigh = actMed.light(150);
    QColor actLow = actMed.dark(120);
    QColor disHigh = disMed.light(150);
    QColor disLow = disMed.dark(120);
    
    close_pix = new QPixmap(16, 16);
    dis_close_pix = new QPixmap(16, 16);
    pact.begin(close_pix); pdis.begin(dis_close_pix);
    bitmap = QBitmap(16, 16, close_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, close_dgray_bits, true);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, close_mask_bits, true);
    close_pix->setMask(bitmap); dis_close_pix->setMask(bitmap);

    minimize_pix = new QPixmap(16, 16);
    dis_minimize_pix = new QPixmap(16, 16);
    pact.begin(minimize_pix); pdis.begin(dis_minimize_pix);
    bitmap = QBitmap(16, 16, iconify_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, iconify_dgray_bits, true);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, iconify_mask_bits, true);
    minimize_pix->setMask(bitmap); dis_minimize_pix->setMask(bitmap);
    
    maximize_pix = new QPixmap(16, 16);
    dis_maximize_pix = new QPixmap(16, 16);
    pact.begin(maximize_pix); pdis.begin(dis_maximize_pix);
    bitmap = QBitmap(16, 16, maximize_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, maximize_dgray_bits, true);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, maximize_mask_bits, true);
    maximize_pix->setMask(bitmap); dis_maximize_pix->setMask(bitmap);

    normalize_pix = new QPixmap(16, 16);
    dis_normalize_pix = new QPixmap(16, 16);
    pact.begin(normalize_pix); pdis.begin(dis_normalize_pix);
    bitmap = QBitmap(16, 16, maximizedown_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, maximizedown_dgray_bits, true);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, maximizedown_mask_bits, true);
    normalize_pix->setMask(bitmap); dis_normalize_pix->setMask(bitmap);

    menu_pix = new QPixmap(16, 16);
    dis_menu_pix = new QPixmap(16, 16);
    pact.begin(menu_pix); pdis.begin(dis_menu_pix);
    bitmap = QBitmap(16, 16, menu_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, menu_dgray_bits, true);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, menu_mask_bits, true);
    menu_pix->setMask(bitmap); dis_menu_pix->setMask(bitmap);

    pinup_pix = new QPixmap(16, 16);
    dis_pinup_pix = new QPixmap(16, 16);
    pact.begin(pinup_pix); pdis.begin(dis_pinup_pix);
    bitmap = QBitmap(16, 16, pinup_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, pinup_gray_bits, true);
    pact.setPen(actMed); pdis.setPen(disMed);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, pinup_dgray_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, pinup_mask_bits, true);
    pinup_pix->setMask(bitmap); dis_pinup_pix->setMask(bitmap);
    
    pindown_pix = new QPixmap(16, 16);
    dis_pindown_pix = new QPixmap(16, 16);
    pact.begin(pindown_pix); pdis.begin(dis_pindown_pix);
    bitmap = QBitmap(16, 16, pindown_white_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actHigh); pdis.setPen(disHigh);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, pindown_gray_bits, true);
    pact.setPen(actMed); pdis.setPen(disMed);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    bitmap = QBitmap(16, 16, pindown_dgray_bits, true);
    bitmap.setMask(bitmap);
    pact.setPen(actLow); pdis.setPen(disLow);
    pact.drawPixmap(0, 0, bitmap);
    pdis.drawPixmap(0, 0, bitmap);
    pact.end(); pdis.end();
    bitmap = QBitmap(16, 16, pindown_mask_bits, true);
    pindown_pix->setMask(bitmap); dis_pindown_pix->setMask(bitmap);
    
}

void WidgetCanvas::drawSampleWidgets()
{
    int textLen, tmp;
    int highlightVal, lowlightVal;

    // Calculate the highlight and lowloght from contrast value and create
    // color group from color scheme.

    highlightVal=100+(2*contrast+4)*16/10;
    lowlightVal=100+(2*contrast+4)*10;

    QColorGroup cg( txt, back,
                    back.light(highlightVal),
                    back.dark(lowlightVal),
                    back.dark(120),
                    txt, window );

    // We will need this brush.

    QBrush brush(SolidPattern);
    brush.setColor( back );

    // Create a scrollbar and redirect drawing into a temp. pixmap to save a
    // lot of fiddly drawing later.

    QScrollBar *vertScrollBar = new QScrollBar( QScrollBar::Vertical, this );
    // TODO: vertScrollBar->setStyle( new QMotifStyle() );
    vertScrollBar->setGeometry( 400, 400, SCROLLBAR_SIZE, height());
    vertScrollBar->setRange( 0,  0 );
    vertScrollBar->setPalette( QPalette(cg,cg,cg));
    vertScrollBar->show();

    QPixmap pm( vertScrollBar->width(), vertScrollBar->height() );
    pm.fill( back );
    QPainter::redirect( vertScrollBar, &pm );
    vertScrollBar->repaint();
    QPainter::redirect( vertScrollBar, 0 );
    vertScrollBar->hide();

    // Reset the titlebar pixmaps
    resetTitlebarPixmaps(aTitleBtn, iTitleBtn);
	
    // Initialize the pixmap which we draw sample widgets into.

    smplw.resize(width(), height());
    //smplw.fill( parentWidget()->back() );
    smplw.fill( parentWidget()->colorGroup().mid() );

    // Actually start painting in

    QPainter paint( &smplw );

    // Inactive window

    qDrawWinPanel ( &paint, 15, 5, width()-48, height(), cg, FALSE,
                    &brush);

    paint.setBrush( iaTitle );
    paint.setPen( iaTitle );
    //paint.drawRect( 20, 10, width()-60, 20 );

    KPixmap pmTitle;
    pmTitle.resize( width()-160, 20 );
    KPixmapEffect::gradient(pmTitle, iaTitle, iaBlend,
                            KPixmapEffect::HorizontalGradient);
    paint.drawPixmap( 60, 10, pmTitle );

    QFont fnt = KGlobalSettings::generalFont();
    paint.setFont( fnt );
    paint.setPen( iaTxt );
    paint.drawText( 65, 25, i18n("Inactive window") );
    textLen = paint.fontMetrics().width(  i18n("Inactive window") );

    tmp = width()-100;
    paint.drawPixmap(22, 12, *dis_menu_pix);
    paint.drawPixmap(42, 12, *dis_pinup_pix);
    paint.drawPixmap(tmp+2, 12, *dis_minimize_pix);
    paint.drawPixmap(tmp+22, 12, *dis_maximize_pix);
    paint.drawPixmap(tmp+42, 12, *dis_close_pix);
    
    int spot = 0;
    hotspots[ spot++ ] =
        HotSpot( QRect( 65, 25-14, textLen, 14 ), 1 ); // inactive text

    hotspots[ spot++ ] =
        HotSpot( QRect( 60, 10, (width()-160)/2, 20 ), 0 ); // inactive title

    hotspots[ spot++ ] =
        HotSpot( QRect( 65+(width()-160)/2, 10,
                        (width()-160)/2, 20 ), 2 ); // inactive blend

    hotspots[spot++] =
        HotSpot(QRect(20, 12, 40, 20), 15); // inactive titlebtn bg
    hotspots[spot++] =
        HotSpot(QRect(tmp, 12, 60, 20), 15);
    

    // Active window

    qDrawWinPanel ( &paint, 20, 25+5, width()-40, height(), cg, FALSE,
                    &brush);

    paint.setBrush( aTitle );paint.setPen( aTitle );
    paint.drawRect( 65, 30+5, width()-152, 20 );

    pmTitle.resize( width()-152, 20 );
    KPixmapEffect::gradient(pmTitle, aTitle, aBlend,
                            KPixmapEffect::VerticalGradient);
    paint.drawPixmap( 65, 35, pmTitle );

    paint.setFont( fnt );
    paint.setPen( aTxt );
    paint.drawText( 75, 50,  i18n("Active window") );
    textLen = paint.fontMetrics().width(  i18n("Active window" ));

    tmp = width()-152+65;
    paint.drawPixmap(27, 35, *menu_pix);
    paint.drawPixmap(47, 35, *pinup_pix);
    paint.drawPixmap(tmp+2, 35, *minimize_pix);
    paint.drawPixmap(tmp+22, 35, *maximize_pix);
    paint.drawPixmap(tmp+42, 35, *close_pix);

    hotspots[ spot++ ] =
        HotSpot( QRect( 75, 50, textLen, 14 ), 4 ); // Active text
    hotspots[ spot ++] =
        HotSpot( QRect( 65, 35, (width()-152)/2, 20 ), 3 ); // Active title
    hotspots[ spot ++] =
        HotSpot( QRect( 65+(width()-152)/2, 35,
                        (width()-152)/2, 20 ), 5 ); // Active title

    hotspots[spot++] =
        HotSpot(QRect(25, 35, 40, 20), 14); // Active titlebtn fg
    hotspots[spot++] =
        HotSpot(QRect(tmp, 35, 60, 20), 14);
    
    // Menu bar

    qDrawShadePanel ( &paint, 25, 55, width()-52, 28, cg, FALSE, 2, &brush);

    fnt.setBold(FALSE);
    paint.setFont( fnt );
    paint.setPen(txt );
    textLen = paint.fontMetrics().width( i18n("File") );
    qDrawShadePanel ( &paint, 30, 59, textLen + 10, 21, cg, FALSE, 2, &brush);
    paint.drawText( 35, 74, i18n("File") );

    hotspots[ spot++ ] =
        HotSpot( QRect( 35, 62, textLen, 14 ), 7 );
    hotspots[ spot++ ] =
        HotSpot( QRect( 27, 57, 33, 21 ), 6 );

    paint.setFont( fnt );
    paint.setPen( txt );
    paint.drawText( 35 + textLen + 20, 69+5, i18n("Edit") );
    textLen = paint.fontMetrics().width( i18n("Edit") );

    hotspots[ spot++ ] = HotSpot( QRect( 65, 62, textLen, 14 ), 7 ); // text

    // Button Rects need to go before window

    // Frame and window contents

    brush.setColor( window );
    qDrawShadePanel ( &paint, 25, 80+5-4, width()-7-45-2,
                      height(), cg, TRUE, 2, &brush);

    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( windowTxt );
    paint.drawText( 140, 127-20, i18n( "Window text") );
    textLen = paint.fontMetrics().width( i18n("Window text") );

    hotspots[ spot++ ] =
        HotSpot( QRect( 140, 113-20, textLen, 14 ), 11 ); // window text


    textLen = paint.fontMetrics().width( i18n("Selected text") );
    paint.setBrush( select );paint.setPen( select );
    paint.drawRect ( 120, 115, textLen+40, 32);

    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( selectTxt );
    paint.drawText( 140, 135, i18n( "Selected text") );

    hotspots[ spot++ ] =
        HotSpot( QRect( 140, 121, textLen, 14 ), 9 ); // select text
    hotspots[ spot++ ] =
        HotSpot( QRect( 120, 115, textLen+40, 32), 8 ); // select bg


    // Button
    int xpos = 120 + textLen + 40 + 16;
    int ypos = 115 + 2;
    textLen = paint.fontMetrics().width(i18n("Push Button"));
    hotspots[ spot++ ] =
        HotSpot( QRect(xpos+16, ypos+((28-paint.fontMetrics().height())/2),
                       textLen, paint.fontMetrics().height()), 13 );
    hotspots[ spot++ ] =
        HotSpot( QRect(xpos, ypos, textLen+32, 28), 12 );
    brush.setColor( button );
    qDrawWinButton(&paint, xpos, ypos, textLen+32, 28, cg, false, &brush);
    paint.setPen(buttonTxt);
    paint.drawText(xpos, ypos, textLen+32, 28, AlignCenter,
                   i18n("Push Button"));

    // Scrollbar
    paint.drawPixmap(width()-55+27-16-2,80+5-2,pm);

    // Menu

    brush.setColor( back );
    qDrawShadePanel ( &paint, 30, 80, 84, 73, cg, FALSE, 2, &brush);

    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( txt );
    paint.drawText( 38, 97, i18n("New") );

    hotspots[ spot++ ] =
        HotSpot( QRect( 38, 83, textLen, 14 ), 7 );
    //hotspots[ spot++ ] =
    //HotSpot( QRect( 28, 97, 78, 21 ), 6 );

    //qDrawShadePanel ( &paint, 32, 101, 80, 25, cg, FALSE, 2,
    //&brush);

    paint.setFont( fnt );
    paint.drawText( 38, 119, i18n("Open") );
    textLen = paint.fontMetrics().width( i18n("Open") );

    hotspots[ spot++ ] =
        HotSpot( QRect( 38, 105, textLen, 14 ), 7 );
    hotspots[ spot++ ] =
        HotSpot( QRect( 28, 101, 78, 21 ), 6 );

    paint.setFont( fnt );
    paint.setPen( lightGray.dark() );
    paint.drawText( 38, 141, i18n("Save") );
    textLen = paint.fontMetrics().width( i18n("Save") );


    hotspots[ spot++ ] =
        HotSpot( QRect(25, 80+5-4, width()-7-45-2, height()), 10 ); // window bg


    // Valance

    qDrawWinPanel ( &paint, 0, 0, width(), height(),
                    parentWidget()->colorGroup(), TRUE, 0);

    // Stop the painting

    hotspots[ spot++ ] =
        HotSpot( QRect( 0, 0, width(), height() ), 6 );

    repaint( FALSE );
}
