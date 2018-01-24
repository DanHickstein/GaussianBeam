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

#include "gui/GaussianBeamPlot.h"
#include "gui/GaussianBeamModel.h"
#include "src/GaussianBeam.h"

/////////////////////////////////////////////////
// GaussianBeamPlotData

GaussianBeamPlotData::GaussianBeamPlotData(GaussianBeamModel* model, YPlotType YData)
	: QwtData()
	, m_model(model)
	, m_YData(YData)
{}

QwtData* GaussianBeamPlotData::copy() const
{
	return new GaussianBeamPlotData(m_model, m_YData);
}

size_t GaussianBeamPlotData::size() const
{
	return 100;
}

double GaussianBeamPlotData::x(size_t i) const
{
	return double(i)/200.;
}

double GaussianBeamPlotData::y(size_t i) const
{
	const Beam& beam = m_model->beam(1);
	double z = x(i);


	if (m_YData == YPlotRadius)
		return beam.radius(z);
	else if (m_YData == YPlotCurvature)
		return beam.curvature(z);
	else if (m_YData == YPlotGouy)
		return beam.gouyPhase(z);
	else if (m_YData == YPlotWaist)
		return beam.waist();
	else if (m_YData == YPlotDivergence)
		return beam.divergence();
	else if (m_YData == YPlotRayleigh)
		return beam.rayleigh();

	return 0.;
}

/////////////////////////////////////////////////
// GaussianBeamPlot

GaussianBeamPlot::GaussianBeamPlot(QWidget* parent, GaussianBeamModel* model)
	: QwtPlot(parent)
	, m_model(model)
{
	setAxisTitle(xBottom, "x");
	setAxisTitle(yLeft, "y");
	setCanvasBackground(Qt::yellow);

	// Insert new curves
	QwtPlotCurve* curve = new QwtPlotCurve();
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);
	curve->setPen(QPen(Qt::red));
	curve->attach(this);

	// Create sin and cos data
	curve->setData(GaussianBeamPlotData(m_model, YPlotRadius));

	// Insert markers
/*
	//  ...a horizontal line at y = 0...
	QwtPlotMarker *mY = new QwtPlotMarker();
	mY->setLabel(QString::fromLatin1("y = 0"));
	mY->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
	mY->setLineStyle(QwtPlotMarker::HLine);
	mY->setYValue(0.0);
	mY->attach(this);

	//  ...a vertical line at x = 2 * pi
	QwtPlotMarker *mX = new QwtPlotMarker();
	mX->setLabel(QString::fromLatin1("x = 2 pi"));
	mX->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
	mX->setLineStyle(QwtPlotMarker::VLine);
	mX->setXValue(6.284);
	mX->attach(this);*/
}

