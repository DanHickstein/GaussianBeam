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

#ifndef GAUSSIANBEAMPLOT_H
#define GAUSSIANBEAMPLOT_H

#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_data.h>

enum YPlotType {YPlotWaist, YPlotDivergence, YPlotRayleigh, YPlotRadius, YPlotCurvature, YPlotGouy};

class GaussianBeamModel;

class GaussianBeamPlotData : public QwtData
{
public:
	GaussianBeamPlotData(GaussianBeamModel* model, YPlotType YData);

public:
	virtual QwtData *copy() const;
	virtual size_t size() const;
	virtual double x(size_t i) const;
	virtual double y(size_t i) const;

private:
	GaussianBeamModel* m_model;
	YPlotType m_YData;
};

class GaussianBeamPlot : public QwtPlot
{
public:
	GaussianBeamPlot(QWidget* parent, GaussianBeamModel* model);

private:
	GaussianBeamModel* m_model;
};

#endif
