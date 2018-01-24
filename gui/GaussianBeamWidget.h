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

#ifndef GAUSSIANBEAMWIDGET_H
#define GAUSSIANBEAMWIDGET_H

#include "src/Optics.h"
#include "src/OpticsBench.h"
#include "ui_GaussianBeamWidget.h"

#include <QWidget>

class QStandardItemModel;
class QDomElement;
class QAction;
class GaussianBeamWindow;

class GaussianBeamWidget : public QWidget,
                           private Ui::GaussianBeamWidget,
                           protected OpticsBenchEventListener
{
Q_OBJECT

public:
	GaussianBeamWidget(OpticsBench* bench, GaussianBeamWindow* window);
	~GaussianBeamWidget();

public:
	/// @todo transfer this logic to view properties
	void displayShowTargetBeam(bool show);

// OpticsBench notifications
protected:
	virtual void onOpticsBenchDataChanged(int startOptics, int endOptics);
	virtual void onOpticsBenchTargetBeamChanged();
	virtual void onOpticsBenchBoundariesChanged();
	virtual void onOpticsBenchFitAdded(int index);
	virtual void onOpticsBenchFitsRemoved(int index, int count);
	virtual void onOpticsBenchFitDataChanged(int index);
	virtual void onOpticsBenchWavelengthChanged();

// UI slots
protected slots:
	// Optics bench
	void on_doubleSpinBox_LeftBoundary_valueChanged(double value);
	void on_doubleSpinBox_RightBoundary_valueChanged(double value);
	// Magic waist
	void on_pushButton_MagicWaist_clicked();
	void on_pushButton_LocalOptimum_clicked();
	void on_checkBox_ShowTargetBeam_toggled(bool checked);
	void on_comboBox_TargetOrientation_currentIndexChanged(int dataIndex);
	void on_doubleSpinBox_HTargetWaist_valueChanged(double value);
	void on_doubleSpinBox_HTargetPosition_valueChanged(double value);
	void on_doubleSpinBox_VTargetWaist_valueChanged(double value);
	void on_doubleSpinBox_VTargetPosition_valueChanged(double value);
	void on_doubleSpinBox_MinOverlap_valueChanged(double value);
	// Waist fit
	void on_comboBox_Fit_currentIndexChanged(int index);
	void on_comboBox_FitOrientation_currentIndexChanged(int dataIndex);
	void on_comboBox_FitData_currentIndexChanged(int dataIndex);
	void on_pushButton_AddFit_clicked();
	void on_pushButton_RemoveFit_clicked();
	void on_pushButton_RenameFit_clicked();
	void on_pushButton_FitColor_clicked();
	void on_pushButton_SetInputBeam_clicked();
	void on_pushButton_SetTargetBeam_clicked();
	void on_pushButton_FitAddRow_clicked();
	void on_pushButton_FitRemoveRow_clicked();

private slots:
	void fitModelChanged(const QModelIndex& start = QModelIndex(), const QModelIndex& stop = QModelIndex());

private:
	void displayOverlap();
	void updateUnits();
	void insertOptics(OpticsType opticsType);
	void updateTargetInformation();
	void updateCavityInformation();
	void updateFitInformation(int index);
	void readSettings();
	void writeSettings();
	QVariant formattedFitData(double value);

private:
	GaussianBeamWindow* m_window;

	QStandardItemModel* fitModel;
	QItemSelectionModel* fitSelectionModel;

	bool m_updatingFit, m_updatingTarget;
};

#endif
