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

#ifndef GAUSSIANFIT_H
#define GAUSSIANFIT_H

#include "GaussianBeam.h"

#include <vector>

/**
* Type of data measured by the user for beam fitting
*/
enum FitDataType {Radius_e2 = 0, Diameter_e2, standardDeviation, FWHM, HWHM};

/**
* @class Fit
* Find the waist radius and position for a given set of radii measurement of a Gaussian beam
* It fits the given data with a linear fit, and finds the only hyperbola
* that is tangent to the resulting line.
* The fit orientation determines if the data are identical on both axis (Spherical),
* taken on one particular axis (Horizontal or Vertical) or independantly on both axis (Ellispoidal)
*/
class Fit
{
public:
	/// Constructor
	Fit(int nData = 0);
	/// Comparison operator
	bool operator==(const Fit& other) const;

public:
	/// @return the number of points in the fit
	int size() const { return m_positions.size(); }
	/// @return the user name given to the fit
	std::string name() const { return m_name; }
	/// Set the user name of the fit
	void setName(std::string name);
	/// @return the type of measured data
	FitDataType dataType() const { return m_dataType; }
	/// Set the type of mesured data
	void setDataType(FitDataType dataType);
	/// @return the orientation of the fit
	Orientation orientation() const { return m_orientation; }
	/// Set the orientation of the fit
	void setOrientation(Orientation orientation);
	/// @return the RGB color associated to the fit
	unsigned int color() const { return m_color; }
	/// Set the RGB color accociated to the fit
	void setColor(unsigned int color);
	/// @return the position of date number @p index
	double position(unsigned int index) const;
	/// @return the measured value number @p index
	double value(unsigned int index, Orientation orientation) const;
	/// @return the measured beam radius at 1/e² computed from the measured data
	double radius(unsigned int index, Orientation orientation) const;
	/// Add a data point @p value , measured at position @p position to the fit
	void addData(double position, double value, Orientation orientation);
	/// Set data point number @p index to @p value at position @p position
	void setData(unsigned int index, double position, double value, Orientation orientation);
	/// Remove data point number @p index
	void removeData(unsigned int index);
	/// Remove all data in the fit
	void clear();
	/// @return true if the entry @p index has at least a non zero value (H or V)
	bool nonZeroEntry(int index) const;
	/// @return true if a fit result is available on @p orientation
	bool fitAvailable(Orientation orientation) const;
	/**
	* Apply the fit result to beam @p beam
	* @return the fit residue
	* @note the given bema wavelength chosen as the fit wavelength
	*/
	double applyFit(Beam& beam) const;

// Signals
public:
	Utils::Signal<Fit*> changed;

private:
	/// @return the number of points with non zero measured value in the fit
	int nonZeroSize(Orientation orientation) const;
	/// Actually o the fit
	void fitBeam(double wavelength) const;
	double fitBeam(double wavelength, Orientation orientation) const;
	/// Non linear fit functions
	std::pair<Beam,double> nonLinearFit(const Beam& guessBeam) const;
	static void lm_evaluate_beam(const double* par, int m_dat, const void* data, double* fvec, int* info);
	/// Linear fit functions
	Beam linearFit(const std::vector<double>& positions, const std::vector<double>& radii, double wavelength) const;


private:
	// State variables. Remember to update the == operator when changind this list
	std::string m_name;
	FitDataType m_dataType;
	std::vector<double> m_positions;
	std::vector<std::pair<double, double> > m_values;
	unsigned int m_color;
	Orientation m_orientation;

	// Mutables
	mutable bool m_dirty;
	mutable Beam m_beam;
	mutable double m_lastWavelength;
	mutable double m_residue;
	mutable std::vector<double> m_tmpPositions;
	mutable std::vector<double> m_tmpRadii;
};


#endif
