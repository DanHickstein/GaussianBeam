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

#include "GaussianFit.h"
#include "Statistics.h"
#include "lmmin.h"

#include <iostream>
#include <cmath>

using namespace std;

Fit::Fit(int nData)
{
	m_dirty = true;
	m_lastWavelength = 0.;
	m_dataType = Radius_e2;
	m_color = 0;
	m_orientation = Spherical;

	for (int i = 0; i < nData; i++)
		addData(0., 0., Spherical);
}

int Fit::nonZeroSize(Orientation orientation) const
{
	int result = 0;

	for (int i = 0; i < size(); i++)
		if (radius(i, orientation) > Utils::epsilon)
			result++;

	return result;
}

bool Fit::fitAvailable(Orientation orientation) const
{
	if (orientation == Spherical)
		return (nonZeroSize(Spherical) > 1);

	if ((orientation == Horizontal) || (orientation == Vertical))
	{
		if (m_orientation == quadrature(orientation))
			return false;
		return (nonZeroSize(orientation) > 1);
	}

	cerr << "Wrong orientation argument in fitAvailable(" << orientation << ")" << endl;
	return false;
}

bool Fit::nonZeroEntry(int index) const
{
	if (m_orientation == Spherical)
		return value(index, Spherical) != 0.;
	if ((m_orientation == Horizontal) || (m_orientation == Vertical))
		return value(index, m_orientation) != 0.;
	if (m_orientation == Ellipsoidal)
		return ((value(index, Horizontal) != 0.) || (value(index, Vertical) != 0.));

	return false;
}

void Fit::setName(std::string name)
{
	m_name = name;
	changed.emit(this);
}

void Fit::setDataType(FitDataType dataType)
{
	m_dataType = dataType;
	m_dirty = true;
	changed.emit(this);
}

void Fit::setOrientation(Orientation orientation)
{
	m_orientation = orientation;
	m_dirty = true;
	changed.emit(this);
}

void Fit::setColor(unsigned int color)
{
	m_color = color;
	changed.emit(this);
}

double Fit::position(unsigned int index) const
{
	return m_positions[index];
}

/*
* Orientation table
*  orientation in the argument in columns
*  fit orientation in rows
*    f\a S H V E
*    -----------
*    S | H H V X
*    H | X H X X
*    V | X X V X
*    E | X H V X
*/
double Fit::value(unsigned int index, Orientation orientation) const
{
	if ((m_orientation == Spherical) &&
	    (m_values[index].first != m_values[index].second))
	{
		cerr << "Fit mismatch in horizontal and vertical values " << m_values[index].first << " " << m_values[index].second << endl;
		return 0;
	}

	if (((orientation == Spherical ) && (m_orientation == Spherical)) ||
	    ((orientation == Horizontal) && (m_orientation != Vertical )))
		return m_values[index].first;

	if ((orientation == Vertical) && (m_orientation != Horizontal))
		return m_values[index].second;

	cerr << "Wrong orientation argument in Fit::value(" << index << "," << orientation << ")" << endl;
	return 0.;
}

double Fit::radius(unsigned int index, Orientation orientation) const
{
	if (m_dataType == Radius_e2)
		return value(index, orientation);
	else if (m_dataType == Diameter_e2)
		return value(index, orientation)/2.;
	else  if (m_dataType == standardDeviation)
		return value(index, orientation)*2.;
	else  if (m_dataType == FWHM)
		return value(index, orientation)/sqrt(2.*log(2.));
	else  if (m_dataType == HWHM)
		return value(index, orientation)*sqrt(2./log(2.));

	return 0.;
}

void Fit::addData(double position, double value, Orientation orientation)
{
	setData(size(), position, value, orientation);
}

/*
* Orientation table
*  orientation in the argument in columns
*  fit orientation in rows
*    f\a S  H V E
*    -----------
*    S | HV X X X
*    H | H  H X X
*    V | V  X V X
*    E | HV H V X
*/
void Fit::setData(unsigned int index, double position, double value, Orientation orientation)
{
	if ((orientation == Ellipsoidal) ||
	    ((m_orientation == Spherical ) && (orientation != Spherical )) ||
	    ((m_orientation == Horizontal) && (orientation == Vertical  )) ||
	    ((m_orientation == Vertical  ) && (orientation == Horizontal)))
	{
		cerr << "Wrong orientation argument in Fit::setValue(" << index << "," << orientation << ")" << endl;
		return;
	}


	if (m_positions.size() <= index)
	{
		m_positions.resize(index + 1);
		m_values.resize(index + 1);
	}

	if ((orientation == Horizontal) || ((orientation == Spherical) && (m_orientation != Vertical)))
		m_values[index].first = value;

	if ((orientation == Vertical)   || ((orientation == Spherical) && (m_orientation != Horizontal)))
		m_values[index].second = value;

	m_positions[index] = position;
	m_dirty = true;

	changed.emit(this);
}

void Fit::removeData(unsigned int index)
{
	m_positions.erase(m_positions.begin() + index);
	m_values.erase(m_values.begin() + index);
	m_dirty = true;

	changed.emit(this);
}

void Fit::clear()
{
	m_positions.clear();
	m_values.clear();
	m_dirty = true;

	changed.emit(this);
}

double Fit::applyFit(Beam& beam) const
{
	fitBeam(beam.wavelength());

	if ((m_orientation != Vertical) && (nonZeroSize(Horizontal) >= 2))
	{
		beam.setWaist(m_beam.waist(Horizontal), Horizontal);
		beam.setWaistPosition(m_beam.waistPosition(Horizontal), Horizontal);
	}
	if ((m_orientation != Horizontal) && (nonZeroSize(Vertical) >= 2))
	{
		beam.setWaist(m_beam.waist(Vertical), Vertical);
		beam.setWaistPosition(m_beam.waistPosition(Vertical), Vertical);
	}
	return m_residue;
}

