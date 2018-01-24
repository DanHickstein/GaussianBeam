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

#include "src/OpticsBench.h"
#include "src/GaussianBeam.h"
#include "src/Utils.h"
#include "gui/GaussianBeamModel.h"
#include "gui/OpticsWidgets.h"
#include "gui/Unit.h"
#include "gui/Names.h"

#include <QtGui>
#include <QDebug>
#include <QTextStream>

GaussianBeamModel::GaussianBeamModel(OpticsBench* bench, TablePropertySelector* propertySelector, QObject* parent)
	: QAbstractTableModel(parent)
	, m_propertySelector(propertySelector)

{
	m_bench = bench;
	m_bench->registerEventListener(this);

	m_columns = m_propertySelector->checkedItems();

	connect(m_propertySelector, SIGNAL(propertyChanged()), this, SLOT(propertyWidgetModified()));

	// Sync with bench
	for (int i = 0; i < m_bench->nOptics(); i++)
		onOpticsBenchOpticsAdded(i);
}

GaussianBeamModel::~GaussianBeamModel()
{
}

void GaussianBeamModel::propertyWidgetModified()
{
	beginResetModel();
	m_columns = m_propertySelector->checkedItems();
	endResetModel();
}

int GaussianBeamModel::rowCount(const QModelIndex& /*parent*/) const
{
	return m_bench->nOptics();
}

int GaussianBeamModel::columnCount(const QModelIndex& /*parent*/) const
{
	return m_columns.size();
}

