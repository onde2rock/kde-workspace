/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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

#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__

#include <kmainwindow.h>

class QTabWidget;
class QSplitter;

class KToggleAction;
class KAction;

class DockContainer;
class IndexWidget;
class SearchWidget;
class HelpWidget;
class ConfigModule;
class ConfigModuleList;

class TopLevel : public KMainWindow
{
  Q_OBJECT

public:
  TopLevel( const char* name=0 );
  ~TopLevel();

  void showModule(QString desktopFile);

protected:
  void setupActions();

protected slots:
  void activateModule(const QString& name);
  void moduleActivated(ConfigModule *module);
  void newModule(const QString &name, const QString& docPath, const QString &quickhelp);
  void activateIconView();
  void activateTreeView();

  void reportBug();

  void activateSmallIcons();
  void activateMediumIcons();
  void activateLargeIcons();

  void deleteDummyAbout();
    
  void slotHelpRequest();

private:
  QTabWidget     *_tab;
  DockContainer  *_dock;
  QSplitter      *_splitter;

  KToggleAction *tree_view, *icon_view;
  KToggleAction *icon_small, *icon_medium, *icon_large;
  KAction *report_bug;

  IndexWidget  *_indextab;
  SearchWidget *_searchtab;
  HelpWidget   *_helptab;

  ConfigModule     *_active;
  ConfigModuleList *_modules;

  /**
   * if someone wants to report a bug
   * against a module with no about data
   * we construct one for him
   **/
  KAboutData *dummyAbout;
};

#endif
