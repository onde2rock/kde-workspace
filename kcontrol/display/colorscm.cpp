//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qslider.h>
#include <qradiobt.h>
#include <qmsgbox.h>
#include <qdrawutl.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qbitmap.h>

#include <kapp.h>
#include <kcharsets.h>
#include <kmsgbox.h>
#include <ksimpleconfig.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "kcolordlg.h"
#include "kwmcom.h"

#include "widgetcanvas.h"
#include "kresourceman.h"

#include "colorscm.h"
#include "colorscm.moc"

#define SUPPLIED_SCHEMES 5

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define hand_width 16
#define hand_height 16

static unsigned char hand_bits[] = {
	0x00,0x00,0xfe,0x01,0x01,0x02,0x7e,0x04,0x08,0x08,0x70,0x08,0x08,0x08,0x70,
	0x14,0x08,0x22,0x30,0x41,0xc0,0x20,0x40,0x12,0x80,0x08,0x00,0x05,0x00,0x02,
	0x00,0x00};
static unsigned char hand_mask_bits[] = {
	0xfe,0x01,0xff,0x03,0xff,0x07,0xff,0x0f,0xfe,0x1f,0xf8,0x1f,0xfc,0x1f,0xf8,
	0x3f,0xfc,0x7f,0xf8,0xff,0xf0,0x7f,0xe0,0x3f,0xc0,0x1f,0x80,0x0f,0x00,0x07,
	0x00,0x02};


int dropError(Display *, XErrorEvent *)
{
	return 0;
}

