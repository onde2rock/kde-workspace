// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) (C) 2000,2001,2002 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <netwm.h>

#include <QTimer>
#include <QX11Info>
#include <QUuid>
#include <QFile>

#include <kconfig.h>
#include <kdialog.h>
#include <ktextedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kprocess.h>
#include <kservice.h>
#include <kdebug.h>
#include <kstringhandler.h>
#include <kmacroexpander.h>
#include <kglobal.h>
#include <kmimetypetrader.h>
#include <kmimetype.h>

#include "klippersettings.h"
#include "urlgrabber.h"

// TODO:
// - script-interface?

URLGrabber::URLGrabber()
{
    m_myCurrentAction = 0L;
    m_myMenu = 0L;
    m_myPopupKillTimeout = 8;
    m_trimmed = true;

    m_myPopupKillTimer = new QTimer( this );
    m_myPopupKillTimer->setSingleShot( true );
    connect( m_myPopupKillTimer, SIGNAL( timeout() ),
             SLOT( slotKillPopupMenu() ));

    // testing
    /*
    ClipAction *action;
    action = new ClipAction( "^http:\\/\\/", "Web-URL" );
    action->addCommand("kfmclient exec %s", "Open with Konqi", true);
    action->addCommand("netscape -no-about-splash -remote \"openURL(%s, new-window)\"", "Open with Netscape", true);
    m_myActions->append( action );

    action = new ClipAction( "^mailto:", "Mail-URL" );
    action->addCommand("kmail --composer %s", "Launch kmail", true);
    m_myActions->append( action );

    action = new ClipAction( "^\\/.+\\.jpg$", "Jpeg-Image" );
    action->addCommand("kuickshow %s", "Launch KuickShow", true);
    action->addCommand("kview %s", "Launch KView", true);
    m_myActions->append( action );
    */
}


URLGrabber::~URLGrabber()
{
    qDeleteAll(m_myActions);
    m_myActions.clear();
    delete m_myMenu;
}

//
// Called from Klipper::slotRepeatAction, i.e. by pressing Ctrl-Alt-R
// shortcut. I.e. never from clipboard monitoring
//
void URLGrabber::invokeAction( const QString& clip )
{
    if ( !clip.isEmpty() )
        m_myClipData = clip;
    if ( m_trimmed )
        m_myClipData = m_myClipData.trimmed();

    actionMenu( false );
}


void URLGrabber::setActionList( const ActionList& list )
{
    qDeleteAll(m_myActions);
    m_myActions.clear();
    m_myActions = list;
}

void URLGrabber::matchingMimeActions(const QString& clipData)
{
    KUrl url(clipData);
    KConfigGroup cg(KGlobal::config(), "General");
    if(!cg.readEntry("EnableMagicMimeActions",true)) { //Secret configuration entry
    //    kDebug() << "skipping mime magic due to configuration";
    	return;
    }
    if(!url.isValid()) {
    //    kDebug() << "skipping mime magic due to invalid url";
    	return;
    }
    if(url.isRelative()) {  //openinng a relative path will just not work. what path should be used?
    //    kDebug() << "skipping mime magic due to relative url";
    	return;
    }
    if(url.isLocalFile()) {
	if(!QFile::exists(url.toLocalFile())) {
	//    kDebug() << "skipping mime magic due to nonexistant localfile";
	    return;
	}
    }

    // try to figure out if clipData contains a filename
    KMimeType::Ptr mimetype = KMimeType::findByUrl( url, 0,
                                                    false,
                                                    true /*fast mode*/ );

    // let's see if we found some reasonable mimetype.
    // If we do we'll populate menu with actions for apps
    // that can handle that mimetype

    // first: if clipboard contents starts with http, let's assume it's "text/html".
    // That is even if we've url like "http://www.kde.org/somescript.pl", we'll
    // still treat that as html page, because determining a mimetype using kio
    // might take a long time, and i want this function to be quick!
    if ( ( clipData.startsWith( "http://" ) || clipData.startsWith( "https://"))
         && mimetype->name() != "text/html" )
    {
        // use a fake path to create a mimetype that corresponds to "text/html"
        mimetype = KMimeType::findByPath( "/tmp/klipper.html", 0, true /*fast mode*/ );
    }

    if ( !mimetype->isDefault() ) {
        ClipAction* action = new ClipAction( QString(), mimetype->comment() );
        KService::List lst = KMimeTypeTrader::self()->query( mimetype->name(), "Application" );
        foreach( const KService::Ptr &service, lst ) {
            action->addCommand( service->exec(), service->name(), true, service->icon() );
        }
        if ( !lst.isEmpty() )
            m_myMatches.append( action );
    }
}

