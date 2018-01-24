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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>

/**
* Generic class for multi-dimensionnal functions
*/
class Function
{
public:
	/// Constructor
	Function();
	virtual ~Function() {}

public:
	/// Evaluate the function point @p x
	virtual double value(const std::vector<double>& x) const = 0;
	/// Compute the function gradient at point @p x
	std::vector<double> gradient(const std::vector<double>& x) const;
	/// Compute the vector of second derivatives at point @p x
	std::vector<double> curvature(const std::vector<double>& x) const;
	/// Search the extremum of the function along a line that crosses point @p x and directed along @p u
	std::vector<double> lineExtremum(const std::vector<double>& x, const std::vector<double>& u, bool min) const;
	std::vector<double> lineMinimum(const std::vector<double>& x, const std::vector<double>& u) const { return lineExtremum(x, u, true); }
	std::vector<double> lineMaximum(const std::vector<double>& x, const std::vector<double>& u) const { return lineExtremum(x, u, false); }
	/// Search the local extremum around point @p x
	std::vector<double> localExtremum(const std::vector<double>& x, bool min) const;
	std::vector<double> localMinimum(const std::vector<double>& x) const { return localExtremum(x, true); }
	std::vector<double> localMaximum(const std::vector<double>& x) const { return localExtremum(x, false); }
	/// Search the absolute extremum by random search
	std::vector<double> absoluteExtremum(bool min) const;
	std::vector<double> absoluteMinimum() const { return absoluteExtremum(true); }
	std::vector<double> absoluteMaximum() const { return absoluteExtremum(false); }
	/// Indicate whether the above search function succeded or not
	bool optimizationSuccess() { return m_success; }

private:
	// Set search extremum type
	void setExtremumType(bool min) const { m_min = min; }
	// Set line search parameters: start point and direction
	void setLine(const std::vector<double>& point, const std::vector<double>& direction) const;
	// Parametric representation of a line, given line parameters defined by setLine
	std::vector<double> lineParametric(double x) const;
	// Value of the function along along a line this value is signed according to the type of extremum searched
	double lineValue(double x) const;
	// Line search algorithms
	std::vector<double> bracketMinimum(double start, double stop) const;
	double brent(const std::vector<double>& bracket) const;


private:
	mutable bool m_min, m_success;
	mutable std::vector<double> m_linePoint;
	mutable std::vector<double> m_lineDirection;
};

#endif