KColorScheme::KColorScheme( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{	
	changed = FALSE;
	nSysSchemes = 2;

	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	debug("KColorScheme::KColorScheme");
	
	kde_display = x11Display();
	KDEChangePalette = XInternAtom( kde_display, "KDEChangePalette", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);
	
	setName( i18n("Colors") );
	
	QBitmap cb( hand_width, hand_height, hand_bits, TRUE );
	QBitmap cm( hand_width, hand_height, hand_mask_bits, TRUE );
	QCursor handCursor( cb, cm, 0, 0 );
	
	cs = new WidgetCanvas( this );
	cs->setCursor( handCursor );
	
	// LAYOUT
	
	QGridLayout *topLayout = new QGridLayout( this, 4, 4, 10 );
	
	topLayout->setRowStretch(0,0);
	topLayout->setRowStretch(1,10);
	topLayout->setRowStretch(2,10);
	topLayout->setRowStretch(3,0);
	
	topLayout->setColStretch(0,0);
	topLayout->setColStretch(1,10);
	topLayout->setColStretch(2,10);
	topLayout->setColStretch(3,0);
	
	//cs->drawSampleWidgets();
	cs->setFixedHeight( 150 );
	connect( cs, SIGNAL( widgetSelected( int ) ), 
			SLOT( slotWidgetColor( int ) ) );
			
	topLayout->addMultiCellWidget( cs, 1, 1, 1, 2 );

	QGroupBox *group = new QGroupBox( i18n("Color Scheme"), this );
	
	topLayout->addWidget( group, 2, 1 );
	
	QBoxLayout *groupLayout = new QVBoxLayout( group, 10 );
	
	sFileList = new QStrList(); 
	sList = new QListBox( group );
	
	sList->clear();
	sFileList->clear();
	sList->insertItem( i18n("Current scheme"), 0 );
	sFileList->append( "Not a  kcsrc file" );
	sList->insertItem( i18n("KDE default"), 1 );
	sFileList->append( "Not a kcsrc file" );
	readSchemeNames();
	sList->setCurrentItem( 0 );
	connect( sList, SIGNAL( highlighted( int ) ),
			SLOT( slotPreviewScheme( int ) ) );
					
	groupLayout->addSpacing( 20 );
	groupLayout->addWidget( sList, 10 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	groupLayout->addLayout( pushLayout );
	
	addBt = new QPushButton(  i18n("&Add ..."), group );
	addBt->setFixedHeight( addBt->sizeHint().height() );
	connect( addBt, SIGNAL( clicked() ), SLOT( slotAdd() ) );
	
	pushLayout->addWidget( addBt, 10 );
	
	removeBt = new QPushButton(  i18n("&Remove"), group );
	removeBt->setFixedHeight( removeBt->sizeHint().height() );
	removeBt->setEnabled(FALSE);
	connect( removeBt, SIGNAL( clicked() ), SLOT( slotRemove() ) );
	
	pushLayout->addWidget( removeBt, 10 );
	
	saveBt = new QPushButton(  i18n("&Save changes"), group );
	saveBt->setFixedHeight( saveBt->sizeHint().height() );
	saveBt->setEnabled(FALSE);
	connect( saveBt, SIGNAL( clicked() ), SLOT( slotSave() ) );
	
	groupLayout->addWidget( saveBt, 10 );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 10 );
	
	topLayout->addLayout( stackLayout, 2, 2 );

	group = new QGroupBox(  i18n("Widget color"), this );
	
	stackLayout->addWidget( group, 10 );
	
	groupLayout = new QVBoxLayout( group, 10 );

	wcCombo = new QComboBox( false, group );
	wcCombo->insertItem(  i18n("Inactive title bar") );
	wcCombo->insertItem(  i18n("Inactive title text") );
	wcCombo->insertItem(  i18n("Inactive title blend") );
	wcCombo->insertItem(  i18n("Active title bar") );
	wcCombo->insertItem(  i18n("Active title text") );
	wcCombo->insertItem(  i18n("Active title blend") );
	wcCombo->insertItem(  i18n("Background") );
	wcCombo->insertItem(  i18n("Text") );
	wcCombo->insertItem(  i18n("Select background") );
	wcCombo->insertItem(  i18n("Select text") );
	wcCombo->insertItem(  i18n("Window background") );
	wcCombo->insertItem(  i18n("Window text") );
	wcCombo->setMinimumSize( wcCombo->sizeHint() );
	wcCombo->setFixedHeight( wcCombo->sizeHint().height() );
	connect( wcCombo, SIGNAL( activated(int) ),
			SLOT( slotWidgetColor(int)  )  );
	
	groupLayout->addSpacing( 20 );		
	groupLayout->addWidget( wcCombo );
	
	colorButton = new KColorButton( group );
	colorButton->setFixedHeight( wcCombo->sizeHint().height() );
	colorButton->setColor( cs->iaTitle );
	colorPushColor = cs->iaTitle;
	connect( colorButton, SIGNAL( changed(const QColor &) ),
		SLOT( slotSelectColor(const QColor &) ) );
		
	group->setMinimumHeight( 3*wcCombo->sizeHint().height()+20 );
		
	groupLayout->addWidget( colorButton );
	
	group = new QGroupBox(  i18n("Contrast"), this );
	
	stackLayout->addWidget( group, 50 );
	
	groupLayout = new QHBoxLayout( group, 20 );
	
	sb = new QSlider( QSlider::Horizontal,group,"Slider" );
    sb->setRange( 0, 10 );
    sb->setValue(cs->contrast);
    sb->setFocusPolicy( QWidget::StrongFocus ); 
	sb->setFixedHeight( sb->sizeHint().height() );
    connect( sb, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)) );
	
	QLabel *label = new QLabel( sb, i18n("&Low"), group );
	label->setFixedHeight( sb->sizeHint().height() );
	label->setFixedWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );
	groupLayout->addWidget( sb, 10 );

	label = new QLabel( group );
    label->setText(  i18n("High"));
	label->setFixedHeight( sb->sizeHint().height() );
	label->setFixedWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );
	
	slotPreviewScheme( 0 );
	
	topLayout->activate();
	
	kwmcom_init( qt_xdisplay(), 0 );
}

void KColorScheme::resizeEvent( QResizeEvent * )
{
	cs->drawSampleWidgets();
}

void KColorScheme::sliderValueChanged( int val )
{
	cs->contrast = val;
	cs->drawSampleWidgets();
}

void KColorScheme::slotSave( )
{
	KSimpleConfig *config = 
			new KSimpleConfig( sFileList->at( sList->currentItem() ) );
				
	config->setGroup( "Color Scheme" );
	config->writeEntry( "background", cs->back );
	config->writeEntry( "selectBackground", cs->select );
	config->writeEntry( "foreground", cs->txt );
	config->writeEntry( "activeForeground", cs->aTxt );
	config->writeEntry( "inactiveBackground", cs->iaTitle );
	config->writeEntry( "inactiveBlend", cs->iaBlend );
	config->writeEntry( "activeBackground", cs->aTitle );
	config->writeEntry( "activeBlend", cs->aBlend );
	config->writeEntry( "inactiveForeground", cs->iaTxt );
	config->writeEntry( "windowForeground", cs->windowTxt );
	config->writeEntry( "windowBackground", cs->window );
	config->writeEntry( "selectForeground", cs->selectTxt );
	config->writeEntry( "contrast", cs->contrast );
	
	config->sync();
	
	saveBt->setEnabled( FALSE );
}