const ActionList& URLGrabber::matchingActions( const QString& clipData )
{
    m_myMatches.clear();

    matchingMimeActions(clipData);


    // now look for matches in custom user actions
    foreach (ClipAction* action, m_myActions) {
        if ( action->matches( clipData ) )
            m_myMatches.append( action );
    }

    return m_myMatches;
}


bool URLGrabber::checkNewData( const QString& clipData )
{
    // kDebug() << "** checking new data: " << clipData;
    m_myClipData = clipData;
    if ( m_trimmed )
        m_myClipData = m_myClipData.trimmed();

    if ( m_myActions.isEmpty() )
        return false;

    actionMenu( true ); // also creates m_myMatches

    return !m_myMatches.isEmpty();
}


void URLGrabber::actionMenu( bool wm_class_check )
{
    if ( m_myClipData.isEmpty() )
        return;

    ActionList matchingActionsList = matchingActions( m_myClipData );

    if (!matchingActionsList.isEmpty()) {
        // don't react on konqi's/netscape's urls...
        if ( wm_class_check && isAvoidedWindow() )
            return;

        m_myCommandMapper.clear();

        m_myPopupKillTimer->stop();

        m_myMenu = new KMenu;

        connect(m_myMenu, SIGNAL(triggered(QAction*)), SLOT(slotItemSelected(QAction*)));

        foreach (ClipAction* clipAct, matchingActionsList) {
            m_myMenu->addTitle(KIcon( "klipper" ),
                               i18n("%1 - Actions For: %2", clipAct->description(), KStringHandler::csqueeze(m_myClipData, 45)));
            QList<ClipCommand> cmdList = clipAct->commands();
            int listSize = cmdList.count();
            for (int i=0; i<listSize;++i) {
                ClipCommand command = cmdList.at(i);

                QString item = command.description;
                if ( item.isEmpty() )
                    item = command.command;

                QString id = QUuid::createUuid().toString();
                QAction * action = new QAction(this);
                action->setData(id);
                action->setText(item);

                if (!command.pixmap.isEmpty())
                    action->setIcon(KIcon(command.pixmap));

                m_myCommandMapper.insert(id, qMakePair(clipAct,i));
                m_myMenu->addAction(action);
            }
        }

        // only insert this when invoked via clipboard monitoring, not from an
        // explicit Ctrl-Alt-R
        if ( wm_class_check )
        {
            m_myMenu->addSeparator();
            QAction *disableAction = new QAction(i18n("Disable This Popup"), this);
            connect(disableAction, SIGNAL(triggered()), SIGNAL(sigDisablePopup()));
            m_myMenu->addAction(disableAction);
        }
        m_myMenu->addSeparator();
        // add an edit-possibility
        QAction *editAction = new QAction(KIcon("document-properties"), i18n("&Edit Contents..."), this);
        connect(editAction, SIGNAL(triggered()), SLOT(editData()));
        m_myMenu->addAction(editAction);

        QAction *cancelAction = new QAction(KIcon("dialog-cancel"), i18n("&Cancel"), this);
        connect(cancelAction, SIGNAL(triggered()), m_myMenu, SLOT(hide()));
        m_myMenu->addAction(cancelAction);

        if ( m_myPopupKillTimeout > 0 )
            m_myPopupKillTimer->start( 1000 * m_myPopupKillTimeout );

        emit sigPopup( m_myMenu );
    }
}


void URLGrabber::slotItemSelected(QAction *action)
{
    if (m_myMenu)
        m_myMenu->hide(); // deleted by the timer or the next action

    QString id = action->data().toString();

    if (id.isEmpty()) {
        kDebug() << "Klipper: no command associated";
        return;
    }

    // first is action ptr, second is command index
    QPair<ClipAction*, int> actionCommand = m_myCommandMapper.value(id);

    if (actionCommand.first)
        execute(actionCommand.first, actionCommand.second);
    else
        kDebug() << "Klipper: cannot find associated action";
}


