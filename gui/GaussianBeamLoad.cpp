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
#include <QBuffer>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtXml/QDomDocument>
#include <QtXmlPatterns/QXmlQuery>

/**************************************************************
Change log for GaussianBeam files. All this changes are coded in XSL-T documents
able to automatically perform file format conversion

File versions:
==============

GaussianBeam 0.1 -> file version 1.0
GaussianBeam 0.2 -> file version 1.0
GaussianBeam 0.3 -> file version 1.0
GaussianBeam 0.4 -> file version 1.1
GaussianBeam 0.5 -> file version 1.2

GaussianBeam 0.2 (1.0)
======================

	New <flatMirror> and <genericABCD> tags

GaussianBeam 0.3 (1.0)
======================

	<scramble> deprecated
	New <showTargetWaist>, <absoluteLock> and <relativeLockParent> tags

GaussianBeam 0.4 (1.1)
======================

	New <bench id="0"> and <view id="0" bench="0"> tags

	<magicWaist> -> <targetBeam id="0">
		<targetWaist> -> <waist>
		<targetPosition> -> <position>
		New <minOverlap> and <overlapCriterion> tags
		<showTargetWaist> -> <showTargetBeam> (in <view>)

	<waistFit> -> <beamFit id="n">
		New <name> and <color> tags
		<fitDataType> -> <dataType>
		<fitData> -> <data id = "n">
			<dataPosition> -> <position>
			<dataValue> -> <value>

	<display> dropped. Values transfered to <view>

	New <opticsList> tag

	New <dielectricSlab> and <curvedMirror> tags

GaussianBeam 0.5 (1.2)
======================



***************************************************************/

void GaussianBeamWindow::convertFormat(QByteArray* data, const QString& xsltPath) const
{
	QXmlQuery query(QXmlQuery::XSLT20);
	QByteArray convertedData;
	QBuffer inputBuffer(data);
	QBuffer outputBuffer(&convertedData);

	inputBuffer.open(QIODevice::ReadOnly);
	outputBuffer.open(QIODevice::WriteOnly);
	// Set the data to convert
	query.setFocus(&inputBuffer);
	// Set the xslt style-sheet
	QFile xslt(xsltPath);
	xslt.open(QFile::ReadOnly);
	query.setQuery(&xslt);
	xslt.close();
	// Convert
	query.evaluateTo(&outputBuffer);
	outputBuffer.close();
	inputBuffer.close();
	*data = convertedData;
}