void KColorScheme::slotRemove()
{
	QString kcsPath( getenv( "HOME" ) );
	kcsPath += "/.kde/share/apps/kdisplay/color-schemes";
	
	QDir d;
	d.setPath( kcsPath );
	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.kcsrc");
	
	uint ind = sList->currentItem();
	
	if ( !d.remove( sFileList->at( ind ) ) ) {
		QMessageBox::critical( 0, i18n("Error removing"),
			i18n("This color scheme could not be removed.\n"\
				"Perhaps you do not have permission to alter the file\n"\
				"system where the color scheme is stored." ) );
		return;
	}
	
	sList->removeItem( ind );
	sFileList->remove( ind );
	
}

void KColorScheme::slotAdd()
{
	SaveScm *ss = new SaveScm( 0,  "save scheme" );
	
	bool nameValid;
	QString sName;
	QString sFile; 
	
	do {
	
	nameValid = TRUE;
	
	if ( ss->exec() ) {
		sName = ss->nameLine->text();
		if ( sName.stripWhiteSpace().isEmpty() )
			return;
			
		sName = sName.simplifyWhiteSpace();
		sFile.sprintf(sName);
		
		int ind = 0;
		while ( ind < (int) sFile.length() ) {
			
			// parse the string for first white space
			
			ind = sFile.find(" ");
			if (ind == -1) {
				ind = sFile.length();
				break;
			}
		
			// remove from string
		
			sFile.remove( ind, 1);
			
			// Make the next letter upper case 
			
			QString s = sFile.mid( ind, 1 );
			s = s.upper();
			sFile.replace( ind, 1, s.data() );
			
		}
		
		for ( int i = 0; i < (int) sList->count(); i++ ) {
			if ( sName == sList->text( i ) ) {
				nameValid = FALSE;
				QMessageBox::critical( 0, i18n( "Naming error" ),
					i18n( "Please choose a unique name for the new color\n"\
							"scheme. The one you entered already appears\n"\
							"in the color scheme list." ) );
			}
		}
	} else return;
	
	} while ( nameValid == FALSE );
	
	disconnect( sList, SIGNAL( highlighted( int ) ), this,
			SLOT( slotPreviewScheme( int ) ) );
	
	sList->insertItem( sName.data() );
	sList->setFocus();
	sList->setCurrentItem( sList->count()-1 );
	
	QString kcsPath( getenv( "HOME" ) );
	kcsPath += "/.kde/share/apps/kdisplay/color-schemes/";
	
	QDir d( kcsPath.data() );
	if ( !d.exists() )
		d.mkdir( kcsPath.data() );
	
	sFile.prepend( kcsPath );
	sFile += ".kcsrc";
	sFileList->append( sFile.data() );
	
	KSimpleConfig *config = 
			new KSimpleConfig( sFile.data() );
			
	config->setGroup( "Color Scheme" );
	config->writeEntry( "name", sName );
	delete config;
	
	slotSave();
	
	connect( sList, SIGNAL( highlighted( int ) ), this,
			SLOT( slotPreviewScheme( int ) ) );
			
	slotPreviewScheme( sList->currentItem() );
}

void KColorScheme::slotSelectColor( const QColor &col )
{
	colorPushColor = col;
	
	int selection;
	selection = wcCombo->currentItem()+1;
	switch(selection) {
		case 1:		cs->iaTitle = colorPushColor;
					break;
		case 2:		cs->iaTxt = colorPushColor;
					break;
		case 3:		cs->iaBlend = colorPushColor;
					break;
		case 4:		cs->aTitle = colorPushColor;
					break;
		case 5:		cs->aTxt = colorPushColor;
					break;
		case 6:		cs->aBlend = colorPushColor;
					break;
		case 7:		cs->back = colorPushColor;
					break;
		case 8:		cs->txt = colorPushColor;
					break;
		case 9:		cs->select = colorPushColor;
					break;
		case 10:	cs->selectTxt = colorPushColor;
					break;
		case 11:	cs->window = colorPushColor;
					break;
		case 12:	cs->windowTxt = colorPushColor;
					break;
	}
	
	cs->drawSampleWidgets();
	
	if ( removeBt->isEnabled() )
		saveBt->setEnabled( TRUE );
	else
		saveBt->setEnabled( FALSE );
		

	changed=TRUE;
}