void URLGrabber::execute( const ClipAction *action, int cmdIdx ) const
{
    if (!action) {
        kDebug() << "Action object is null";
        return;
    }

    ClipCommand command = action->command(cmdIdx);

    if ( command.isEnabled ) {
        QHash<QChar,QString> map;
        map.insert( 's', m_myClipData );

        // support %u, %U (indicates url param(s)) and %f, %F (file param(s))
        map.insert( 'u', m_myClipData );
        map.insert( 'U', m_myClipData );
        map.insert( 'f', m_myClipData );
        map.insert( 'F', m_myClipData );

        const QStringList matches = action->regExpMatches();
        // support only %0 and the first 9 matches...
        const int numMatches = qMin(10, matches.count());
        for ( int i = 0; i < numMatches; ++i )
            map.insert( QChar( '0' + i ), matches.at( i ) );

        QString cmdLine = KMacroExpander::expandMacrosShellQuote( command.command, map );

        if ( cmdLine.isEmpty() )
            return;

        KProcess proc;
        proc.setShellCommand(cmdLine.trimmed());
        if (!proc.startDetached())
            kDebug() << "Klipper: Could not start process!";
    }
}


void URLGrabber::editData()
{
    m_myPopupKillTimer->stop();
    KDialog *dlg = new KDialog( 0 );
    dlg->setModal( true );
    dlg->setCaption( i18n("Edit Contents") );
    dlg->setButtons( KDialog::Ok | KDialog::Cancel );

    KTextEdit *edit = new KTextEdit( dlg );
    edit->setText( m_myClipData );
    edit->setFocus();
    edit->setMinimumSize( 300, 40 );
    dlg->setMainWidget( edit );
    dlg->adjustSize();

    if ( dlg->exec() == KDialog::Accepted ) {
        m_myClipData = edit->toPlainText();
        QTimer::singleShot( 0, this, SLOT( slotActionMenu() ) );
    }
    else
    {
        m_myMenu->deleteLater();
        m_myMenu = 0;
    }
    delete dlg;
}


void URLGrabber::loadSettings()
{
    m_trimmed = KlipperSettings::stripWhiteSpace();
    m_myAvoidWindows = KlipperSettings::noActionsForWM_CLASS();
    m_myPopupKillTimeout = KlipperSettings::timeoutForActionPopups();

    qDeleteAll(m_myActions);
    m_myActions.clear();

    KConfigGroup cg(KGlobal::config(), "General");
    int num = cg.readEntry("Number of Actions", 0);
    QString group;
    for ( int i = 0; i < num; i++ ) {
        group = QString("Action_%1").arg( i );
        m_myActions.append( new ClipAction( KGlobal::config(), group ) );
    }
}

void URLGrabber::saveSettings() const
{
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry( "Number of Actions", m_myActions.count() );

    int i = 0;
    QString group;
    foreach (ClipAction* action, m_myActions) {
        group = QString("Action_%1").arg( i );
        action->save( KGlobal::config(), group );
        ++i;
    }

    KlipperSettings::setNoActionsForWM_CLASS(m_myAvoidWindows);
}

// find out whether the active window's WM_CLASS is in our avoid-list
// digged a little bit in netwm.cpp
bool URLGrabber::isAvoidedWindow() const
{
    Display *d = QX11Info::display();
    static Atom wm_class = XInternAtom( d, "WM_CLASS", true );
    static Atom active_window = XInternAtom( d, "_NET_ACTIVE_WINDOW", true );
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, unused;
    unsigned char *data_ret;
    long BUFSIZE = 2048;
    bool ret = false;
    Window active = 0L;
    QString wmClass;

    // get the active window
    if (XGetWindowProperty(d, DefaultRootWindow( d ), active_window, 0l, 1l,
                           False, XA_WINDOW, &type_ret, &format_ret,
                           &nitems_ret, &unused, &data_ret)
        == Success) {
        if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
            active = *((Window *) data_ret);
        }
        XFree(data_ret);
    }
    if ( !active )
        return false;

    // get the class of the active window
    if ( XGetWindowProperty(d, active, wm_class, 0L, BUFSIZE, False, XA_STRING,
                            &type_ret, &format_ret, &nitems_ret,
                            &unused, &data_ret ) == Success) {
        if ( type_ret == XA_STRING && format_ret == 8 && nitems_ret > 0 ) {
            wmClass = QString::fromUtf8( (const char *) data_ret );
            ret = (m_myAvoidWindows.indexOf( wmClass ) != -1);
        }

        XFree( data_ret );
    }

    return ret;
}


