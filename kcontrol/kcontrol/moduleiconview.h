/*
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
*/                                                                            

#ifndef __moduleiconview_h__
#define __moduleiconview_h__

#include <klistview.h>

class ConfigModule;
class ConfigModuleList;

class ModuleIconItem : public KListViewItem
{

public:
  ModuleIconItem(QListView *parent, const QString& text, const QPixmap& pm, ConfigModule *m = 0)
	: KListViewItem(parent, text)
	, _tag(QString::null)
	, _module(m)
	{
	  setPixmap(0, pm);
	}

  void setConfigModule(ConfigModule* m) { _module = m; }
  void setTag(const QString& t) { _tag = t; }
  void setOrderNo(int order) {  setText(1, QString::number(order)); };
  ConfigModule* module() { return _module; }
  QString tag() { return _tag; }


private:
  QString       _tag;
  ConfigModule *_module;
};

class ModuleIconView : public KListView
{
  Q_OBJECT

public:
  ModuleIconView(ConfigModuleList *list, QWidget * parent = 0, const char * name = 0);
  
  void makeSelected(ConfigModule* module);
  void makeVisible(ConfigModule *module);
  void fill();

signals:
  void moduleSelected(ConfigModule*);

protected slots:
  void slotItemSelected(QListViewItem*);

protected:
 QDragObject *dragObject(); 
 void keyPressEvent(QKeyEvent *);
  
private:
  QString           _path; 
  ConfigModuleList *_modules;

};



#endif