QVariant GaussianBeamModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || ((role != Qt::DisplayRole) && (role != Qt::EditRole)))
		return QVariant();

	int row = index.row();
	Property::Type column = m_columns[index.column()];
	const Optics* optics = m_bench->optics(row);

	QList<QVariant> values;

	if (column == Property::OpticsType)
		return OpticsName::fullName[m_bench->optics(row)->type()];
	else if ((column == Property::OpticsPosition) && (optics->type() != CreateBeamType))
		values << optics->position()*Unit(UnitPosition).divider();
	else if ((column == Property::OpticsRelativePosition) && (row > 0))
		values << (optics->position() - m_bench->optics(row-1)->position())*Unit(UnitPosition).divider();
	else if (column == Property::OpticsProperties)
	{
		if (optics->type() == CreateBeamType)
		{
			return QString("n = ") + QString::number(dynamic_cast<const CreateBeam*>(optics)->beam()->index()) +
			       QString(", " + tr("M²") + " = ") + QString::number(dynamic_cast<const CreateBeam*>(optics)->beam()->M2());
		}
		else if (optics->type() == LensType)
		{
			return QString("f = ") + QString::number(dynamic_cast<const Lens*>(optics)->focal()*Unit(UnitFocal).divider())
			                       + Unit(UnitFocal).string();
		}
		else if (optics->type() == CurvedMirrorType)
		{
			return QString("R = ") + QString::number(dynamic_cast<const CurvedMirror*>(optics)->curvatureRadius()*Unit(UnitCurvature).divider())
			                       + Unit(UnitCurvature).string();
		}
		else if (optics->type() == FlatInterfaceType)
		{
			return QString("n2/n1 = ") + QString::number(dynamic_cast<const FlatInterface*>(optics)->indexRatio());
		}
		else if (optics->type() == CurvedInterfaceType)
		{
			const CurvedInterface* interface = dynamic_cast<const CurvedInterface*>(optics);
			return QString("n2/n1 = ") + QString::number(interface->indexRatio()) +
			       QString("\nR = ") + QString::number(interface->surfaceRadius()*Unit(UnitCurvature).divider())
			                       + Unit(UnitCurvature).string();
		}
		else if (optics->type() == DielectricSlabType)
		{
			const DielectricSlab* slab = dynamic_cast<const DielectricSlab*>(optics);
			return QString("n2/n1 = ") + QString::number(slab->indexRatio()) +
			       QString("\n") + tr("width") + " = " + QString::number(optics->width()*Unit(UnitWidth).divider())
			                       + Unit(UnitWidth).string();
		}
		else if (optics->type() == GenericABCDType)
		{
			const ABCD* abcd = dynamic_cast<const ABCD*>(optics);
			if (optics->orientation() == Spherical)
			{
				return QString(  "A = ") + QString::number(abcd->A(Spherical)) +
				       QString("\nB = ") + QString::number(abcd->B(Spherical)*Unit(UnitABCD).divider()) + Unit(UnitABCD).string() +
				       QString("\nC = ") + QString::number(abcd->C(Spherical)/Unit(UnitABCD).divider()) + " /" + Unit(UnitABCD).string(false) +
				       QString("\nD = ") + QString::number(abcd->D(Spherical)) +
				       QString("\n")     + tr("width") + " = " + QString::number(optics->width()*Unit(UnitWidth).divider()) + Unit(UnitWidth).string();
			}
			else
			{
				return QString(  "A(H) = ") + QString::number(abcd->A(Horizontal)) +
				       QString("\nA(V) = ") + QString::number(abcd->A(Vertical  )) +
				       QString("\nB(H) = ") + QString::number(abcd->B(Horizontal)*Unit(UnitABCD).divider()) + Unit(UnitABCD).string() +
				       QString("\nB(V) = ") + QString::number(abcd->B(Vertical  )*Unit(UnitABCD).divider()) + Unit(UnitABCD).string() +
				       QString("\nC(H) = ") + QString::number(abcd->C(Horizontal)/Unit(UnitABCD).divider()) + " /" + Unit(UnitABCD).string(false) +
				       QString("\nC(V) = ") + QString::number(abcd->C(Vertical  )/Unit(UnitABCD).divider()) + " /" + Unit(UnitABCD).string(false) +
				       QString("\nD(H) = ") + QString::number(abcd->D(Horizontal)) +
				       QString("\nD(V) = ") + QString::number(abcd->D(Vertical  )) +
				       QString("\n")     + tr("width") + " = " + QString::number(optics->width()*Unit(UnitWidth).divider()) + Unit(UnitWidth).string();
			}
		}
	}
	else if (column == Property::BeamWaist)
	{
		values << m_bench->beam(row)->waist(Horizontal)*Unit(UnitWaist).divider();
		if (!m_bench->isSpherical()) values << m_bench->beam(row)->waist(Vertical)*Unit(UnitWaist).divider();
	}
	else if (column == Property::BeamWaistPosition)
	{
		values << m_bench->beam(row)->waistPosition(Horizontal)*Unit(UnitPosition).divider();
		if (!m_bench->isSpherical()) values << m_bench->beam(row)->waistPosition(Vertical)*Unit(UnitPosition).divider();
	}
	else if (column == Property::BeamRayleigh)
	{
		values << m_bench->beam(row)->rayleigh(Horizontal)*Unit(UnitRayleigh).divider();
		if (!m_bench->isSpherical()) values << m_bench->beam(row)->rayleigh(Vertical)*Unit(UnitRayleigh).divider();
	}
	else if (column == Property::BeamDivergence)
	{
		values << m_bench->beam(row)->divergence(Horizontal)*Unit(UnitDivergence).divider();
		if (!m_bench->isSpherical()) values << m_bench->beam(row)->divergence(Vertical)*Unit(UnitDivergence).divider();
	}
	else if (column == Property::OpticsSensitivity)
		values << fabs(m_bench->sensitivity(row))*100./sqr(Unit(UnitPosition).divider());
	else if (column == Property::OpticsName)
	{
		return QString::fromUtf8(optics->name().c_str());
	}
	else if (column == Property::OpticsLock)
	{
		if (optics->absoluteLock())
			return tr("absolute");
		else if (optics->relativeLockParent())
			return QString::fromUtf8(optics->relativeLockParent()->name().c_str());
		else
			return tr("none");
	}
	else if ((column == Property::OpticsAngle) && optics->isRotable())
		values << optics->angle()*180./M_PI;
	else if ((column == Property::OpticsOrientation) && optics->isOrientable())
		return OrientationName::fullName[optics->orientation()];
	else
		return QVariant();

	if (role == Qt::EditRole)
		return values;

	QString string;
	QTextStream data(&string);
	data.setRealNumberNotation(QTextStream::SmartNotation);
//	data.setRealNumberPrecision(2);
	bool start = true;
	foreach(QVariant value, values)
	{
		if (!start) data << "\n";
		data << value.toDouble();
		start = false;
	}

	return string;
}

QVariant GaussianBeamModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			Property::Type type = m_columns[section];
			QString header = m_propertySelector->showFullName() ? Property::fullName[type] : Property::shortName[type];
			if (Property::unit[type] != UnitLess)
				header += " (" + Unit(Property::unit[type]).string(false) + ")";
			/// @todo handle this special case in a more general way
			if (type == Property::OpticsSensitivity)
				header += " (%/" + Unit(UnitPosition).string(false) + tr("²") + ")";
			return breakString(header);
		}
		else if (orientation == Qt::Vertical)
			return section;
	}

	return QVariant();
}

