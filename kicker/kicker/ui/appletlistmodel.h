/**
  * This file is part of the KDE project
  * Copyright (C) 2006 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef APPLETLISTMODEL_H
#define APPLETLISTMODEL_H

#include <QAbstractListModel>
#include <kicon.h>

#include "appletinfo.h"

class AppletItemDelegate;

class AppletListModel
	: public QAbstractListModel
{
	Q_OBJECT

public:
	/**
	  * @brief Constructor for the model of an applet list.
	  */
	AppletListModel(const AppletInfo::List &appletInfoList, QObject *parent = 0);

	/**
	  * @brief Destructor for the model of an applet list.
	  */
	~AppletListModel();

	/**
	  * @brief Returns data from the data structure.
	  */
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	/**
	  * @brief Get the children model index for the given row.
	  */
	QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

	/**
	  * @brief Get the number of applets added to the list.
	  */
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
	class Private;
	Private *d;
};

#endif
