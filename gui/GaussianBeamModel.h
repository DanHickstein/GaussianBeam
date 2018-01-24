/* This file is part of the GaussianBeam project
   Copyright (C) 2007-2010 Jérôme Lodewyck <jerome dot lodewyck at normalesup.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef GAUSSIANBEAMMODEL_H
#define GAUSSIANBEAMMODEL_H

#include "src/Optics.h"
#include "src/OpticsBench.h"

#include <QAbstractTableModel>
#include <QList>

class TablePropertySelector;
class OpticsBench;

class GaussianBeamModel : public QAbstractTableModel, protected OpticsBenchEventListener
{
Q_OBJECT

public:
	GaussianBeamModel(OpticsBench* bench, TablePropertySelector* propertySelector, QObject* parent = 0);
	~GaussianBeamModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
	Property::Type columnContent(int column) const { return m_columns[column]; }

private slots:
	void propertyWidgetModified();

protected:
	virtual void onOpticsBenchDataChanged(int startOptics, int endOptics);
	virtual void onOpticsBenchOpticsAdded(int index);
	virtual void onOpticsBenchOpticsRemoved(int index, int count);

private:
	QList<Property::Type> m_columns;
	TablePropertySelector* m_propertySelector;
};

#endif
