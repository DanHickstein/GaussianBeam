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

#include "OpticsFunction.h"
#include "OpticsBench.h"
#include "Optics.h"

#include <vector>
#include <algorithm>

using namespace std;

OpticsFunction::OpticsFunction(const std::vector<Optics*>& optics, double wavelength)
	: Function()
	, m_optics(optics)
	, m_wavelength(wavelength)
{}

vector<Optics*> OpticsFunction::cloneOptics() const
{
	vector<Optics*> opticsClone;

	// Clone optics objects
	for (vector<Optics*>::const_iterator it = m_optics.begin(); it != m_optics.end(); it++)
		opticsClone.push_back((*it)->clone());

	// Rebuild the locking tree
	for (vector<Optics*>::const_iterator itChild = m_optics.begin(); itChild != m_optics.end(); itChild++)
		if ((*itChild)->relativeLockParent())
			for (vector<Optics*>::const_iterator itParent = m_optics.begin(); itParent != m_optics.end(); itParent++)
				if ((*itParent) == (*itChild)->relativeLockParent())
				{
					opticsClone[itChild - m_optics.begin()]->relativeLockTo(opticsClone[itParent - m_optics.begin()]);
					break;
				}

	return opticsClone;
}

Beam OpticsFunction::beam(const std::vector<double>& x) const
{
	/// @todo cache the cloned optics
	vector<Optics*> opticsClone = cloneOptics();

	for (unsigned int i = 0; i < ::min(opticsClone.size(), x.size()); i++)
		if (m_checkLock)
			opticsClone[i]->setPosition(x[i], true);
		else
			opticsClone[i]->setPosition(x[i], false);

	sort(opticsClone.begin() + 1, opticsClone.end(), less<Optics*>());

	Beam result;
	result.setWavelength(m_wavelength);

	for (vector<Optics*>::const_iterator it = opticsClone.begin(); it != opticsClone.end(); it++)
		result = (*it)->image(result);

	for (vector<Optics*>::const_iterator it = opticsClone.begin(); it != opticsClone.end(); it++)
		delete (*it);

	return result;
}

double OpticsFunction::value(const std::vector<double>& x) const
{
	return Beam::overlap(m_overlapBeam, beam(x));
}

vector<double> OpticsFunction::currentPosition() const
{
	vector<double> position;

	for (vector<Optics*>::const_iterator it = m_optics.begin(); it != m_optics.end(); it++)
		position.push_back((*it)->position());

	return position;
}