void KColorScheme::slotWidgetColor( int indx )
{
	int selection;
	QColor col;
	
	if ( wcCombo->currentItem() != indx )
		wcCombo->setCurrentItem( indx );

	selection = indx + 1;
	switch(selection) {
		case 1:		col = cs->iaTitle;
					break;
		case 2:		col = cs->iaTxt;
					break;
		case 3: 	col = cs->iaBlend;
					break;
		case 4:		col = cs->aTitle;
					break;
		case 5:		col = cs->aTxt;
					break;	
		case 6: 	col = cs->aBlend;
					break;
		case 7:		col = cs->back;
					break;
		case 8:		col = cs->txt;
					break;
		case 9:		col = cs->select;
					break;
		case 10:	col = cs->selectTxt;
					break;
		case 11:	col = cs->window;
					break;
		case 12:	col = cs->windowTxt;
					break;
	}

	colorButton->setColor( col );
	colorPushColor = col;	
}

void KColorScheme::writeNamedColor( KConfigBase *config,
							const char *key, const char *name)
{
	QColor tmp;
	tmp.setNamedColor( name) ;
	config->writeEntry( key, tmp );
}

KColorScheme::~KColorScheme()
{
}

void KColorScheme::readScheme( int index )
{
	KConfigBase* config;
	
	if( index == 1 ) {
		cs->back = lightGray;
		cs->txt = black;
		cs->select = darkBlue;
		cs->selectTxt = white;
		cs->window = white;
		cs->windowTxt = black;
		cs->iaTitle = darkGray;
		cs->iaTxt = lightGray;
		cs->iaBlend = lightGray;
		cs->aTitle = darkBlue;
		cs->aTxt = white;
		cs->aBlend = black;
		cs->contrast = 7;
		
		return;
	} if ( index == 0 ) {
		config  = kapp->getConfig();
	} else {
		config = 
			new KSimpleConfig( sFileList->at( index ), true );
	}
	
	if ( index == 0 )
		config->setGroup( "General" );
	else
		config->setGroup( "Color Scheme" );

	cs->txt = 
	config->readColorEntry( "foreground", &black );

	cs->back = 
	config->readColorEntry( "background", &lightGray );

	cs->select =
	config->readColorEntry( "selectBackground", &darkBlue);

	cs->selectTxt =
	config->readColorEntry( "selectForeground", &white );

	cs->window =
	config->readColorEntry( "windowBackground", &white );

	cs->windowTxt =
	config->readColorEntry( "windowForeground", &black );
	
	if ( index == 0 ) config->setGroup( "WM" );
	
	cs->iaTitle = 
	config->readColorEntry( "inactiveBackground", &darkGray);

	cs->iaTxt = 
	config->readColorEntry( "inactiveForeground", &lightGray );

	cs->iaBlend = 
	config->readColorEntry( "inactiveBlend", &lightGray );

	cs->aTitle =
	config->readColorEntry( "activeBackground", &darkBlue );

	cs->aTxt = 
	config->readColorEntry( "activeForeground", &white );

	cs->aBlend = 
	config->readColorEntry( "activeBlend", &black );

	if ( index == 0 ) config->setGroup( "KDE" );

	cs->contrast = 
	config->readNumEntry( "contrast", 7 );
}

void KColorScheme::readSchemeNames( )
{
	QString kcsPath( kapp->kde_datadir() );
	kcsPath += "/kdisplay/color-schemes";
	
	QDir d;
	d.setPath( kcsPath );
	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.kcsrc");
	
	if ( const QFileInfoList *sysList = d.entryInfoList() ) {
		QFileInfoListIterator sysIt( *sysList );
		QFileInfo *fi;

		// Always a current and a default scheme 
		nSysSchemes = 2;

		QString str;

		// This for system files
		while ( ( fi = sysIt.current() ) ) {           // for each file...

			KSimpleConfig *config = new KSimpleConfig( fi->filePath(), true );
			config->setGroup( "Color Scheme" );
			str = config->readEntry( "name" );

			sList->insertItem( str.data() );
			sFileList->append( fi->filePath() );

			++sysIt;
			nSysSchemes++;

			delete config;
		}
	}
	
	kcsPath.sprintf( getenv( "HOME" ) );
	kcsPath += "/.kde/share/apps/kdisplay/color-schemes";
	
	d.setPath( kcsPath );
	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.kcsrc");
	
	if ( const QFileInfoList *userList = d.entryInfoList() ) {
		QFileInfoListIterator userIt( *userList );
		QFileInfo *fi;
		QString str;

		// This for users files
		while ( ( fi = userIt.current() ) ) {

			KSimpleConfig config( fi->filePath(), true );
			config.setGroup( "Color Scheme" );
			str = config.readEntry( "name" );

			sList->insertItem( str.data() );
			sFileList->append( fi->filePath() );

			++userIt;
		}
	}
		
}

