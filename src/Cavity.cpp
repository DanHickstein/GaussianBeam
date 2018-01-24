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

#include "Cavity.h"
#include "OpticsBench.h"

#include <iostream>
#include <algorithm>

using namespace std;

Cavity::Cavity()
{
	m_closingFreeSpace = 0.;
	m_dirty = true;
}

void Cavity::addOptics(const ABCD* optics)
{
	if (!isOpticsInCavity(optics))
	{
		m_opticsList.push_back(optics);
		m_dirty = true;
	}
}

void Cavity::removeOptics(const ABCD* optics)
{
	m_opticsList.remove(optics);
	m_dirty = true;
}

bool Cavity::isOpticsInCavity(const ABCD* optics) const
{
	if (find(m_opticsList.begin(), m_opticsList.end(), optics) != m_opticsList.end())
		return true;

	return false;
}

void Cavity::computeMatrix() const
{
	if (!m_dirty)
		return;

	/// @todo sort optics

	// Compute cavity
	m_matrix = FreeSpace(0., 0.);
	list<const ABCD*>::const_iterator lastIt = m_opticsList.end();
	for (list<const ABCD*>::const_iterator it = m_opticsList.begin(); it != m_opticsList.end(); lastIt = it++)
	{
		if (lastIt != m_opticsList.end())
			m_matrix *= FreeSpace((*it)->position() - (*(lastIt))->endPosition(), (*lastIt)->endPosition());
		cerr << " Cavity ABCD added free space = " << m_matrix << endl;
		m_matrix *= *(*it);
		cerr << " Cavity ABCD added optics = " << m_matrix << endl;
	}
	// Free space that closes the cavity
	if (lastIt != m_opticsList.end())
		m_matrix *= FreeSpace(m_closingFreeSpace, (*lastIt)->endPosition());
		cerr << " Cavity ABCD added last free space = " << m_matrix << endl;

	m_dirty = false;
}

double Cavity::delta() const
{
	computeMatrix();
	return sqr(m_matrix.D(Spherical) - m_matrix.A(Spherical)) + 4.*m_matrix.B(Spherical)*m_matrix.C(Spherical);
}

bool Cavity::isStable() const
{
	if (m_opticsList.empty())
		return false;

	computeMatrix();

	/// @todo deal with orientation

	// Stability criterion from I don't know where...
	//return fabs((m_matrix.A(Spherical) + m_matrix.B(Spherical))/2.) < 1.;

	// Stability criterion: stable if the eigen beam q parameter of
	// the ABCD matrix has a non-zero imaginary part
	cerr << "Delta = " << delta() << endl;
	return delta() < 0.;
}

const Beam* Cavity::eigenBeam(double wavelength, int index) const
{
	computeMatrix();

	/// @todo deal with orientation
	/// @todo what is the index ?

	if (isStable() && (m_matrix.C(Spherical) != 0.))
		m_beam = Beam(complex<double>(0.5*(m_matrix.A(Spherical) - m_matrix.D(Spherical))/m_matrix.C(Spherical),
		              0.5*sqrt(-delta())/m_matrix.C(Spherical)),
		              m_opticsList.front()->position(), wavelength, 1.0, 1.0);

	/// @todo reimplement checks
/*	if (!isStable() ||
	   (index < m_firstCavityIndex) ||
	   (m_ringCavity && (index >= m_lastCavityIndex)) ||
	   (!m_ringCavity && (index > m_lastCavityIndex)))
		return Beam();
*/

//	for (int i = m_firstCavityIndex; i <= index; i++)
//		beam = m_bench.optics(i)->image(beam);

	return &m_beam;
}
