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

#include "gui/Names.h"
#include "gui/OpticsWidgets.h"
#include "gui/OpticsView.h"
#include "gui/Unit.h"

#include <QWheelEvent>
#include <QListWidget>
#include <QCheckBox>
#include <QSettings>
#include <QDebug>

/////////////////////////////////////////////////
// RullerSlider

GraduatedRuller::GraduatedRuller(OpticsView* view, Qt::Orientation orientation)
{
	m_height = 16.;
	m_view = view;
	m_orientation = orientation;

	if (m_orientation == Qt::Horizontal)
		setFixedHeight(int(m_height));
	else if (m_orientation == Qt::Vertical)
		setFixedWidth(int(m_height));

	connect(m_view, SIGNAL(rangeChanged()), this, SLOT(update()));
}

double GraduatedRuller::optimalSpacing(double length, int nMajorTics) const
{
	double result = 0.1/double(nMajorTics);

	while (length > 10.)
	{
		length /= 10.;
		result *= 10.;
	}

	while (length < 1.)
	{
		length *= 10.;
		result /= 10.;
	}

	if (length < 1.5)
		return result;
	else if (length < 3.)
		return 2.*result;
	else if (length < 7.)
		return 5.*result;
	else
		return 10.*result;
}

void GraduatedRuller::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QColor backgroundColor(245, 245, 200);
	QBrush backgroundBrush(backgroundColor);
	QPen backgroundPen(backgroundColor);
	QPen mainTickPen(Qt::black);
	QPen secondTickPen(Qt::lightGray);

	int length;
	double start, stop; // Edges of the ruller in scene coordinates
	double unit = Unit(UnitPosition).divider();
	if (m_orientation == Qt::Horizontal)
	{
		length = width();
		start = m_view->mapToScene(m_view->viewport()->rect().topLeft()).x();
		stop = m_view->mapToScene(m_view->viewport()->rect().bottomRight()).x();
	}
	else
	{
		length = height();
		painter.rotate(-90.);
		painter.translate(-height(), 0.);
		start = m_view->mapToScene(m_view->viewport()->rect().topLeft()).y();
		stop = m_view->mapToScene(m_view->viewport()->rect().bottomRight()).y();
		OpticsScene* scene = dynamic_cast<OpticsScene*>(m_view->scene());
		if (scene->bench()->is1D())
		{
			start /= scene->beamScale();
			stop  /= scene->beamScale();
			unit = Unit(UnitWaist).divider();
		}
	}
	if ((stop <= start) || (length == 0))
		return;
	double scale = double(length)/(stop - start); // pixels/(Scene Coordinates)

	// These 3 variables are in scene coordinates
	double spacing = optimalSpacing(stop - start, m_orientation == Qt::Horizontal ? 10 : 5);
	const double lastGraduation  = double(int(stop /spacing) + 1)*spacing;
	const double firstGraduation = double(int(start/spacing) - 1)*spacing;

	// Background
	painter.setBrush(backgroundBrush);
	painter.setPen(backgroundPen);
	painter.drawRect(QRectF(0., 0., length, m_height));

	for (double x = lastGraduation; x >= firstGraduation; x -= spacing)
	{
		painter.setPen(secondTickPen);
		double pos = (x - start)*scale;
		double gradHeight = 0.4;
		if (abs((int(round(x/spacing)) % 10)) == 5)
			gradHeight = 0.65;
		else if ((int(round(x/spacing)) % 10) == 0)
		{
			gradHeight = 0.8;
			painter.setPen(mainTickPen);
			QString text;
			text.setNum(round(x*unit));
			QRectF textRect(0., 0., 0., 0.);
			textRect.moveCenter(QPointF(pos + 2., m_height/2.));
			textRect = painter.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
			painter.drawText(textRect, Qt::AlignCenter, text);
		}
		painter.drawLine(QPointF(pos, 0.), QPointF(pos, m_height*gradHeight));
	}
}