/////////////////////////////////////////////////
// Non linear fit functions

/**
* @p par   input array. At the end of the minimization, it contains the approximate solution vector.
* @p m_dat positive integer input variable set to the number of functions.
* @p fvec  is an output array of length m_dat which contains the function values the square sum of which ought to be minimized.
* @p data  user data. Here null
* @p info  integer output variable. If set to a negative value, the minimization procedure will stop.
*/
void Fit::lm_evaluate_beam(const double* par, int /*m_dat*/, const void* data, double* fvec, int* /*info*/)
{
	const Fit* fit = static_cast<const Fit*>(data);

	int j = 0;
	/// @todo index = 1., M2 = 1. ?
	Beam beam(par[0], par[1], fit->m_lastWavelength, 1., 1.);

	for (unsigned int i = 0; i < fit->m_tmpRadii.size(); i++)
			fvec[j++] = fit->m_tmpRadii[i] - beam.radius(fit->m_tmpPositions[i]);

	// if <parameters drifted away> { *info = -1; }
}

pair<Beam, double> Fit::nonLinearFit(const Beam& guessBeam) const
{
	const int nPar = 2;
	double par[nPar] = {guessBeam.waist(), guessBeam.waistPosition()};

	lm_control_struct control = lm_control_double;
	lm_status_struct status;
	lmmin(nPar, par, m_tmpRadii.size(), (void*)(this), Fit::lm_evaluate_beam, &control, &status, NULL);

	pair<Beam, double> result(guessBeam, status.fnorm);
	result.first.setWaist(par[0]);
	result.first.setWaistPosition(par[1]);

	return result;
}

/////////////////////////////////////////////////
// Linear fit functions

Beam Fit::linearFit(const vector<double>& positions, const vector<double>& radii, double wavelength) const
{
	Statistics stats(positions, radii);

	// Some point whithin the fit
	const double z = stats.meanX;
	// beam radius at z
	const double fz = stats.m*z + stats.p;
	// derivative of the beam radius at z
	const double fpz = stats.m;
	// (z - zw)/z0  (zw : position of the waist, z0 : Rayleigh range)
	const double alpha = M_PI*fz*fpz/wavelength;
	/// @todo index = 1., M2 = 1. ?
	Beam result(fz/sqrt(1. + sqr(alpha)), 0., wavelength, 1., 1.);
	result.setWaistPosition(z - result.rayleigh()*alpha);

	return result;
}

/////////////////////////////////////////////////
// Actually do the fit

void Fit::fitBeam(double wavelength) const
{
	if (!m_dirty && (wavelength == m_lastWavelength))
		return;

	m_lastWavelength = wavelength;

	if (m_orientation != Ellipsoidal)
		m_residue = fitBeam(wavelength, m_orientation);
	else
	{
		double r1 = fitBeam(wavelength, Horizontal);
		double r2 = fitBeam(wavelength, Vertical);
		m_residue = sqrt(r1*r2);
	}

	m_dirty = false;
}

double Fit::fitBeam(double wavelength, Orientation orientation) const
{
	if (nonZeroSize(orientation) < 2)
		return 1.;

	// Gather all non zero data
	m_tmpPositions.clear();
	m_tmpRadii.clear();
	for (int i = 0; i < size(); i++)
		if (radius(i, orientation) > Utils::epsilon)
		{
			m_tmpPositions.push_back(position(i));
			m_tmpRadii.push_back(radius(i, orientation));
		}

	pair<Beam, double> result, tmpResult;

	// 1/ linear fit with all data
	result.first = linearFit(m_tmpPositions, m_tmpRadii, wavelength);
	// 2/ non linear fit with the linear fit result as initial guess
	result = nonLinearFit(result.first);
	// 3/ Try a linear fit with the first two points + non linear fit on all points
	vector<double> positions, radii;
	for (int i = 0; (i < size()) && (positions.size() < 2); i++)
		if (radius(i, orientation) > Utils::epsilon)
		{
			positions.push_back(position(i));
			radii.push_back(radius(i, orientation));
		}
	tmpResult.first = linearFit(positions, radii, wavelength);
	tmpResult = nonLinearFit(tmpResult.first);
	if (tmpResult.second < result.second)
		result = tmpResult;
	// 4/ Try a linear fit with the last two points + non linear fit on all points
	positions.clear();
	radii.clear();
	for (int i = size()-1; (i >= 0) && (positions.size() < 2); i--)
		if (radius(i, orientation) > Utils::epsilon)
		{
			positions.push_back(position(i));
			radii.push_back(radius(i, orientation));
		}
	tmpResult.first = linearFit(positions, radii, wavelength);
	tmpResult = nonLinearFit(tmpResult.first);
	if (tmpResult.second < result.second)
		result = tmpResult;

	m_beam.setWaist(result.first.waist(), orientation);
	m_beam.setWaistPosition(result.first.waistPosition(), orientation);

	return result.second;
}

bool Fit::operator==(const Fit& other) const
{
	return (m_name        == other.m_name       ) &&
	       (m_dataType    == other.m_dataType   ) &&
	       (m_positions   == other.m_positions  ) &&
	       (m_values      == other.m_values     ) &&
	       (m_color       == other.m_color      ) &&
	       (m_orientation == other.m_orientation);
}