void URLGrabber::slotKillPopupMenu()
{
    if ( m_myMenu && m_myMenu->isVisible() )
    {
        if ( m_myMenu->geometry().contains( QCursor::pos() ) &&
             m_myPopupKillTimeout > 0 )
        {
            m_myPopupKillTimer->start( 1000 * m_myPopupKillTimeout );
            return;
        }
    }

    if ( m_myMenu ) {
        m_myMenu->deleteLater();
        m_myMenu = 0;
    }
}

///////////////////////////////////////////////////////////////////////////
////////

ClipCommand::ClipCommand(const QString &_command, const QString &_description,
                         bool _isEnabled, const QString &_icon)
    : command(_command),
      description(_description),
      isEnabled(_isEnabled)
{

    if (!_icon.isEmpty())
        pixmap = _icon;
    else
    {
        // try to find suitable icon
        QString appName = command.section( ' ', 0, 0 );
        if ( !appName.isEmpty() )
        {
            QPixmap iconPix = KIconLoader::global()->loadIcon(
                                         appName, KIconLoader::Small, 0,
                                         KIconLoader::DefaultState,
                                         QStringList(), 0, true /* canReturnNull */ );
            if ( !iconPix.isNull() )
                pixmap = appName;
            else
                pixmap.clear();
        }
    }
}


ClipAction::ClipAction( const QString& regExp, const QString& description )
    : m_myRegExp( regExp ), m_myDescription( description )
{
}

ClipAction::ClipAction( KSharedConfigPtr kc, const QString& group )
    : m_myRegExp( kc->group(group).readEntry("Regexp") ),
      m_myDescription (kc->group(group).readEntry("Description") )
{
    KConfigGroup cg(kc, group);

    int num = cg.readEntry( "Number of commands", 0 );

    // read the commands
    for ( int i = 0; i < num; i++ ) {
        QString _group = group + "/Command_%1";
        KConfigGroup _cg(kc, _group.arg(i));

        addCommand( _cg.readPathEntry( "Commandline", QString() ),
                    _cg.readEntry( "Description" ), // i18n'ed
                    _cg.readEntry( "Enabled" , false),
                    _cg.readEntry( "Icon") );
    }
}


ClipAction::~ClipAction()
{
    m_myCommands.clear();
}


void ClipAction::addCommand( const QString& command,
                             const QString& description, bool enabled, const QString& icon )
{
    if ( command.isEmpty() )
        return;

    m_myCommands.append( ClipCommand(command, description, enabled, icon) );
}

void ClipAction::replaceCommand( int idx, const ClipCommand& cmd )
{
    if ( idx < 0 || idx >= m_myCommands.count() ) {
        kDebug() << "wrong command index given";
        return;
    }

    m_myCommands[idx] = cmd;
}


// precondition: we're in the correct action's group of the KConfig object
void ClipAction::save( KSharedConfigPtr kc, const QString& group ) const
{
    KConfigGroup cg(kc, group);
    cg.writeEntry( "Description", description() );
    cg.writeEntry( "Regexp", regExp() );
    cg.writeEntry( "Number of commands", m_myCommands.count() );

    int i=0;
    // now iterate over all commands of this action
    foreach (const ClipCommand& cmd, m_myCommands) {
        QString _group = group + "/Command_%1";
        KConfigGroup cg(kc, _group.arg(i));

        cg.writePathEntry( "Commandline", cmd.command );
        cg.writeEntry( "Description", cmd.description );
        cg.writeEntry( "Enabled", cmd.isEnabled );
        cg.writeEntry( "Icon", cmd.pixmap );

        ++i;
    }
}

#include "urlgrabber.moc"
