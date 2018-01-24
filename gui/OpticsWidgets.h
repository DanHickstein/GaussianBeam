/* This file is part of the GaussianBeam project
   Copyright (C) 2008-2010 Jérôme Lodewyck <jerome dot lodewyck at normalesup.org>

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

#ifndef OPTICSWIDGETS_H
#define OPTICSWIDGETS_H

#include "src/GaussianBeam.h"
#include "ui_OpticsViewProperties.h"

#include <QScrollBar>
#include <QModelIndex>

class OpticsView;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QStatusBar;

/**
* Displays graduated rullers instead of scroll bars
*/
class GraduatedRuller : public QWidget
{
Q_OBJECT

public:
	GraduatedRuller(OpticsView* view, Qt::Orientation orientation);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
//	virtual void mousePressEvent(QMouseEvent* event);

private:
	double optimalSpacing(double length, int nMajorTics) const;

private:
	OpticsView* m_view;
	Qt::Orientation m_orientation;
	double m_height;
};

/**
* Widget used to tune the range and offset of a view
*/
class OpticsViewProperties : public QWidget, private Ui::OpticsViewProperties
{
Q_OBJECT

public:
	OpticsViewProperties(OpticsView* view);

public:
	void setOrigin(QPointF origin);
	void setHorizontalRange(double horizontalRange);
	void setBeamScale(double beamScale);
	void setOpticsHeight(double opticsHeight);
	void setLock(bool locked);
	void setLockVisible(bool visible);

// UI slots
protected slots:
	void on_toolButton_ZoomIn_clicked(bool checked);
	void on_toolButton_ZoomOut_clicked(bool checked);
	void on_toolButton_ZoomFull_clicked(bool checked);
	void on_toolButton_Lock_toggled(bool checked);
	void on_doubleSpinBox_Origin_valueChanged(double value);
	void on_doubleSpinBox_HorizontalRange_valueChanged(double value);
	void on_doubleSpinBox_BeamScale_valueChanged(double value);
	void on_doubleSpinBox_OpticsHeight_valueChanged(double value);

private:
	OpticsView* m_view;
	bool m_update;
};

/**
* Widget that displays a small icon used to trigger a widget
*/
class CornerWidget : public QWidget
{
Q_OBJECT

public:
	CornerWidget(QColor backgroundColor, const char*  pixmapName, QWidget* widget, QWidget* parent = 0);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);

private:
	QColor m_backgroundColor;
	QPixmap m_pixmap;
	QWidget* m_widget;
};

/**
* Widget with a selectable and sortable list of properties
*/
class PropertySelector : public QWidget
{
Q_OBJECT

public:
	PropertySelector(QWidget* parent = 0);
	~PropertySelector();

public:
	bool showFullName() const;
	QList<Property::Type> checkedItems() const;

signals:
	void propertyChanged();

protected:
	void readSettings(QList<Property::Type> propertyList, QList<bool> checkList);

private:
	void writeSettings() const;

protected:
	QString m_settingsKey;

private:
	QListWidget* m_propertyListWidget;
	QCheckBox* m_symbolCheck;

private slots:
	void checkBoxModified(int state);
	void itemModified(QListWidgetItem* item);
};

/**
* Property selector for status bar properties
*/
class StatusPropertySelector : public PropertySelector
{
Q_OBJECT

public:
	StatusPropertySelector(QWidget* parent = 0);
};

/**
* Property selector for table columns
*/
class TablePropertySelector : public PropertySelector
{
Q_OBJECT

public:
	TablePropertySelector(QWidget* parent = 0);
};

/**
* Optics view status bar widget
*/
class StatusWidget : public QWidget
{
Q_OBJECT

public:
	StatusWidget(QStatusBar* statusBar);

public:
	void showBeamInfo(const Beam* beam, double z, Orientation orientation);

private:
	QStatusBar* m_statusBar;
	QLabel* m_label;
	StatusPropertySelector* m_configWidget;
};

#endif
