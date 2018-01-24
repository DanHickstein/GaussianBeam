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

#include "gui/GaussianBeamWindow.h"
#include "gui/OpticsView.h"
#include "gui/Unit.h"
#include "gui/Names.h"
#include "src/GaussianFit.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtXml/QDomDocument>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	#include <QXmlStreamWriter>
#else
	#include <QtXml/QXmlStreamWriter>
#endif


void GaussianBeamWindow::writeOrientedElement(QXmlStreamWriter& xmlWriter, QString name, QString data, Orientation orientation) const
{
	xmlWriter.writeStartElement(name);
	xmlWriter.writeAttribute("orientation", OrientationName::codedName[orientation]);
	xmlWriter.writeCharacters(data);
	xmlWriter.writeEndElement();
}

bool GaussianBeamWindow::writeFile(const QString& fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		QMessageBox::warning(this, tr("Saving file"), tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return false;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument("1.0");
	xmlWriter.writeDTD("<!DOCTYPE gaussianBeam>");
	xmlWriter.writeStartElement("gaussianBeam");
	xmlWriter.writeAttribute("version", "1.2");
		xmlWriter.writeStartElement("bench");
		xmlWriter.writeAttribute("id", "0");
			writeBench(xmlWriter);
		xmlWriter.writeEndElement();
		xmlWriter.writeStartElement("view");
		xmlWriter.writeAttribute("id", "0");
		xmlWriter.writeAttribute("bench", "0");
			writeView(xmlWriter);
		xmlWriter.writeEndElement();
	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	file.close();
	return true;
}

void GaussianBeamWindow::writeWaist(QXmlStreamWriter& xmlWriter, const Beam* beam, Orientation orientation) const
{
	writeOrientedElement(xmlWriter, "waist", QString::number(beam->waist(orientation)), orientation);
	writeOrientedElement(xmlWriter, "waistPosition", QString::number(beam->waistPosition(orientation)), orientation);
}

void GaussianBeamWindow::writeBeam(QXmlStreamWriter& xmlWriter, const Beam* beam) const
{
	if (beam->isSpherical())
		writeWaist(xmlWriter, beam, Spherical);
	else
	{
		writeWaist(xmlWriter, beam, Horizontal);
		writeWaist(xmlWriter, beam, Vertical);
	}
	xmlWriter.writeTextElement("wavelength", QString::number(beam->wavelength()));
	xmlWriter.writeTextElement("index", QString::number(beam->index()));
	xmlWriter.writeTextElement("M2", QString::number(beam->M2()));
}

void GaussianBeamWindow::writeBench(QXmlStreamWriter& xmlWriter) const
{
	xmlWriter.writeTextElement("wavelength", QString::number(m_bench->wavelength()));
	xmlWriter.writeTextElement("leftBoundary", QString::number(m_bench->leftBoundary()));
	xmlWriter.writeTextElement("rightBoundary", QString::number(m_bench->rightBoundary()));

	xmlWriter.writeStartElement("targetBeam");
	xmlWriter.writeAttribute("id", "0");
		writeBeam(xmlWriter, m_bench->targetBeam());
		xmlWriter.writeTextElement("targetOverlap", QString::number(m_bench->targetOverlap()));
		xmlWriter.writeTextElement("targetOrientation", QString::number(m_bench->targetOrientation()));
		/// @todo should we save the "showTargetBeam" property ?
	xmlWriter.writeEndElement();

	for (int i = 0; i < m_bench->nFit(); i++)
	{
		Fit* fit = m_bench->fit(i);
		xmlWriter.writeStartElement("beamFit");
		xmlWriter.writeAttribute("id", QString::number(i));
			xmlWriter.writeTextElement("name", QString::fromUtf8(fit->name().c_str()));
			xmlWriter.writeTextElement("dataType", QString::number(int(fit->dataType())));
			xmlWriter.writeTextElement("color", QString::number(fit->color()));
			xmlWriter.writeTextElement("orientation", OrientationName::codedName[fit->orientation()]);
			for (int j = 0; j < fit->size(); j++)
			{
				xmlWriter.writeStartElement("data");
				xmlWriter.writeAttribute("id", QString::number(j));
					xmlWriter.writeTextElement("position", QString::number(fit->position(j)));
					if (fit->orientation() == Spherical)
						writeOrientedElement(xmlWriter, "value", QString::number(fit->value(j, Spherical)), Spherical);
					else
					{
						if (fit->orientation() != Vertical)
							writeOrientedElement(xmlWriter, "value", QString::number(fit->value(j, Horizontal)), Horizontal);
						if (fit->orientation() != Horizontal)
							writeOrientedElement(xmlWriter, "value", QString::number(fit->value(j, Vertical  )), Vertical  );
					}
				xmlWriter.writeEndElement();
			}
		xmlWriter.writeEndElement();
	}

	xmlWriter.writeStartElement("opticsList");
	for (int i = 0; i < m_bench->nOptics(); i++)
	{
		xmlWriter.writeStartElement(OpticsName::codedName[m_bench->optics(i)->type()]);
		xmlWriter.writeAttribute("id", QString::number(i));
		writeOptics(xmlWriter, m_bench->optics(i));
		xmlWriter.writeEndElement();
	}
	xmlWriter.writeEndElement();
}

void GaussianBeamWindow::writeOptics(QXmlStreamWriter& xmlWriter, const Optics* optics) const
{
	xmlWriter.writeTextElement("position", QString::number(optics->position()));
	xmlWriter.writeTextElement("angle", QString::number(optics->angle()));
	xmlWriter.writeTextElement("orientation", OrientationName::codedName[optics->orientation()]);
	xmlWriter.writeTextElement("name", QString::fromUtf8(optics->name().c_str()));
	xmlWriter.writeTextElement("absoluteLock", QString::number(optics->absoluteLock() ? true : false));
	if (optics->relativeLockParent())
		xmlWriter.writeTextElement("relativeLockParent", QString::number(m_bench->opticsIndex(optics->relativeLockParent())));

	if (optics->type() == CreateBeamType)
	{
		xmlWriter.writeStartElement("beam");
		writeBeam(xmlWriter, dynamic_cast<const CreateBeam*>(optics)->beam());
		xmlWriter.writeEndElement();
	}
	else if (optics->type() == LensType)
		xmlWriter.writeTextElement("focal", QString::number(dynamic_cast<const Lens*>(optics)->focal()));
	else if (optics->type() == CurvedMirrorType)
		xmlWriter.writeTextElement("curvatureRadius", QString::number(dynamic_cast<const CurvedMirror*>(optics)->curvatureRadius()));
	else if (optics->type() == FlatInterfaceType)
		xmlWriter.writeTextElement("indexRatio", QString::number(dynamic_cast<const FlatInterface*>(optics)->indexRatio()));
	else if (optics->type() == CurvedInterfaceType)
	{
		xmlWriter.writeTextElement("indexRatio", QString::number(dynamic_cast<const CurvedInterface*>(optics)->indexRatio()));
		xmlWriter.writeTextElement("surfaceRadius", QString::number(dynamic_cast<const CurvedInterface*>(optics)->surfaceRadius()));
	}
	else if (optics->type() == DielectricSlabType)
	{
		xmlWriter.writeTextElement("indexRatio", QString::number(dynamic_cast<const DielectricSlab*>(optics)->indexRatio()));
		xmlWriter.writeTextElement("width", QString::number(optics->width()));
	}
	else if (optics->type() == GenericABCDType)
	{
		const GenericABCD* ABCDOptics = dynamic_cast<const GenericABCD*>(optics);
		xmlWriter.writeTextElement("width", QString::number(optics->width()));
		if (optics->orientation() == Spherical)
		{
			writeOrientedElement(xmlWriter, "A", QString::number(ABCDOptics->A(Spherical)), Spherical);
			writeOrientedElement(xmlWriter, "B", QString::number(ABCDOptics->B(Spherical)), Spherical);
			writeOrientedElement(xmlWriter, "C", QString::number(ABCDOptics->C(Spherical)), Spherical);
			writeOrientedElement(xmlWriter, "D", QString::number(ABCDOptics->D(Spherical)), Spherical);
		}
		else
		{
			writeOrientedElement(xmlWriter, "A", QString::number(ABCDOptics->A(Horizontal)), Horizontal);
			writeOrientedElement(xmlWriter, "A", QString::number(ABCDOptics->A(Vertical  )), Vertical  );
			writeOrientedElement(xmlWriter, "B", QString::number(ABCDOptics->B(Horizontal)), Horizontal);
			writeOrientedElement(xmlWriter, "B", QString::number(ABCDOptics->B(Vertical  )), Vertical  );
			writeOrientedElement(xmlWriter, "C", QString::number(ABCDOptics->C(Horizontal)), Horizontal);
			writeOrientedElement(xmlWriter, "C", QString::number(ABCDOptics->C(Vertical  )), Vertical  );
			writeOrientedElement(xmlWriter, "D", QString::number(ABCDOptics->D(Horizontal)), Horizontal);
			writeOrientedElement(xmlWriter, "D", QString::number(ABCDOptics->D(Vertical  )), Vertical  );
		}
	}
}

void GaussianBeamWindow::writeView(QXmlStreamWriter& xmlWriter) const
{
	xmlWriter.writeTextElement("horizontalRange", QString::number(m_hOpticsView->horizontalRange()));
	/// @todo vertial origin
	xmlWriter.writeTextElement("origin", QString::number(m_hOpticsView->origin().x()));
	xmlWriter.writeStartElement("showTargetBeam");
	xmlWriter.writeAttribute("id", "0");
	xmlWriter.writeCharacters(QString::number(m_hOpticsScene->targetBeamVisible()));
	xmlWriter.writeEndElement();
}
