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

#ifndef GAUSSIANBEAMDELEGATE_H
#define GAUSSIANBEAMDELEGATE_H

#include "src/OpticsBench.h"

#include <QItemDelegate>
#include <QModelIndex>
#include <QWidget>
#include <QDoubleSpinBox>

class GaussianBeamModel;

class EditorProperty
{
public:
	EditorProperty(double a_minimum, double a_maximum, QString a_prefix = "", QString a_suffix = "")
		: minimum(a_minimum), maximum(a_maximum), prefix(a_prefix), suffix(a_suffix) {}

public:
	double minimum;
	double maximum;
	QString prefix;
	QString suffix;
};

class PropertyEditor : public QWidget
{
Q_OBJECT

public:
	PropertyEditor(QList<EditorProperty>& properties, QWidget* parent = 0);

public:
	double value(int index) const { return m_doubleSpinBoxes[index]->value(); }
	QList<QVariant> values() const;
	void setValue(int index, double value) { m_doubleSpinBoxes[index]->setValue(value); }

private:
	QList<QDoubleSpinBox*> m_doubleSpinBoxes;
};

class GaussianBeamDelegate : public QItemDelegate
{
Q_OBJECT

public:
	GaussianBeamDelegate(QObject* parent, GaussianBeamModel* model, OpticsBench* bench);

public:
	QWidget *createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	GaussianBeamModel* m_model;
	const OpticsBench* m_bench;
};

#endif