bool GaussianBeamWindow::parseFile(const QString& fileName)
{
	// Load data
	QFile file(fileName);
	if (!(file.open(QFile::ReadOnly | QFile::Text)))
	{
		QMessageBox::warning(this, tr("Opening file"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return false;
	}
	QByteArray data = file.readAll();
	file.close();

	// Convert old file versions
	convertFormat(&data, ":/xslt/1_0_to_1_1.xsl");
	convertFormat(&data, ":/xslt/1_1_to_1_2.xsl");

	// Parse XML file
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;

	if (!domDocument.setContent(data, true, &errorStr, &errorLine, &errorColumn))
	{
		QMessageBox::information(window(), tr("XML error"), tr("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr));
		file.close();
		return false;
	}

	// XML version
	QDomElement root = domDocument.documentElement();
	if (root.tagName() != "gaussianBeam")
	{
		QMessageBox::information(window(), tr("XML error"), tr("The file is not an GaussianBeam file."));
		return false;
	}

	if (!root.hasAttribute("version"))
	{
		QMessageBox::information(window(), tr("XML error"), tr("This file does not contain any version information."));
		return false;
	}

	if (root.attribute("version") != "1.2")
	{
		QMessageBox::information(window(), tr("XML error"), tr("Your version of GaussianBeam is too old."));
		return false;
	}

	m_bench->clear();
	parseXml(root);

	return true;
}

void GaussianBeamWindow::parseXml(const QDomElement& element)
{
	QDomElement child = element.firstChildElement();

	while (!child.isNull())
	{
		if (child.tagName() == "bench")
			parseBench(child);
		else if (child.tagName() == "view")
			parseView(child);
		else
			qDebug() << " -> Unknown tag: " << element.tagName();
		child = child.nextSiblingElement();
	}
}

void GaussianBeamWindow::parseBench(const QDomElement& element)
{
	QDomElement child = element.firstChildElement();
	QList<QString> lockTree;

	while (!child.isNull())
	{
		if (child.tagName() == "wavelength")
			m_bench->setWavelength(child.text().toDouble());
		else if (child.tagName() == "leftBoundary")
			m_bench->setLeftBoundary(child.text().toDouble());
		else if (child.tagName() == "rightBoundary")
			m_bench->setRightBoundary(child.text().toDouble());
		else if (child.tagName() == "targetBeam")
			parseTargetBeam(child);
		else if (child.tagName() == "beamFit")
			parseFit(child);
		else if (child.tagName() == "opticsList")
		{
			QMap<int, Optics*> opticsList; // Key = id
			QMap<int, int> lockTree;       // Key = child id, value = parent id

			QDomElement opticsElement = child.firstChildElement();
			while (!opticsElement.isNull())
			{
				parseOptics(opticsElement, opticsList, lockTree);
				opticsElement = opticsElement.nextSiblingElement();
			}

			for(QMap<int, int>::const_iterator it = lockTree.constBegin(); it != lockTree.constEnd(); ++it)
			{
				Optics* opticsChild  = opticsList.value(it.key());
				Optics* opticsParent = opticsList.value(it.value());
				if (opticsChild && opticsParent)
					opticsChild->relativeLockTo(opticsParent);
			}
		}
		else
			qDebug() << " -> Unknown tag: " << child.tagName();

		child = child.nextSiblingElement();
	}
}

void GaussianBeamWindow::parseTargetBeam(const QDomElement& element)
{
	Beam targetBeam = *m_bench->targetBeam();
	parseBeam(element, targetBeam);
	m_bench->setTargetBeam(targetBeam);
}

#include <iostream>
using namespace std;

void GaussianBeamWindow::parseBeam(const QDomElement& element, Beam& beam)
{
	QDomElement child = element.firstChildElement();

	while (!child.isNull())
	{
		/// @todo showTargetBeam is missing
		if (child.tagName() == "waist")
			beam.setWaist(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "waistPosition")
			beam.setWaistPosition(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "wavelength")
			beam.setWavelength(child.text().toDouble());
		else if (child.tagName() == "index")
			beam.setIndex(child.text().toDouble());
		else if (child.tagName() == "M2")
			beam.setM2(child.text().toDouble());
		// The next tags are specific to target beams
		else if ((child.tagName() == "targetOverlap") || (child.tagName() == "minOverlap"))
			m_bench->setTargetOverlap(child.text().toDouble());
		else if (child.tagName() == "targetOrientation")
			m_bench->setTargetOrientation(Orientation(child.text().toInt()));
		else
			qDebug() << " -> Unknown tag in parseBeam: " << child.tagName();
		child = child.nextSiblingElement();
	}
}

void GaussianBeamWindow::parseFit(const QDomElement& element)
{
	QDomElement child = element.firstChildElement();
	Fit* fit = m_bench->addFit(m_bench->nFit());

	while (!child.isNull())
	{
		if (child.tagName() == "name")
			fit->setName(child.text().toUtf8().data());
		else if (child.tagName() == "dataType")
			fit->setDataType(FitDataType(child.text().toInt()));
		else if (child.tagName() == "color")
			fit->setColor(child.text().toUInt());
		else if (child.tagName() == "orientation")
			fit->setOrientation(OrientationName::codedName.key(child.text()));
		else if (child.tagName() == "data")
		{
			QDomElement dataElement = child.firstChildElement();
			double position = 0.;
			bool added = false;
			while (!dataElement.isNull())
			{
				if (dataElement.tagName() == "position")
					position = dataElement.text().toDouble();
				else if (dataElement.tagName() == "value")
				{
					double value = dataElement.text().toDouble();
					Orientation orientation = OrientationName::codedName.key(dataElement.attribute("orientation"));
					if (added)
						fit->setData(fit->size() - 1, position, value, orientation);
					else
					{
						fit->addData(position, value, orientation);
						added = true;
					}
				}
				else
					qDebug() << " -> Unknown tag: " << dataElement.tagName();
				dataElement = dataElement.nextSiblingElement();
			}
		}
		else
			qDebug() << " -> Unknown tag: " << child.tagName();
		child = child.nextSiblingElement();
	}
}

void GaussianBeamWindow::parseOptics(const QDomElement& element, QMap<int, Optics*>& opticsList, QMap<int, int>& lockTree)
{
	Optics* optics = 0;

	if (element.tagName() == OpticsName::codedName[CreateBeamType])
		optics = new CreateBeam(1., 1., 1., "");
	else if (element.tagName() == OpticsName::codedName[LensType])
		optics = new Lens(1., 1., "");
	else if (element.tagName() == OpticsName::codedName[FlatMirrorType])
		optics = new FlatMirror(1., "");
	else if (element.tagName() == OpticsName::codedName[CurvedMirrorType])
		optics = new CurvedMirror(1., 1., "");
	else if (element.tagName() == OpticsName::codedName[FlatInterfaceType])
		optics = new FlatInterface(1., 1., "");
	else if (element.tagName() == OpticsName::codedName[CurvedInterfaceType])
		optics = new CurvedInterface(1., 1., 1., "");
	else if (element.tagName() == OpticsName::codedName[DielectricSlabType])
		optics = new DielectricSlab(1., 1., 1., "");
	else if (element.tagName() == OpticsName::codedName[GenericABCDType])
		optics = new GenericABCD(1., 1., 1., 1., 1., 1., "");
	else
		qDebug() << " -> Unknown tag in parseOptics: " << element.tagName();

	if (!optics)
		return;

	int id = element.attribute("id").toInt();

	QDomElement child = element.firstChildElement();

	while (!child.isNull())
	{
		if (child.tagName() == "position")
			optics->setPosition(child.text().toDouble(), false);
		else if (child.tagName() == "angle")
			optics->setAngle(child.text().toDouble());
		else if (child.tagName() == "orientation")
			optics->setOrientation(OrientationName::codedName.key(child.text()));
		else if (child.tagName() == "name")
			optics->setName(child.text().toUtf8().data());
		else if (child.tagName() == "absoluteLock")
			optics->setAbsoluteLock(child.text().toInt() == 1 ? true : false);
		else if (child.tagName() == "relativeLockParent")
			lockTree[id] = child.text().toInt();
		else if (child.tagName() == "width")
			optics->setWidth(child.text().toDouble());
		else if (child.tagName() == "focal")
			dynamic_cast<Lens*>(optics)->setFocal(child.text().toDouble());
		else if (child.tagName() == "curvatureRadius")
			dynamic_cast<CurvedMirror*>(optics)->setCurvatureRadius(child.text().toDouble());
		else if (child.tagName() == "indexRatio")
			dynamic_cast<Dielectric*>(optics)->setIndexRatio(child.text().toDouble());
		else if (child.tagName() == "surfaceRadius")
			dynamic_cast<CurvedInterface*>(optics)->setSurfaceRadius(child.text().toDouble());
		else if (child.tagName() == "A")
			dynamic_cast<GenericABCD*>(optics)->setA(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "B")
			dynamic_cast<GenericABCD*>(optics)->setB(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "C")
			dynamic_cast<GenericABCD*>(optics)->setC(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "D")
			dynamic_cast<GenericABCD*>(optics)->setD(child.text().toDouble(), OrientationName::codedName.key(child.attribute("orientation")));
		else if (child.tagName() == "beam")
		{
			Beam inputBeam;
			parseBeam(child, inputBeam);
			dynamic_cast<CreateBeam*>(optics)->setBeam(inputBeam);
		}
		else
			qDebug() << " -> Unknown tag in parseOptics: " << child.tagName();

		child = child.nextSiblingElement();
	}

	opticsList[id] = optics;
	m_bench->addOptics(optics, m_bench->nOptics());
}

void GaussianBeamWindow::parseView(const QDomElement& element)
{
	QDomElement child = element.firstChildElement();

	while (!child.isNull())
	{
		if (child.tagName() == "horizontalRange")
			m_hOpticsView->setHorizontalRange(child.text().toDouble());
//		else if (child.tagName() == "verticalRange")
//			m_hOpticsView->setVerticalRange(child.text().toDouble());
		else if (child.tagName() == "origin")
			/// @todo vertical origin
			m_hOpticsView->setOrigin(QPointF(child.text().toDouble(), 0.));
		else if (child.tagName() == "showTargetBeam")
			showTargetBeam(child.text().toInt());
		else
			qDebug() << " -> Unknown tag: " << element.tagName();

		child = child.nextSiblingElement();
	}
}