bool GaussianBeamModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid() || (role != Qt::EditRole) || value.isNull() || !value.isValid())
		return false;

	int row = index.row();
	Property::Type column = m_columns[index.column()];

	QList<Orientation> orientations;
	if (m_bench->isSpherical())
		orientations << Spherical;
	else
		orientations << Horizontal << Vertical;

	if (column == Property::OpticsPosition)
		m_bench->setOpticsPosition(row, value.toDouble()*Unit(UnitPosition).multiplier());
	else if (column == Property::OpticsRelativePosition)
	{
		if (row == 0)
			return false;

		const Optics* optics = m_bench->optics(row);
		const Optics* preceedingOptics = m_bench->optics(row-1);
		if (optics->relativeLockedTo(preceedingOptics))
			return false;
		m_bench->setOpticsPosition(row, value.toDouble()*Unit(UnitPosition).multiplier() + preceedingOptics->position());
	}
	else if (column == Property::OpticsProperties)
	{
		Optics* optics = m_bench->opticsForPropertyChange(row);

		if (optics->type() == CreateBeamType)
		{
			CreateBeam* createBeam = dynamic_cast<CreateBeam*>(optics);
			Beam beam = *createBeam->beam();
			beam.setIndex(value.toList()[0].toDouble());
			beam.setM2(value.toList()[1].toDouble());
			createBeam->setBeam(beam);
		}
		else if (optics->type() == LensType)
			dynamic_cast<Lens*>(optics)->setFocal(value.toList()[0].toDouble()*Unit(UnitFocal).multiplier());
		else if (optics->type() == CurvedMirrorType)
			dynamic_cast<CurvedMirror*>(optics)->setCurvatureRadius(value.toList()[0].toDouble()*Unit(UnitCurvature).multiplier());
		else if (optics->type() == FlatInterfaceType)
			dynamic_cast<FlatInterface*>(optics)->setIndexRatio(value.toList()[0].toDouble());
		else if (optics->type() == CurvedInterfaceType)
		{
			CurvedInterface* curvedInterfaceOptics = dynamic_cast<CurvedInterface*>(optics);
			curvedInterfaceOptics->setIndexRatio(value.toList()[0].toDouble());
			curvedInterfaceOptics->setSurfaceRadius(value.toList()[1].toDouble()*Unit(UnitCurvature).multiplier());
		}
		else if (optics->type() == DielectricSlabType)
		{
			DielectricSlab* dielectricSlabOptics = dynamic_cast<DielectricSlab*>(optics);
			dielectricSlabOptics->setIndexRatio(value.toList()[0].toDouble());
			dielectricSlabOptics->setWidth(value.toList()[1].toDouble()*Unit(UnitWidth).multiplier());
		}
		else if (optics->type() == GenericABCDType)
		{
			/// @todo check that the ABCD matrix is valid, e.g. by introducing bool GenericABCD::isValid()
			GenericABCD* ABCDOptics = dynamic_cast<GenericABCD*>(optics);
			if (optics->orientation() == Spherical)
			{
				ABCDOptics->setA(value.toList()[0].toDouble(), Spherical);
				ABCDOptics->setB(value.toList()[1].toDouble()*Unit(UnitABCD).multiplier(), Spherical);
				ABCDOptics->setC(value.toList()[2].toDouble()/Unit(UnitABCD).multiplier(), Spherical);
				ABCDOptics->setD(value.toList()[3].toDouble(), Spherical);
				ABCDOptics->setWidth(value.toList()[4].toDouble()*Unit(UnitWidth).multiplier());
			}
			else
			{
				ABCDOptics->setA(value.toList()[0].toDouble(), Horizontal);
				ABCDOptics->setA(value.toList()[1].toDouble(), Vertical  );
				ABCDOptics->setB(value.toList()[2].toDouble()*Unit(UnitABCD).multiplier(), Horizontal);
				ABCDOptics->setB(value.toList()[3].toDouble()*Unit(UnitABCD).multiplier(), Vertical  );
				ABCDOptics->setC(value.toList()[4].toDouble()/Unit(UnitABCD).multiplier(), Horizontal);
				ABCDOptics->setC(value.toList()[5].toDouble()/Unit(UnitABCD).multiplier(), Vertical  );
				ABCDOptics->setD(value.toList()[6].toDouble(), Horizontal);
				ABCDOptics->setD(value.toList()[7].toDouble(), Vertical  );
				ABCDOptics->setWidth(value.toList()[8].toDouble()*Unit(UnitWidth).multiplier());
			}
		}
		m_bench->opticsPropertyChanged(row);
	}
	else if (column == Property::BeamWaist)
		for (int i = 0; i < orientations.size(); i++)
		{
			Beam beam = *m_bench->beam(row);
			beam.setWaist(value.toList()[i].toDouble()*Unit(UnitWaist).multiplier(), orientations[i]);
			m_bench->setBeam(beam, row);
		}
	else if (column == Property::BeamWaistPosition)
		for (int i = 0; i < orientations.size(); i++)
		{
			Beam beam = *m_bench->beam(row);
			beam.setWaistPosition(value.toList()[i].toDouble()*Unit(UnitPosition).multiplier(), orientations[i]);
			m_bench->setBeam(beam, row);
		}
	else if (column == Property::BeamRayleigh)
		for (int i = 0; i < orientations.size(); i++)
		{
			Beam beam = *m_bench->beam(row);
			beam.setRayleigh(value.toList()[i].toDouble()*Unit(UnitRayleigh).multiplier(), orientations[i]);
			m_bench->setBeam(beam, row);
		}
	else if (column == Property::BeamDivergence)
		for (int i = 0; i < orientations.size(); i++)
		{
			Beam beam = *m_bench->beam(row);
			beam.setDivergence(value.toList()[i].toDouble()*Unit(UnitDivergence).multiplier(), orientations[i]);
			m_bench->setBeam(beam, row);
		}
	else if (column == Property::OpticsName)
	{
		Optics* optics = m_bench->opticsForPropertyChange(row);
		optics->setName(value.toString().toUtf8().data());
		m_bench->opticsPropertyChanged(row);
	}
	else if (column == Property::OpticsLock)
	{
		/// @todo make specific functions in OpticsBench to change this. Move this logic to OpticsBench
		int lockId = value.toInt();
		if (value == -2)
		{
			Optics* optics = m_bench->opticsForPropertyChange(row);
			optics->setAbsoluteLock(false);
			optics->relativeUnlock();
			m_bench->opticsPropertyChanged(row);
		}
		else if (value == -1)
		{
			Optics* optics = m_bench->opticsForPropertyChange(row);
			optics->setAbsoluteLock(true);
			m_bench->opticsPropertyChanged(row);
		}
		else
		{
			Optics* optics = m_bench->opticsForPropertyChange(row);
			optics->relativeLockTo(m_bench->opticsForPropertyChange(lockId));
			m_bench->opticsPropertyChanged(row);
		}
	}
	else if (column == Property::OpticsAngle)
	{
		Optics* optics = m_bench->opticsForPropertyChange(row);
		optics->setAngle(value.toDouble()*M_PI/180.);
		m_bench->opticsPropertyChanged(row);
	}
	else if (column == Property::OpticsOrientation)
	{
		Optics* optics = m_bench->opticsForPropertyChange(row);
		optics->setOrientation(Orientation(value.toInt()));
		m_bench->opticsPropertyChanged(row);
	}

	return true;
}