void GraduatedRuller::wheelEvent(QWheelEvent* event)
{
	const int numDegrees = event->delta()/8;
	const double numSteps = double(numDegrees)/15.;

	if (m_orientation == Qt::Horizontal)
	{
		if (QApplication::keyboardModifiers() == Qt::ControlModifier)
			m_view->setHorizontalRange(m_view->horizontalRange()*pow(1.2, numSteps));
		else
			m_view->setOrigin(m_view->origin() + QPointF(m_view->horizontalRange()*numSteps	/10., 0.));
	}
	else if (m_orientation == Qt::Vertical)
	{
		OpticsScene* scene = dynamic_cast<OpticsScene*>(m_view->scene());
		scene->setBeamScale(scene->beamScale()*pow(1.2, numSteps));
		/// @todo scroll if 2D bench
	}
}

/////////////////////////////////////////////////
// OpticsViewProperties

OpticsViewProperties::OpticsViewProperties(OpticsView* view)
	: QWidget(view)
{
	m_view = view;
	setupUi(this);
	toolButton_Lock->setVisible(false);
	setAttribute(Qt::WA_DeleteOnClose);
	m_update = true;
}

void OpticsViewProperties::on_toolButton_ZoomIn_clicked(bool checked)
{
	Q_UNUSED(checked);

	/// @todo use scaleview

	if (m_update)
		m_view->zoom(1./1.2);
}

void OpticsViewProperties::on_toolButton_ZoomOut_clicked(bool checked)
{
	Q_UNUSED(checked);

	/// @todo use scaleview

	if (m_update)
		m_view->zoom(1.2);
}

void OpticsViewProperties::on_toolButton_ZoomFull_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (m_update)
		m_view->showFullBench();
}

void OpticsViewProperties::on_toolButton_Lock_toggled(bool checked)
{
	if (m_update)
		dynamic_cast<OpticsScene*>(m_view->scene())->setScenesLocked(checked);
}

void OpticsViewProperties::on_doubleSpinBox_Origin_valueChanged(double value)
{
	/// @todo vertical origin
	if (m_update)
	{
		QPointF origin = m_view->origin();
		origin.setX(value*Unit(UnitPosition).multiplier());
		m_view->setOrigin(origin);
	}
}

void OpticsViewProperties::on_doubleSpinBox_HorizontalRange_valueChanged(double value)
{
	if (m_update)
		m_view->setHorizontalRange(value*Unit(UnitPosition).multiplier());
}

void OpticsViewProperties::on_doubleSpinBox_BeamScale_valueChanged(double value)
{
	if (m_update)
		dynamic_cast<OpticsScene*>(m_view->scene())->setBeamScale(value);
}

void OpticsViewProperties::on_doubleSpinBox_OpticsHeight_valueChanged(double value)
{
	if (m_update)
		dynamic_cast<OpticsScene*>(m_view->scene())->setOpticsHeight(value*Unit(UnitPosition).multiplier());
}

void OpticsViewProperties::setOrigin(QPointF origin)
{
	/// @todo vertical origin
	m_update = false;
	doubleSpinBox_Origin->setValue(origin.x()*Unit(UnitPosition).divider());
	m_update = true;
}

void OpticsViewProperties::setHorizontalRange(double horizontalRange)
{
	m_update = false;
	doubleSpinBox_HorizontalRange->setValue(horizontalRange*Unit(UnitPosition).divider());
	m_update = true;
}

void OpticsViewProperties::setBeamScale(double beamScale)
{
	m_update = false;
	doubleSpinBox_BeamScale->setValue(beamScale);
	m_update = true;
}

void OpticsViewProperties::setOpticsHeight(double opticsHeight)
{
	m_update = false;
	doubleSpinBox_OpticsHeight->setValue(opticsHeight*Unit(UnitPosition).divider());
	m_update = true;
}

void OpticsViewProperties::setLock(bool locked)
{
	m_update = false;
	toolButton_Lock->setChecked(locked);
	m_update = true;
}

void OpticsViewProperties::setLockVisible(bool visible)
{
	toolButton_Lock->setVisible(visible);
}

/////////////////////////////////////////////////
// CornerWidget

