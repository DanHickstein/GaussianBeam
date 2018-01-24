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

#include "src/GaussianBeam.h"
#include "src/OpticsBench.h"
#include "src/GaussianFit.h"

#include "gui/GaussianBeamWidget.h"
#include "gui/GaussianBeamWindow.h"
#include "gui/Unit.h"

#include <QApplication>
#include <QPushButton>
#include <QStandardItemModel>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
#include <QSettings>

#include <cmath>

GaussianBeamWidget::GaussianBeamWidget(OpticsBench* bench, GaussianBeamWindow* window)
	: QWidget(window)
	, m_window(window)
{
	m_updatingFit = false;
	m_updatingTarget = false;
	setupUi(this);

	m_bench = bench;
	m_bench->registerEventListener(this);

	//toolBox->setSizeHint(100);
	//toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));

	// Extra widgets, not included in designer
#ifdef GBPLOT
/*	plot = new GaussianBeamPlot(this, model);
	splitter->addWidget(plot);
	checkBox_ShowGraph->setVisible(true);
	checkBox_ShowGraph->setEnabled(true);
	//checkBox_ShowGraph->setChecked(true);
	plot->setVisible(checkBox_ShowGraph->isChecked());*/
#else
/*	checkBox_ShowGraph->setVisible(false);
	checkBox_ShowGraph->setEnabled(false);*/
#endif

	// Waist fit
	fitModel = new QStandardItemModel(0, 3, this);
	fitTable->setModel(fitModel);
	fitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	fitTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	fitSelectionModel = new QItemSelectionModel(fitModel);
	fitTable->setSelectionModel(fitSelectionModel);
	fitTable->setColumnWidth(0, 82);
	fitTable->setColumnWidth(1, 82);

	// Connect slots
	connect(fitModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(fitModelChanged(const QModelIndex&, const QModelIndex&)));

	// Set up default values
	on_checkBox_ShowTargetBeam_toggled(checkBox_ShowTargetBeam->isChecked());
	updateUnits();
	readSettings();

	// Sync with bench
	onOpticsBenchTargetBeamChanged();
	onOpticsBenchWavelengthChanged();
	onOpticsBenchBoundariesChanged();
	for (int i = 0; i < m_bench->nFit(); i++)
		onOpticsBenchFitAdded(i);

	updateFitInformation(comboBox_Fit->currentIndex());
	updateTargetInformation();
}

GaussianBeamWidget::~GaussianBeamWidget()
{
	writeSettings();
}

void GaussianBeamWidget::writeSettings()
{
	QSettings settings;
	settings.setValue("GaussianBeamWidget/toolboxIndex", toolBox->currentIndex());
}

void GaussianBeamWidget::readSettings()
{
	QSettings settings;
	toolBox->setCurrentIndex(settings.value("GaussianBeamWidget/toolboxIndex", 0).toInt());
}

void GaussianBeamWidget::updateUnits()
{
	doubleSpinBox_HTargetWaist->setSuffix(Unit(UnitWaist).string());
	doubleSpinBox_HTargetPosition->setSuffix(Unit(UnitPosition).string());
	doubleSpinBox_VTargetWaist->setSuffix(Unit(UnitWaist).string());
	doubleSpinBox_VTargetPosition->setSuffix(Unit(UnitPosition).string());
	doubleSpinBox_LeftBoundary->setSuffix(Unit(UnitPosition).string());
	doubleSpinBox_RightBoundary->setSuffix(Unit(UnitPosition).string());
	/// @todo update table headers and status bar and wavelength
}

void GaussianBeamWidget::onOpticsBenchDataChanged(int /*startOptics*/, int /*endOptics*/)
{
	displayOverlap();
}

void GaussianBeamWidget::onOpticsBenchWavelengthChanged()
{
	displayOverlap();
	updateFitInformation(comboBox_Fit->currentIndex());
}

///////////////////////////////////////////////////////////
// OPTICS BENCH PAGE