Qt::ItemFlags GaussianBeamModel::flags(const QModelIndex& index) const
{
	int row = index.row();
	Property::Type column = m_columns[index.column()];

	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	const Optics* optics = m_bench->optics(row);

	if ((column == Property::OpticsName) ||
		(column == Property::OpticsLock) ||
		(column == Property::BeamWaist) ||
		(column == Property::BeamWaistPosition) ||
		(column == Property::BeamRayleigh) ||
		(column == Property::BeamDivergence) ||
		((column == Property::OpticsProperties)  && (optics->type() != FlatMirrorType)) ||
		((column == Property::OpticsAngle)       && (optics->isRotable())) ||
		((column == Property::OpticsOrientation) && (optics->isOrientable())))
			flags |= Qt::ItemIsEditable;

	if (column == Property::OpticsPosition)
		if (!optics->relativeLockTreeAbsoluteLock() && (optics->type() != CreateBeamType))
			flags |= Qt::ItemIsEditable;

	if ((column == Property::OpticsRelativePosition) && (row != 0))
	{
		const Optics* preceedingOptics = m_bench->optics(row-1);
		if (!optics->relativeLockedTo(preceedingOptics) &&
		    !optics->relativeLockTreeAbsoluteLock())
			flags |= Qt::ItemIsEditable;
	}


	return flags;
}

bool GaussianBeamModel::insertRows(int row, int count, const QModelIndex& parent)
{
	beginInsertRows(parent, row, row + count -1);
	endInsertRows();
	return true;
}

bool GaussianBeamModel::removeRows(int row, int count, const QModelIndex& parent)
{
	beginRemoveRows(parent, row, row + count -1);
	endRemoveRows();
	return true;
}

void GaussianBeamModel::onOpticsBenchDataChanged(int startOptics, int endOptics)
{
	emit dataChanged(index(startOptics, 0), index(endOptics, columnCount()-1));
}

void GaussianBeamModel::onOpticsBenchOpticsAdded(int index)
{
	insertRows(index, 1);
}

void GaussianBeamModel::onOpticsBenchOpticsRemoved(int index, int count)
{
	removeRows(index, count);
}