CornerWidget::CornerWidget(QColor backgroundColor, const char* pixmapName, QWidget* widget, QWidget* parent)
	: QWidget(parent)
	, m_backgroundColor(backgroundColor)
	, m_widget(widget)
{
	m_pixmap = QPixmap(pixmapName);
	setFixedSize(m_pixmap.rect().size());
}

void CornerWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QBrush backgroundBrush(m_backgroundColor);
	QPen backgroundPen(m_backgroundColor);
	painter.setBrush(backgroundBrush);
	painter.setPen(backgroundPen);
	painter.drawRect(rect());
	painter.drawPixmap(m_pixmap.rect(), m_pixmap, m_pixmap.rect());
}

void CornerWidget::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event);

	if (m_widget)
		m_widget->setVisible(!m_widget->isVisible());
}

/////////////////////////////////////////////////
// Property selector

PropertySelector::PropertySelector(QWidget* parent)
	: QWidget(parent)
{
	m_propertyListWidget = new QListWidget;
	m_propertyListWidget->setDragDropMode(QAbstractItemView::InternalMove);

	m_symbolCheck = new QCheckBox(tr("Display full name"));

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Check and sort properties:")));
	layout->addWidget(m_propertyListWidget);
	layout->addWidget(m_symbolCheck);
	setLayout(layout);
	connect(m_symbolCheck, SIGNAL(stateChanged(int)), this, SLOT(checkBoxModified(int)));
	connect(m_propertyListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(checkBoxModified(int)));
	connect(m_propertyListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemModified(QListWidgetItem*)));

	setWindowIcon(QIcon(":/images/preferences-system.png"));
}

PropertySelector::~PropertySelector()
{
	writeSettings();
}

void PropertySelector::checkBoxModified(int)
{
	emit(propertyChanged());
}

void PropertySelector::itemModified(QListWidgetItem*)
{
	emit(propertyChanged());
}

bool PropertySelector::showFullName() const
{
	return m_symbolCheck->checkState() == Qt::Checked;
}

QList<Property::Type> PropertySelector::checkedItems() const
{
	QList<Property::Type> items;

	for (int i = 0; i < m_propertyListWidget->count(); i++)
		if (m_propertyListWidget->item(i)->checkState() == Qt::Checked)
			items << Property::Type(m_propertyListWidget->item(i)->data(Qt::UserRole).toInt());

	return items;
}

void PropertySelector::readSettings(QList<Property::Type> propertyList, QList<bool> checkList)
{
	QSettings settings;
	QList<QVariant> propertySettings = settings.value(m_settingsKey + "/properties").toList();
	QList<QVariant> checkSettings = settings.value(m_settingsKey + "/checks").toList();

	// Read settings
	if ((propertySettings.size() == propertyList.size()) && (checkSettings.size() == checkList.size()))
	{
		propertyList.clear();
		checkList.clear();
		for (int i = 0; i < propertySettings.size(); i++)
		{
			propertyList << Property::Type(propertySettings[i].toInt());
			checkList << checkSettings[i].toBool();
		}
	}

	m_propertyListWidget->clear();
	for (int i = 0; i < propertyList.size(); i++)
	{
		QListWidgetItem* widgetItem = new QListWidgetItem;
		QString text = Property::fullName[propertyList[i]];
		if (Property::fullName[propertyList[i]] != Property::shortName[propertyList[i]])
			text += " (" + Property::shortName[propertyList[i]] + ")";
		widgetItem->setText(text);
		widgetItem->setCheckState(checkList[i] ? Qt::Checked : Qt::Unchecked);
		widgetItem->setData(Qt::UserRole, propertyList[i]);
		m_propertyListWidget->addItem(widgetItem);
	}

	m_symbolCheck->setCheckState(Qt::CheckState(settings.value(m_settingsKey + "/fullName", Qt::Checked).toInt()));
}

void PropertySelector::writeSettings() const
{
	QList<QVariant> propertySettings;
	QList<QVariant> checkSettings;

	for (int i = 0; i < m_propertyListWidget->count(); i++)
	{
		propertySettings << m_propertyListWidget->item(i)->data(Qt::UserRole);
		checkSettings << (m_propertyListWidget->item(i)->checkState() == Qt::Checked);
	}

	QSettings settings;
	settings.setValue(m_settingsKey + "/properties", propertySettings);
	settings.setValue(m_settingsKey + "/checks", checkSettings);
	settings.setValue(m_settingsKey + "/fullName", m_symbolCheck->checkState());
}

