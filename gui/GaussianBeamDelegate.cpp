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

#include "gui/GaussianBeamDelegate.h"
#include "gui/GaussianBeamModel.h"
#include "gui/Unit.h"
#include "gui/Names.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	#include <QtWidgets>
#endif
#include <QtGui>

PropertyEditor::PropertyEditor(QList<EditorProperty>& properties, QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	foreach (EditorProperty property, properties)
	{
		QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
		spinBox->setAccelerated(true);
		spinBox->setMinimum(property.minimum);
		spinBox->setMaximum(property.maximum);
		spinBox->setPrefix(property.prefix);
		spinBox->setSuffix(property.suffix);
		layout->addWidget(spinBox);
		m_doubleSpinBoxes << spinBox;
	}

	setLayout(layout);
}

QList<QVariant> PropertyEditor::values() const
{
	QList<QVariant> properties;

	foreach (QDoubleSpinBox* spinBox, m_doubleSpinBoxes)
		properties << spinBox->value();

	return properties;
}

GaussianBeamDelegate::GaussianBeamDelegate(QObject* parent, GaussianBeamModel* model, OpticsBench* bench)
	: QItemDelegate(parent)
	, m_model(model)
	, m_bench(bench)
{
}

QWidget *GaussianBeamDelegate::createEditor(QWidget* parent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
	/// @todo where are all these "new" things deleted ? Check Qt example

	int row = index.row();
	Property::Type column = m_model->columnContent(index.column());
	const Optics* optics = m_bench->optics(row);

	switch (column)
	{
	case Property::OpticsPosition:
	case Property::OpticsRelativePosition:
	case Property::OpticsAngle:
	{
		QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
		editor->setAccelerated(true);
		editor->setMinimum(-Utils::infinity);
		editor->setMaximum(Utils::infinity);
		return editor;
	}
	case Property::BeamWaist:
	case Property::BeamRayleigh:
	case Property::BeamDivergence:
	{
		QList<EditorProperty> properties;
		properties << EditorProperty(0., Utils::infinity);
		if (!m_bench->isSpherical())
			properties << EditorProperty(0., Utils::infinity);
		return new PropertyEditor(properties, parent);
	}
	case Property::BeamWaistPosition:
	{
		QList<EditorProperty> properties;
		properties << EditorProperty(-Utils::infinity, Utils::infinity);
		if (!m_bench->isSpherical())
			properties << EditorProperty(-Utils::infinity, Utils::infinity);
		return new PropertyEditor(properties, parent);
	}
	case Property::OpticsProperties:
	{
		QList<EditorProperty> properties;
		if (optics->type() == CreateBeamType)
			properties << EditorProperty(0., Utils::infinity, "n = ")
			           << EditorProperty(1., Utils::infinity, tr("M²") + " = ");
		else if (optics->type() == LensType)
			properties << EditorProperty(-Utils::infinity, Utils::infinity, "f = ", Unit(UnitFocal).string());
		else if (optics->type() == CurvedMirrorType)
			properties << EditorProperty(-Utils::infinity, Utils::infinity, "R = ", Unit(UnitCurvature).string());
		else if (optics->type() == FlatInterfaceType)
			properties << EditorProperty(0., Utils::infinity, "n2/n1 = ");
		else if (optics->type() == CurvedInterfaceType)
			properties << EditorProperty(0., Utils::infinity, "n2/n1 = ")
			           << EditorProperty(-Utils::infinity, Utils::infinity, "R = ", Unit(UnitCurvature).string());
		else if (optics->type() == DielectricSlabType)
			properties << EditorProperty(0., Utils::infinity, "n2/n1 = ")
			           << EditorProperty(0., Utils::infinity, "width = ", Unit(UnitWidth).string());
		else if (optics->type() == GenericABCDType)
		{
			if (optics->orientation() == Spherical)
				properties << EditorProperty(-Utils::infinity, Utils::infinity, "A = ")
				           << EditorProperty(-Utils::infinity, Utils::infinity, "B = ", Unit(UnitABCD).string())
				           << EditorProperty(-Utils::infinity, Utils::infinity, "C = ", " /" + Unit(UnitABCD).string(false))
				           << EditorProperty(-Utils::infinity, Utils::infinity, "D = ")
				           << EditorProperty(0., Utils::infinity, "width = ", Unit(UnitWidth).string());
			else
				properties << EditorProperty(-Utils::infinity, Utils::infinity, "A(H) = ")
				           << EditorProperty(-Utils::infinity, Utils::infinity, "A(V) = ")
				           << EditorProperty(-Utils::infinity, Utils::infinity, "B(H) = ", Unit(UnitABCD).string())
				           << EditorProperty(-Utils::infinity, Utils::infinity, "B(V) = ", Unit(UnitABCD).string())
				           << EditorProperty(-Utils::infinity, Utils::infinity, "C(H) = ", " /" + Unit(UnitABCD).string(false))
				           << EditorProperty(-Utils::infinity, Utils::infinity, "C(V) = ", " /" + Unit(UnitABCD).string(false))
				           << EditorProperty(-Utils::infinity, Utils::infinity, "D(H) = ")
				           << EditorProperty(-Utils::infinity, Utils::infinity, "D(V) = ")
				           << EditorProperty(0., Utils::infinity, "width = ", Unit(UnitWidth).string());
		}

		return new PropertyEditor(properties, parent);
	}
	case Property::OpticsName:
	{
		QLineEdit* editor = new QLineEdit(parent);
		return editor;
	}
	case Property::OpticsLock:
	{
		QComboBox* editor = new QComboBox(parent);
		editor->addItem(tr("none"), -2);
		editor->addItem(tr("absolute"), -1);
		for (int i = 0; i < m_model->rowCount(); i++)
			if ((!m_bench->optics(i)->relativeLockedTo(optics)) || (m_bench->optics(i) == optics->relativeLockParent()))
				editor->addItem(QString::fromUtf8(m_bench->optics(i)->name().c_str()), i);
		return editor;
	}
	case Property::OpticsOrientation:
	{
		QComboBox* editor = new QComboBox(parent);
		if (optics->isOrientable(Spherical))   editor->addItem(OrientationName::fullName[Spherical]  , Spherical);
		if (optics->isOrientable(Ellipsoidal)) editor->addItem(OrientationName::fullName[Ellipsoidal], Ellipsoidal);
		if (optics->isOrientable(Horizontal))  editor->addItem(OrientationName::fullName[Horizontal] , Horizontal);
		if (optics->isOrientable(Vertical))    editor->addItem(OrientationName::fullName[Vertical]   , Vertical);
		return editor;
	}
	default:
		return 0;
	}

	return 0;
}

void GaussianBeamDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	/// @todo why static cast ?

	if (!index.isValid() || (editor == 0))
		return;

	int row = index.row();
	Property::Type column = m_model->columnContent(index.column());
	const Optics* optics = m_bench->optics(row);
	PropertyEditor* propertyEditor = static_cast<PropertyEditor*>(editor);

	switch (column)
	{
	case Property::OpticsPosition:
	case Property::OpticsAngle:
	case Property::OpticsRelativePosition:
	{
		double value = m_model->data(index, Qt::EditRole).toList()[0].toDouble();
		QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->setValue(value);
		break;
	}
	case Property::BeamWaist:
	case Property::BeamWaistPosition:
	case Property::BeamRayleigh:
	case Property::BeamDivergence:
	{
		QList<QVariant> values = m_model->data(index, Qt::EditRole).toList();
		for(int i = 0; i < values.size(); i++)
			propertyEditor->setValue(i, values[i].toDouble());
		break;
	}
	case Property::OpticsProperties:
	{
		if (optics->type() == CreateBeamType)
		{
			const CreateBeam* createBeam = dynamic_cast<const CreateBeam*>(optics);
			propertyEditor->setValue(0, createBeam->beam()->index());
			propertyEditor->setValue(1, createBeam->beam()->M2());
		}
		else if (optics->type() == LensType)
			propertyEditor->setValue(0, dynamic_cast<const Lens*>(optics)->focal()*Unit(UnitFocal).divider());
		else if (optics->type() == CurvedMirrorType)
			propertyEditor->setValue(0, dynamic_cast<const CurvedMirror*>(optics)->curvatureRadius()*Unit(UnitCurvature).divider());
		else if (optics->type() == FlatInterfaceType)
			propertyEditor->setValue(0, dynamic_cast<const FlatInterface*>(optics)->indexRatio());
		else if (optics->type() == CurvedInterfaceType)
		{
			const CurvedInterface* curvedInterface = dynamic_cast<const CurvedInterface*>(optics);
			propertyEditor->setValue(0, curvedInterface->indexRatio());
			propertyEditor->setValue(1, curvedInterface->surfaceRadius()*Unit(UnitCurvature).divider());
		}
		else if (optics->type() == GenericABCDType)
		{
			const GenericABCD* ABCDOptics = dynamic_cast<const GenericABCD*>(optics);
			if (optics->orientation() == Spherical)
			{
				propertyEditor->setValue(0, ABCDOptics->A(Spherical));
				propertyEditor->setValue(1, ABCDOptics->B(Spherical)*Unit(UnitABCD).divider());
				propertyEditor->setValue(2, ABCDOptics->C(Spherical)/Unit(UnitABCD).divider());
				propertyEditor->setValue(3, ABCDOptics->D(Spherical));
				propertyEditor->setValue(4, ABCDOptics->width()*Unit(UnitWidth).divider());
			}
			else
			{
				propertyEditor->setValue(0, ABCDOptics->A(Horizontal));
				propertyEditor->setValue(1, ABCDOptics->A(Vertical  ));
				propertyEditor->setValue(2, ABCDOptics->B(Horizontal)*Unit(UnitABCD).divider());
				propertyEditor->setValue(3, ABCDOptics->B(Vertical  )*Unit(UnitABCD).divider());
				propertyEditor->setValue(4, ABCDOptics->C(Horizontal)/Unit(UnitABCD).divider());
				propertyEditor->setValue(5, ABCDOptics->C(Vertical  )/Unit(UnitABCD).divider());
				propertyEditor->setValue(6, ABCDOptics->D(Horizontal));
				propertyEditor->setValue(7, ABCDOptics->D(Vertical  ));
				propertyEditor->setValue(8, ABCDOptics->width()*Unit(UnitWidth).divider());
			}
			break;
		}
		else if (optics->type() == DielectricSlabType)
		{
			const DielectricSlab* dielectricSlab = dynamic_cast<const DielectricSlab*>(optics);
			propertyEditor->setValue(0, dielectricSlab->indexRatio());
			propertyEditor->setValue(1, dielectricSlab->width()*Unit(UnitWidth).divider());
		}

		break;
	}
	case Property::OpticsName:
	{
		QString name = m_model->data(index, Qt::DisplayRole).toString();
		QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
		lineEdit->setText(name);
		break;
	}
	case Property::OpticsLock:
	{
		QString value = m_model->data(index, Qt::DisplayRole).toString();
		int lockId = -2;
		if (optics->absoluteLock())
			lockId = -1;
		else if (m_bench->optics(row)->relativeLockParent())
			lockId = m_bench->opticsIndex(m_bench->optics(row)->relativeLockParent());
		QComboBox* comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentIndex(comboBox->findData(lockId));
		break;
	}
	case Property::OpticsOrientation:
	{
		QComboBox* comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentIndex(comboBox->findData(optics->orientation()));
		break;
	}
	default:
		return QItemDelegate::setEditorData(editor, index);
	}
}

void GaussianBeamDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
	const QModelIndex& index) const
{
	if (!index.isValid() || (editor == 0))
		return;

	Property::Type column = m_model->columnContent(index.column());

	switch (column)
	{
	case Property::OpticsProperties:
	case Property::BeamWaist:
	case Property::BeamWaistPosition:
	case Property::BeamRayleigh:
	case Property::BeamDivergence:
	{
		model->setData(index, static_cast<PropertyEditor*>(editor)->values());
		break;
	}
	case Property::OpticsLock:
	case Property::OpticsOrientation:
	{
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		model->setData(index, comboBox->itemData(comboBox->currentIndex()));
		break;
	}
	default:
		QItemDelegate::setModelData(editor, model, index);
	}
}

void GaussianBeamDelegate::updateEditorGeometry(QWidget* editor,
	const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid() || (editor == 0))
		return;

	QItemDelegate::updateEditorGeometry(editor, option, index);
}
