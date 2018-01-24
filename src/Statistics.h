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

#ifndef STATISTICS_H
#define STATISTICS_H

#include "Utils.h"

#include <vector>

class Statistics
{
public:
	/// First order momenta
	double meanX, meanY;
	/// Second order momenta
	double VX, VY, XY;
	/// Correlation coefficient (squared)
	double rho2;
	/// Linear fit coefficients y = m*x + p
	double m, p;

public:
	Statistics(const std::vector<double>& X, const std::vector<double>& Y)
	{
		unsigned int size = X.size();
		meanX = meanY = VX = VY = XY = 0.0;

		for(unsigned int i = 0; i < size; i++)
		{
				meanX += X[i];
				meanY += Y[i];
				VX += sqr(X[i]);
				VY += sqr(Y[i]);
				XY += X[i]*Y[i];
		}

		// Mean
		meanX /= size;
		meanY /= size;
		// Correlations
		XY = (XY - meanX*meanY*size)/(size - 1.0);
		// Variance
		VX = (VX-sqr(meanX)*size)/(size-1.0);
		VY = (VY-sqr(meanY)*size)/(size-1.0);
		// Correlation coefficient
		rho2 = sqr(XY)/(VX * VY);
		// Linear Fit
		m = XY/VX;
		p = meanY - m*meanX;
	}
};

#endif