void GaussianBeamWidget::on_doubleSpinBox_LeftBoundary_valueChanged(double value)
{
	m_bench->setLeftBoundary(value*Unit(UnitPosition).multiplier());
}

void GaussianBeamWidget::on_doubleSpinBox_RightBoundary_valueChanged(double value)
{
	m_bench->setRightBoundary(value*Unit(UnitPosition).multiplier());
}

void GaussianBeamWidget::onOpticsBenchBoundariesChanged()
{
	doubleSpinBox_LeftBoundary->setValue(m_bench->leftBoundary()*Unit(UnitPosition).divider());
	doubleSpinBox_RightBoundary->setValue(m_bench->rightBoundary()*Unit(UnitPosition).divider());
}

///////////////////////////////////////////////////////////
// CAVITY PAGE

void GaussianBeamWidget::updateCavityInformation()
{

}

///////////////////////////////////////////////////////////
// MAGIC WAIST PAGE

void GaussianBeamWidget::displayOverlap()
{
	if (m_bench->nOptics() > 0)
	{
		double overlap = Beam::overlap(*m_bench->beam(m_bench->nOptics()-1), *m_bench->targetBeam());
		label_OverlapResult->setText(tr("Overlap: ") + QString::number(overlap*100., 'f', 2) + " %");
	}
	else
		label_OverlapResult->setText("");
}

void GaussianBeamWidget::updateTargetInformation()
{
	m_updatingTarget = true;

	if (m_bench->targetOrientation() == Ellipsoidal)
	{
		comboBox_TargetOrientation->setCurrentIndex(1);
		doubleSpinBox_VTargetWaist->setVisible(true);
		doubleSpinBox_VTargetPosition->setVisible(true);
	}
	else
	{
		comboBox_TargetOrientation->setCurrentIndex(0);
		doubleSpinBox_VTargetWaist->setVisible(false);
		doubleSpinBox_VTargetPosition->setVisible(false);
	}

	doubleSpinBox_HTargetWaist->setValue(m_bench->targetBeam()->waist(Horizontal)*Unit(UnitWaist).divider());
	doubleSpinBox_HTargetPosition->setValue(m_bench->targetBeam()->waistPosition(Horizontal)*Unit(UnitPosition).divider());
	doubleSpinBox_VTargetWaist->setValue(m_bench->targetBeam()->waist(Vertical)*Unit(UnitWaist).divider());
	doubleSpinBox_VTargetPosition->setValue(m_bench->targetBeam()->waistPosition(Vertical)*Unit(UnitPosition).divider());
	doubleSpinBox_MinOverlap->setValue(m_bench->targetOverlap()*100.);
	displayOverlap();

	m_updatingTarget = false;
}

void GaussianBeamWidget::on_comboBox_TargetOrientation_currentIndexChanged(int dataIndex)
{
	if (m_updatingTarget)
		return;

	if (dataIndex == 0)
		m_bench->setTargetOrientation(Spherical);
	else
		m_bench->setTargetOrientation(Ellipsoidal);
}

void GaussianBeamWidget::on_doubleSpinBox_HTargetWaist_valueChanged(double value)
{
	if (m_updatingTarget)
		return;

	Orientation orientation = Horizontal;
	if (m_bench->targetOrientation() == Spherical)
		orientation = Spherical;

	Beam beam = *m_bench->targetBeam();
	beam.setWaist(value*Unit(UnitWaist).multiplier(), orientation);
	m_bench->setTargetBeam(beam);
}

void GaussianBeamWidget::on_doubleSpinBox_HTargetPosition_valueChanged(double value)
{
	if (m_updatingTarget)
		return;

	Orientation orientation = Horizontal;
	if (m_bench->targetOrientation() == Spherical)
		orientation = Spherical;

	Beam beam = *m_bench->targetBeam();
	beam.setWaistPosition(value*Unit(UnitPosition).multiplier(), orientation);
	m_bench->setTargetBeam(beam);
}

