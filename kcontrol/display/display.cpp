/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "display.h"

#include <kcmoduleloader.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <qapplication.h>
#include <qlayout.h>
#include <qtabwidget.h>

typedef KGenericFactory<KCMDisplay, QWidget> DisplayFactory;
K_EXPORT_COMPONENT_FACTORY ( kcm_display, DisplayFactory( "display" ) )

KCMDisplay::KCMDisplay( QWidget *parent, const char *name, const QStringList& )
    : KCModule( parent, name ),
      m_randr( 0 ), m_gamma( 0 ), m_xiner( 0 ), m_energy( 0 )
{
  m_tabs = new QTabWidget( this );

  m_randr = addTab( "randr", i18n( "Size && Orientation" ) );
  m_gamma = addTab( "kgamma", i18n( "Monitor Gamma" ) );
  if ( QApplication::desktop()->isVirtualDesktop() )
    m_xiner = addTab( "xinerama", i18n( "Multiple Monitors" ) );
  m_energy = addTab( "energy", i18n( "Power Control" ) );

  QVBoxLayout *top = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  top->addWidget( m_tabs );

  setButtons( Apply|Help );
  load();
}

KCModule* KCMDisplay::addTab( const QString &name, const QString &label )
{
  QWidget *page = new QWidget( m_tabs, name.latin1() );
  QVBoxLayout *top = new QVBoxLayout( page, KDialog::marginHint() );

  KCModule *kcm = KCModuleLoader::loadModule( name, page );

  if ( kcm )
  {
    top->addWidget( kcm );
    m_tabs->addTab( page, label );

    connect( kcm, SIGNAL( changed(bool) ), SIGNAL( changed(bool) ) );
  }
  else
    delete page;

  return kcm;
}

void KCMDisplay::load()
{
  if ( m_randr )
    m_randr->load();
  if ( m_gamma )
    m_gamma->load();
  if ( m_xiner )
    m_xiner->load();
  if ( m_energy )
    m_energy->load();
}

void KCMDisplay::save()
{
  if ( m_randr )
    m_randr->save();
  if ( m_gamma )
    m_gamma->save();
  if ( m_xiner )
    m_xiner->save();
  if ( m_energy )
    m_energy->save();
}

