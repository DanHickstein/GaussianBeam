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

#ifndef CAVITY_H
#define CAVITY_H

#include "GaussianBeam.h"
#include "Optics.h"

/**
* This class defines a cavity by a set of optics. It can
* tell whether the cavity is stable or not and give the eigen modes
*/
class Cavity
{
public:
	/// Constructor
	Cavity();

public:
	/// Add the optics @p optics to the cavity
	void addOptics(const ABCD* optics);
	/// Remove the optics @p optics from the cavity
	void removeOptics(const ABCD* optics);
	/// Check if a given optics is in the cavity
	bool isOpticsInCavity(const ABCD* optics) const;
	/// @return the freespace interval that closes the cavity
	double closingFreeSpace() const { return m_closingFreeSpace; }
	/// Set the freespace interval that closes the cavity
	void setClosingFreeSpace(double closingFreeSpace) { m_closingFreeSpace = closingFreeSpace; }
	/// @return true if there exist a Gaussian cavity eigen-mode
	bool isStable() const;
	/**
	* @return the cavity eigen-mode
	* @p wavelength wavelength of the eigen-mode
	* @p index return the beam as it is after the @p index cavity optics
	*/
	const Beam* eigenBeam(double wavelength, int index) const;

private:
	void computeMatrix() const;
	double delta() const;

private:
	std::list<const ABCD*> m_opticsList;
	double m_closingFreeSpace;

	mutable GenericABCD m_matrix;
	mutable Beam m_beam;
	mutable bool m_dirty;
};

#endif