void GaussianBeamWidget::on_doubleSpinBox_VTargetWaist_valueChanged(double value)
{
	if (m_updatingTarget)
		return;

	Beam beam = *m_bench->targetBeam();
	beam.setWaist(value*Unit(UnitWaist).multiplier(), Vertical);
	m_bench->setTargetBeam(beam);
}

void GaussianBeamWidget::on_doubleSpinBox_VTargetPosition_valueChanged(double value)
{
	if (m_updatingTarget)
		return;

	Beam beam = *m_bench->targetBeam();
	beam.setWaistPosition(value*Unit(UnitPosition).multiplier(), Vertical);
	m_bench->setTargetBeam(beam);
}

void GaussianBeamWidget::on_doubleSpinBox_MinOverlap_valueChanged(double value)
{
	if (m_updatingTarget)
		return;

	m_bench->setTargetOverlap(value*0.01);
}

void GaussianBeamWidget::on_checkBox_ShowTargetBeam_toggled(bool checked)
{
	if (m_updatingTarget)
		return;

	m_window->showTargetBeam(checked);
}

void GaussianBeamWidget::on_pushButton_MagicWaist_clicked()
{
	label_MagicWaistResult->setText("");

	if (!m_bench->magicWaist())
		label_MagicWaistResult->setText(tr("Desired waist could not be found !"));
	else
		displayOverlap();
}

void GaussianBeamWidget::on_pushButton_LocalOptimum_clicked()
{
	if (!m_bench->localOptimum())
		label_MagicWaistResult->setText(tr("Local optimum not found !"));
	else
		displayOverlap();
}

void GaussianBeamWidget::onOpticsBenchTargetBeamChanged()
{
	updateTargetInformation();
}

///////////////////////////////////////////////////////////
// FIT PAGE

QVariant GaussianBeamWidget::formattedFitData(double value)
{
	if (value != 0.)
		return value*Unit(UnitWaist).divider();

	return QString("");
}

