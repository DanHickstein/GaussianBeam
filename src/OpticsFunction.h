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

#ifndef OPTICSFUNCTION_H
#define OPTICSFUNCTION_H

#include "Function.h"
#include "GaussianBeam.h"

class Optics;
class OpticsBench;

/**
* Optics function is a function which value is the overlap between a Gaussian beam
* produced by a set of optics and a given beam. Its arguments is the set of positions
* of all the optics.
*/
class OpticsFunction : public Function
{
public:
	OpticsFunction(const std::vector<Optics*>& optics, double wavelength);
	virtual ~OpticsFunction() {}

public:
	virtual double value(const std::vector<double>& x) const;
	/// @todo this should be private
	Beam beam(const std::vector<double>& x) const;
	std::vector<double> currentPosition() const;
	void setCheckLock(bool checkLock) { m_checkLock = checkLock; }
	void setOverlapBeam(const Beam& beam) { m_overlapBeam = beam; }

private:
	std::vector<Optics*> cloneOptics() const;

private:
	const std::vector<Optics*>& m_optics;
	double m_wavelength;
	bool m_checkLock;
	Beam m_overlapBeam;
};

#endif