void KColorScheme::writeSettings()
{
	if ( !changed )
		return;
		
	KConfig* sys = kapp->getConfig();
	
	sys->setGroup( "General" );
	sys->writeEntry("background", cs->back, true, true);
	sys->writeEntry("selectBackground", cs->select, true, true);
	sys->writeEntry("foreground", cs->txt, true, true);
	sys->writeEntry("windowForeground", cs->windowTxt, true, true);
	sys->writeEntry("windowBackground", cs->window, true, true);
	sys->writeEntry("selectForeground", cs->selectTxt, true, true);
	
	sys->setGroup( "WM" );
	sys->writeEntry("activeForeground", cs->aTxt, true, true);
	sys->writeEntry("inactiveBackground", cs->iaTitle, true, true);
	sys->writeEntry("inactiveBlend", cs->iaBlend, true, true);
	sys->writeEntry("activeBackground", cs->aTitle, true, true);
	sys->writeEntry("activeBlend", cs->aBlend, true, true);
	sys->writeEntry("inactiveForeground", cs->iaTxt, true, true);
	
	sys->setGroup( "KDE" );
	sys->writeEntry("contrast", cs->contrast, true, true);
    sys->sync();
	
	QApplication::setOverrideCursor( waitCursor );
	
	KResourceMan *krdb = new KResourceMan();
	
	krdb->setGroup( "General" );
	krdb->writeEntry("background", cs->back );
	krdb->writeEntry("selectBackground", cs->select );
	krdb->writeEntry("foreground", cs->txt );
	krdb->writeEntry("windowForeground", cs->windowTxt );
	krdb->writeEntry("windowBackground", cs->window );
	krdb->writeEntry("selectForeground", cs->selectTxt );
	
	krdb->setGroup( "WM" );
	krdb->writeEntry("activeForeground", cs->aTxt );
	krdb->writeEntry("inactiveBackground", cs->iaTitle );
	krdb->writeEntry("inactiveBlend", cs->iaBlend );
	krdb->writeEntry("activeBackground", cs->aTitle );
	krdb->writeEntry("activeBlend", cs->aBlend );
	krdb->writeEntry("inactiveForeground", cs->iaTxt );
	
	krdb->setGroup( "KDE" );
	krdb->writeEntry("contrast", cs->contrast );
    krdb->sync();
	
	QApplication::restoreOverrideCursor();
}

void KColorScheme::slotApply()
{
	writeSettings();
	apply();
}

//Matthias
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;
  
  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((void*) *p);
  return n;
}

//Matthias
static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;
  
  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }
  
  result = p[0];
  XFree((char *) p);
  return TRUE;
}

void KColorScheme::apply( bool  )
{
	if ( !changed )
		return;
	
	XEvent ev;
	unsigned int i, nrootwins;
	Window dw1, dw2, *rootwins;
	int (*defaultHandler)(Display *, XErrorEvent *);

	defaultHandler=XSetErrorHandler(dropError);
	
	XQueryTree(kde_display, root, &dw1, &dw2, &rootwins, &nrootwins);
	
	// Matthias
	Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
	for (i = 0; i < nrootwins; i++) {
	  long result = 0;
	  getSimpleProperty(rootwins[i],a, result);
	  if (result){
	    ev.xclient.type = ClientMessage;
	    ev.xclient.display = kde_display;
	    ev.xclient.window = rootwins[i];
	    ev.xclient.message_type = KDEChangePalette;
	    ev.xclient.format = 32;
	    
	    XSendEvent(kde_display, rootwins[i] , False, 0L, &ev);
	  }
	}

	XFlush(kde_display);
	XSetErrorHandler(defaultHandler);
	
	XFree((void *) rootwins);
	
	changed=FALSE;
}

void KColorScheme::slotPreviewScheme( int indx )
{
	readScheme( indx );
	
	// Set various appropriate for the scheme
	
	cs->drawSampleWidgets();
	sb->setValue( cs->contrast );
	slotWidgetColor( 0 );
	if ( indx < nSysSchemes ) {
		removeBt->setEnabled( FALSE );
		saveBt->setEnabled( FALSE );
	} else
		removeBt->setEnabled( TRUE );
	changed = TRUE;
}

void KColorScheme::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-5.html", "" );
}

void KColorScheme::applySettings()
{
  writeSettings();
  apply();
}