void GaussianBeamWidget::updateFitInformation(int index)
{
	if (index >= m_bench->nFit())
		return;

	// Enable or disable widgets
	if (index < 0)
	{
		pushButton_RemoveFit->setEnabled(false);
		pushButton_RenameFit->setEnabled(false);
		pushButton_FitColor->setEnabled(false);
		pushButton_FitAddRow->setEnabled(false);
		pushButton_FitRemoveRow->setEnabled(false);
		comboBox_FitData->setEnabled(false);
		comboBox_FitOrientation->setEnabled(false);
		fitTable->setEnabled(false);

		pushButton_SetInputBeam->setEnabled(false);
		pushButton_SetTargetBeam->setEnabled(false);
		return;
	}
	else
	{
		pushButton_RemoveFit->setEnabled(true);
		pushButton_RenameFit->setEnabled(true);
		pushButton_FitColor->setEnabled(true);
		pushButton_FitAddRow->setEnabled(true);
		pushButton_FitRemoveRow->setEnabled(true);
		comboBox_FitData->setEnabled(true);
		comboBox_FitOrientation->setEnabled(true);
		fitTable->setEnabled(true);
	}

	m_updatingFit = true;
	// Fill widgets
	Fit* fit = m_bench->fit(index);
	comboBox_Fit->setItemText(index, QString::fromUtf8(fit->name().c_str()));
	pushButton_FitColor->setPalette(QPalette(QColor(fit->color())));
	comboBox_FitData->setCurrentIndex(int(fit->dataType()));
	comboBox_FitOrientation->setCurrentIndex(int(fit->orientation()));

	// Resize rows to match the number of elements in the fit
	while (fit->size() > fitModel->rowCount())
		fitModel->insertRow(0);
	while (fitModel->rowCount() > fit->size())
		fitModel->removeRow(0);

	// Show/hide fit columns
	fitTable->showColumn(1);
	fitTable->showColumn(2);
	if ((fit->orientation() == Horizontal) || (fit->orientation() == Spherical))
		fitTable->hideColumn(2);
	else if (fit->orientation() == Vertical)
		fitTable->hideColumn(1);

	QString header = (fit->orientation() == Spherical) ? tr("Value") : tr("Horizontal\nvalue");
	fitModel->setHeaderData(0, Qt::Horizontal, tr("Position") + "\n(" + Unit(UnitPosition).string(false) + ")");
	fitModel->setHeaderData(1, Qt::Horizontal, header + "\n(" + Unit(UnitWaist).string(false) + ")");
	fitModel->setHeaderData(2, Qt::Horizontal, tr("Vertical\nvalue") + "\n(" + Unit(UnitWaist).string(false) + ")");

	// Fill the fit table
	for (int i = 0; i < fit->size(); i++)
	{
		if ((fit->position(i) != 0.) || fit->nonZeroEntry(i))
			fitModel->setData(fitModel->index(i, 0), fit->position(i)*Unit(UnitPosition).divider());
		else
			fitModel->setData(fitModel->index(i, 0), QString(""));

		if (fit->orientation() != Vertical)
			fitModel->setData(fitModel->index(i, 1), formattedFitData(fit->value(i, Horizontal)));
		if (fit->orientation() != Horizontal)
			fitModel->setData(fitModel->index(i, 2), formattedFitData(fit->value(i, Vertical)));
	}

	bool fh = fit->fitAvailable(Horizontal);
	bool fv = (fit->fitAvailable(Vertical) & (fit->orientation() != Spherical));
	if (fh | fv)
	{
		Beam fitBeam(m_bench->wavelength());
		double residue = fit->applyFit(fitBeam);
		QString text;
		if (fh)
		{
			text += (fv ? tr("Horizonal waist") : tr("Waist")) + " = " + QString::number(fitBeam.waist(Horizontal)*Unit(UnitWaist).divider()) + Unit(UnitWaist).string() + "\n" +
					(fv ? tr("Horizonal position") : tr("Position")) + " = " + QString::number(fitBeam.waistPosition(Horizontal)*Unit(UnitPosition).divider()) + Unit(UnitPosition).string() + "\n";
		}
		if (fv)
		{
			text += (fh ? tr("Vertical waist") : tr("Waist")) + " = " + QString::number(fitBeam.waist(Vertical)*Unit(UnitWaist).divider()) + Unit(UnitWaist).string() + "\n" +
					(fh ? tr("Vertical position") : tr("Position")) + " = " + QString::number(fitBeam.waistPosition(Vertical)*Unit(UnitPosition).divider()) + Unit(UnitPosition).string() + "\n";
		}
		text += tr("Residue") + " = " + QString::number(residue);
		label_FitResult->setText(text);
		pushButton_SetInputBeam->setEnabled(true);
		pushButton_SetTargetBeam->setEnabled(true);
	}
	else
	{
		pushButton_SetInputBeam->setEnabled(false);
		pushButton_SetTargetBeam->setEnabled(false);
		label_FitResult->setText(QString());
	}

	m_updatingFit = false;
}

// Control callback

void GaussianBeamWidget::on_comboBox_Fit_currentIndexChanged(int index)
{
	updateFitInformation(index);
}

void GaussianBeamWidget::on_pushButton_AddFit_clicked()
{
	int index = m_bench->nFit();
	m_bench->addFit(index, 3);
}

void GaussianBeamWidget::on_pushButton_RemoveFit_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	m_bench->removeFit(index);
}

void GaussianBeamWidget::on_pushButton_RenameFit_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	bool ok;
	QString text = QInputDialog::getText(this, tr("Rename fit"),
		tr("Enter a new name for the current fit:"), QLineEdit::Normal,
		m_bench->fit(index)->name().c_str(), &ok);
	if (ok && !text.isEmpty())
		m_bench->fit(index)->setName(text.toUtf8().data());
}