/////////////////////////////////////////////////
// StatusPropertySelector

StatusPropertySelector::StatusPropertySelector(QWidget* parent)
	: PropertySelector(parent)
{
	setWindowTitle(tr("Configure status bar"));
	m_settingsKey = "StatusPropertySelector";

	QList<Property::Type> defaultPropertyList;
	QList<bool> defaultCheckList;

	defaultPropertyList << Property::BeamPosition << Property::BeamRadius << Property::BeamDiameter
	                    << Property::BeamCurvature << Property::BeamGouyPhase
	                    << Property::BeamDistanceToWaist << Property::BeamParameter << Property::Index;
	defaultCheckList << true << true << false << true << false << false << false << false;

	readSettings(defaultPropertyList, defaultCheckList);
}

/////////////////////////////////////////////////
// TablePropertySelector

TablePropertySelector::TablePropertySelector(QWidget* parent)
	: PropertySelector(parent)
{
	setWindowTitle(tr("Configure table columns"));
	m_settingsKey = "TablePropertySelector";

	QList<Property::Type> defaultPropertyList;
	QList<bool> defaultCheckList;

	defaultPropertyList << Property::OpticsType << Property::OpticsPosition
	                    << Property::OpticsRelativePosition << Property::OpticsProperties
	                    << Property::BeamWaist << Property::BeamWaistPosition << Property::BeamRayleigh
	                    << Property::BeamDivergence << Property::OpticsSensitivity
						<< Property::OpticsName << Property::OpticsLock
#ifdef ANGLE
						<< Property::OpticsAngle
#endif
						<< Property::OpticsOrientation;
	defaultCheckList << true << true << true << true << true << true << true << true << true << true << true
#ifdef ANGLE
					 << true
#endif
					 << true;

	readSettings(defaultPropertyList, defaultCheckList);
}

/////////////////////////////////////////////////
// StatusWidget

StatusWidget::StatusWidget(QStatusBar* statusBar)
	: QWidget(statusBar)
	, m_statusBar(statusBar)
{
	m_configWidget = new StatusPropertySelector(this);
	m_configWidget->setWindowFlags(Qt::Window);
	CornerWidget* corner = new CornerWidget(Qt::transparent, ":/images/preferences-system.png",
	                       m_configWidget, this);
	m_label = new QLabel;
	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(corner);
	layout->addWidget(m_label);
	setLayout(layout);
}

void StatusWidget::showBeamInfo(const Beam* beam, double z, Orientation orientation)
{
	m_statusBar->clearMessage();

	bool fullName = m_configWidget->showFullName();
	QList<Property::Type> items = m_configWidget->checkedItems();
	QString text;

	foreach (Property::Type type, items)
	{
		text += fullName ? (Property::fullName[type] + tr(": ")) : (Property::shortName[type] + tr(" = "));
		double value = 0.;
		if (type == Property::BeamPosition)
			value = z;
		else if (type == Property::BeamRadius)
			value = beam->radius(z, orientation);
		else if (type == Property::BeamDiameter)
			value = 2.*beam->radius(z, orientation);
		else if (type == Property::BeamCurvature)
			value = beam->curvature(z, orientation);
		else if (type == Property::BeamGouyPhase)
			value = beam->gouyPhase(z, orientation);
		else if ((type == Property::BeamDistanceToWaist) || (type == Property::BeamParameter))
			value = z - beam->waistPosition(orientation);
		else if (type == Property::Index)
			value = beam->index();

		value *=  Unit(Property::unit[type]).divider();
		text += QString::number(value, 'f', 2) + Unit(Property::unit[type]).string();

		if (type == Property::BeamParameter)
		{
			value = beam->rayleigh(orientation)*Unit(UnitRayleigh).divider();
			text += " + i" + QString::number(value, 'f', 2) + Unit(UnitRayleigh).string();
		}

		text += "    ";

	}
	m_label->setText(text);
}