void GaussianBeamWidget::on_pushButton_FitColor_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	QColor color = QColorDialog::getColor(Qt::black, this);
	m_bench->fit(index)->setColor(color.rgb());
}

void GaussianBeamWidget::on_comboBox_FitOrientation_currentIndexChanged(int dataIndex)
{
	int index = comboBox_Fit->currentIndex();
	if ((index < 0) || m_updatingFit)
		return;

	Fit* fit = m_bench->fit(index);
	fit->setOrientation(Orientation(dataIndex));
}

void GaussianBeamWidget::on_comboBox_FitData_currentIndexChanged(int dataIndex)
{
	int index = comboBox_Fit->currentIndex();
	if ((index < 0) || m_updatingFit)
		return;

	Fit* fit = m_bench->fit(index);
	fit->setDataType(FitDataType(dataIndex));
}

void GaussianBeamWidget::fitModelChanged(const QModelIndex& start, const QModelIndex& stop)
{
	int index = comboBox_Fit->currentIndex();
	if ((index < 0) || m_updatingFit)
		return;

	Fit* fit = m_bench->fit(index);

	for (int row = start.row(); row <= stop.row(); row++)
	{
		double position = fitModel->data(fitModel->index(row, 0)).toDouble()*Unit(UnitPosition).multiplier();
		double hValue = fitModel->data(fitModel->index(row, 1)).toDouble()*Unit(UnitWaist).multiplier();
		double vValue = fitModel->data(fitModel->index(row, 2)).toDouble()*Unit(UnitWaist).multiplier();
		if (fit->orientation() == Spherical)
			fit->setData(row, position, hValue, Spherical);
		else
		{
			if (fit->orientation() != Vertical)
				fit->setData(row, position, hValue, Horizontal);
			if (fit->orientation() != Horizontal)
				fit->setData(row, position, vValue, Vertical);
		}
	}
}

void GaussianBeamWidget::on_pushButton_FitAddRow_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	m_bench->fit(index)->addData(0., 0., Spherical);
}

void GaussianBeamWidget::on_pushButton_FitRemoveRow_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	QList<int> removedRows;

	for (int row = fitModel->rowCount() - 1; row >= 0; row--)
		if (fitSelectionModel->isRowSelected(row, QModelIndex()) && (row < m_bench->fit(index)->size()))
			removedRows << row;

	foreach (int row, removedRows)
		m_bench->fit(index)->removeData(row);
}

void GaussianBeamWidget::on_pushButton_SetInputBeam_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	Beam inputBeam = *m_bench->inputBeam();
	m_bench->fit(index)->applyFit(inputBeam);
	m_bench->setInputBeam(inputBeam);
}

void GaussianBeamWidget::on_pushButton_SetTargetBeam_clicked()
{
	int index = comboBox_Fit->currentIndex();
	if (index < 0)
		return;

	Beam targetBeam = *m_bench->targetBeam();
	m_bench->fit(index)->applyFit(targetBeam);
	m_bench->setTargetBeam(targetBeam);
}

void GaussianBeamWidget::displayShowTargetBeam(bool show)
{
	checkBox_ShowTargetBeam->setCheckState(show ? Qt::Checked : Qt::Unchecked);
}

// OpticsBench callbacks

void GaussianBeamWidget::onOpticsBenchFitAdded(int index)
{
	comboBox_Fit->insertItem(index, QString::fromUtf8(m_bench->fit(index)->name().c_str()));
	comboBox_Fit->setCurrentIndex(index);
}

void GaussianBeamWidget::onOpticsBenchFitsRemoved(int index, int count)
{
	for (int i = index + count - 1; i >= index; i--)
		comboBox_Fit->removeItem(i);
}

void GaussianBeamWidget::onOpticsBenchFitDataChanged(int index)
{
	int comboIndex = comboBox_Fit->currentIndex();
	if ((comboIndex < 0) || (comboIndex != index))
		return;

	updateFitInformation(index);
}
