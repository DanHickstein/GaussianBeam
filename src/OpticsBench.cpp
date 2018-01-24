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

#include "OpticsBench.h"
#include "GaussianFit.h"
#include "OpticsFunction.h"
#include "Utils.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace std;
using namespace Utils;

#define emit(x) for (list<OpticsBenchEventListener*>::iterator it = m_listeners.begin(); it !=  m_listeners.end(); (*it++)->x)

/*
OpticsTreeItem::OpticsTreeItem(Optics* optics, OpticsTreeItem* parent)
{
	m_optics = optics;
	m_parent = 0;
	m_child = 0;
	m_image = new Beam();
	insert(parent);
}

Beam* OpticsTreeItem::axis()
{
	if (m_parent == 0)
		return 0;

	return m_parent->image();
}

void OpticsTreeItem::move(OpticsTreeItem* parent)
{
	if (parent == 0)
		return;

	remove();
	insert(parent);
}

void OpticsTreeItem::insert(OpticsTreeItem* parent)
{
	if (parent == 0)
		return;

	// Attach old parent's child to this item
	OpticsTreeItem* child = parent->m_child;
	m_child = child;
	if (child)
		child->m_parent = this;

	// Attach to the parent
	m_parent = parent;
	m_parent->m_child = this;
}

void OpticsTreeItem::remove()
{
	m_parent->m_child = m_child;
	if (m_child)
		m_child->m_parent = m_parent;

	m_parent = 0;
	m_child = 0;
}

void OpticsTreeItem::sort()
{
	double position = optics()->position();
	for (OpticsTreeItem* newParent = parent; (newParent != 0) && (newParent->optics()->position()
	while (
}

*/

/////////////////////////////////////////////////
// OpticsBench

OpticsBench::OpticsBench()
{
	m_opticsPrefix[LensType]            = "L";
	m_opticsPrefix[FlatMirrorType]      = "M";
	m_opticsPrefix[CurvedMirrorType]    = "R";
	m_opticsPrefix[FlatInterfaceType]   = "I";
	m_opticsPrefix[CurvedInterfaceType] = "C";
	m_opticsPrefix[GenericABCDType]     = "G";
	m_opticsPrefix[DielectricSlabType]  = "D";

	m_beamSpherical = true;
	m_fitSpherical = true;
	m_1D = true;

	resetDefaultValues();
}

OpticsBench::~OpticsBench()
{
	// Delete optics
	for (vector<Optics*>::iterator it = m_optics.begin(); it != m_optics.end(); it++)
		delete (*it);

	// Delelte beams
	for (vector<Beam*>::iterator it = m_beams.begin(); it != m_beams.end(); it++)
		delete (*it);
}

void OpticsBench::resetDefaultValues()
{
	m_wavelength = 461e-9;
	/// @todo better vertical boundaries
	m_boundary = Rect(-0.1, -0.2, 0.7, 0.2);

	m_targetOverlap = 0.95;
	m_targetOrientation = Spherical;
	/// @todo index = 1. ? target = 1.
	setTargetBeam(Beam(0.000150, 0.6, wavelength(), 1., 1.));
}

void OpticsBench::clear()
{
	// Clear objects
	removeOptics(0, nOptics());
	removeFits(0, nFit());

	// Reset to default values
	resetDefaultValues();
}

void OpticsBench::populateDefault()
{
	CreateBeam* inputBeam = new CreateBeam(180e-6, 10e-3, 1., "w0");
	inputBeam->setAbsoluteLock(true);
	addOptics(inputBeam, 0);

	addFit(0, 3);
}

void OpticsBench::setModified(bool modified)
{
	if (modified == m_modified)
		return;

	m_modified = modified;
	emit(onOpticsBenchModified());
}

bool OpticsBench::isSpherical() const
{
	return m_beamSpherical && m_fitSpherical && (m_targetOrientation == Spherical);
}

bool OpticsBench::is1D() const
{
	return m_1D;
}

void OpticsBench::registerEventListener(OpticsBenchEventListener* listener)
{
	m_listeners.push_back(listener);
}

/////////////////////////////////////////////////
// Cavity

void OpticsBench::detectCavities()
{
	// Cavity detection criterions for a given optics i to close a cavity with a previous beam j
	// - The optics is on the beam optical axis
	// - The optics is in the beam range
	// - The beam is copropagating with the optics image
	// - The beam is NOT copropagating with the optics antecedent.
/*
	cerr << "Detecting cavity" << endl;
	bool cavityDetected = false;

	for (int i = 2; i < nOptics(); i++)
		for (int j = 0; j < i-1; j++)
		{
			Optics* optics = m_optics[i];
			Beam* beam = m_beams[j];
			Point opticsCoordinates = beam->beamCoordinates(m_beams[i-1]->absoluteCoordinates(optics->position()));
			cerr << "Checking cavity " << i << " and " << j << endl;
			cerr << (fabs(opticsCoordinates.y()) < Utils::epsilon) << endl;
			cerr << (opticsCoordinates.x() >= beam->start()) << endl;
			cerr << (opticsCoordinates.x() <= beam->stop()) << endl;
			cerr << Beam::copropagating(*beam, *m_beams[i]) << endl;
			cerr << !Beam::copropagating(*beam, *m_beams[i-1]) << endl;
			if (   (fabs(opticsCoordinates.y()) < Utils::epsilon)
			    && (opticsCoordinates.x() >= beam->start())
			    && (opticsCoordinates.x() <= beam->stop())
			    &&  Beam::copropagating(*beam, *m_beams[i])
			    && !Beam::copropagating(*beam, *m_beams[i-1]))
			{
				cavityDetected = true;
				Cavity cavity;
				for (int o = j + 1; o <= i; o++)
				{
					if (m_optics[o]->isABCD())
					{
						cavity.addOptics(dynamic_cast<ABCD*>(m_optics[o]));
						cerr << "   adding optics " << o << endl;
					}
				}
				cerr << m_optics[j]->position() << " - " << opticsCoordinates.x() << endl;
				double closingFreeSpace = m_optics[j+1]->position() - opticsCoordinates.x();
				cerr << closingFreeSpace << endl;
				cavity.setClosingFreeSpace(closingFreeSpace);
				cerr << " Detected cavity around beams " << i << " and " << j << endl;
				cerr << "Stability : " << cavity.isStable() << endl;
				cerr << "EigenBeam : " << *cavity.eigenBeam(wavelength(), 0) << endl;
			}
		}
*/
}

/////////////////////////////////////////////////
// Fit

void OpticsBench::checkFitSpherical()
{
	bool spherical = true;
	for (vector<Fit*>::iterator it = m_fits.begin(); it != m_fits.end(); it++)
		if ((*it)->orientation() != Spherical)
		{
			spherical = false;
			break;
		}

	bool wasSpherical = isSpherical();
	m_fitSpherical = spherical;
	if (wasSpherical ^ isSpherical())
		emit(onOpticsBenchSphericityChanged());
}

int OpticsBench::nFit() const
{
	return m_fits.size();
}

Fit* OpticsBench::addFit(unsigned int index, int nData)
{
	// Find the best name for the fit
	int minFit = 0;
	for (vector<Fit*>::iterator it = m_fits.begin(); it != m_fits.end(); it++)
		if ((*it)->name().substr(0, 3) == "Fit")
		{
			int n = atoi((*it)->name().substr(3).c_str());
			if (n > minFit)
				minFit = n;
		}
	stringstream stream;
	stream << "Fit" << minFit + 1 << ends;
	string name;
	stream >> name;

	// Add a new fit
	Fit* fit = new Fit(nData);
	fit->setName(name);
	fit->changed.connect(this, &OpticsBench::notifyFitChanged);
	m_fits.insert(m_fits.begin() + index, fit);
	checkFitSpherical();

	emit(onOpticsBenchFitAdded(index));
	setModified(true);

	return fit;
}

Fit* OpticsBench::fit(unsigned int index)
{
	if (index < m_fits.size())
		return m_fits[index];

	return 0;
}

void OpticsBench::removeFit(unsigned int index)
{
	removeFits(index, 1);
}

void OpticsBench::removeFits(unsigned int startIndex, int n)
{
	m_fits.erase(m_fits.begin() + startIndex, m_fits.begin() + startIndex + n);
	checkFitSpherical();

	emit(onOpticsBenchFitsRemoved(startIndex, n));
	setModified(true);
}

void OpticsBench::notifyFitChanged(Fit* fit)
{
	int index = 0;

	for (vector<Fit*>::iterator it = m_fits.begin(); it != m_fits.end(); it++)
		if ((*it) == fit)
		{
			index = it - m_fits.begin();
			break;
		}

	checkFitSpherical();

	emit(onOpticsBenchFitDataChanged(index));
	setModified(true);
}

/////////////////////////////////////////////////
// Parameters

void OpticsBench::setWavelength(double wavelength)
{
	m_wavelength = wavelength;
	m_targetBeam.setWavelength(m_wavelength);
	setTargetBeam(m_targetBeam);
	computeBeams();

	emit(onOpticsBenchWavelengthChanged());
	setModified(true);
}

void OpticsBench::setLeftBoundary(double leftBoundary)
{
	if (leftBoundary < m_boundary.x2())
		m_boundary.setX1(leftBoundary);

	updateExtremeBeams();

	emit(onOpticsBenchBoundariesChanged());
	setModified(true);
}

void OpticsBench::setRightBoundary(double rightBoundary)
{
	if (rightBoundary > m_boundary.x1())
		m_boundary.setX2(rightBoundary);

	updateExtremeBeams();

	emit(onOpticsBenchBoundariesChanged());
	setModified(true);
}

/////////////////////////////////////////////////
// Optics

int OpticsBench::opticsIndex(const Optics* optics) const
{
	/// @todo implement a lookup table
	for (vector<Optics*>::const_iterator it = m_optics.begin(); it != m_optics.end(); it++)
		if (*it == optics)
			return it - m_optics.begin();

	cerr << "Error : looking for an optics that is no more in the optics list" << endl;
	return -1;
}

void OpticsBench::addOptics(Optics* optics, int index)
{
/*	OpticsTreeItem* parent = index < m_opticsTree.size() ? &m_opticsTree[index] : 0;
	m_opticsTree.insert(m_opticsTree.begin() + index, OpticsTreeItem(optics, parent));
*/
	m_optics.insert(m_optics.begin() + index,  optics);
	m_beams.insert(m_beams.begin() + index, new Beam(wavelength()));

	emit(onOpticsBenchOpticsAdded(index));
	computeBeams(index);
}

void OpticsBench::addOptics(OpticsType opticsType, int index)
{
	// Find a suitable name
	int minOptics = 0;
	for (vector<Optics*>::iterator it = m_optics.begin(); it != m_optics.end(); it++)
		if ((*it)->name().substr(0, m_opticsPrefix[opticsType].size()) == m_opticsPrefix[opticsType])
		{
			int n = atoi((*it)->name().substr(m_opticsPrefix[opticsType].size()).c_str());
			if (n > minOptics)
				minOptics = n;
		}
	stringstream stream;
	stream << m_opticsPrefix[opticsType] << minOptics + 1 << ends;
	string name;
	stream >> name;

	Optics* optics;

	if (opticsType == LensType)
		optics = new Lens(0.1, 0.0, name);
	else if (opticsType == FlatMirrorType)
	{
		optics = new FlatMirror(0.0, name);
		optics->setAngle(M_PI);
	}
	else if (opticsType == CurvedMirrorType)
	{
		optics = new CurvedMirror(0.05, 0.0, name);
		optics->setAngle(M_PI);
	}
	else if (opticsType == FlatInterfaceType)
		optics = new FlatInterface(1.5, 0.0, name);
	else if (opticsType == CurvedInterfaceType)
		optics = new CurvedInterface(0.1, 1.5, 0.0, name);
	else if (opticsType == DielectricSlabType)
		optics = new DielectricSlab(1.5, 0.1, 0.0, name);
	else if (opticsType == GenericABCDType)
		optics = new GenericABCD(1.0, 0.2, 0.0, 1.0, 0.1, 0.0, name);
	else
		return;

	if (index > 0)
		optics->setPosition(OpticsBench::optics(index-1)->position() + 0.05, false);

	addOptics(optics, index);
}

void OpticsBench::removeOptics(int index, int count)
{
	for (int i = index; i < index + count; i++)
	{
		delete m_optics[index];
		m_optics.erase(m_optics.begin() + index);
		delete m_beams[index];
		m_beams.erase(m_beams.begin() + index);
	}

	emit(onOpticsBenchOpticsRemoved(index, count));
	computeBeams(index);
}

int OpticsBench::setOpticsPosition(int index, double position)
{
	Optics* movedOptics = m_optics[index];

	// Check that the optics does not exit the optical bench
	/// @todo : this function for now only works for a 1D bench
	if ((position < m_boundary.x1()) || (position > m_boundary.x2()))
		return index;

	// Check that the optics does not overlap with another optics
	double start1 = position;
	double stop1  = position + movedOptics->width();
	for (vector<Optics*>::iterator it = m_optics.begin(); it != m_optics.end(); it++)
		if ((*it) != movedOptics)
		{
			double start2 = (*it)->position();
			double stop2  = (*it)->endPosition();
			if (((start2 >= start1) && (start2 <= stop1)) ||
				((stop2  >= start1) && (stop2  <= stop1)) ||
				((start1 >= start2) && (start1 <= stop2)) ||
				((stop1  >= start2) && (stop1  <= stop2)))
				return index;
		}

	// Move the optics
	m_optics[index]->setPosition(position, true);
	sort(m_optics.begin() + 1, m_optics.end(), less<Optics*>());
	computeBeams();

	// Return the new index of the optics
	for (vector<Optics*>::iterator it = m_optics.begin(); it != m_optics.end(); it++)
		if ((*it) == movedOptics)
			return it - m_optics.begin();

	return index;
}

void OpticsBench::printTree()
{
	cerr << "Locking tree" << endl;

	for (vector<Optics*>::iterator it = m_optics.begin(); it != m_optics.end(); it++)
	{
		cerr << " Optics " << (*it)->name() << endl;
		if ((*it)->absoluteLock())
			cerr << "  absolutely locked" << endl;
		if (const Optics* parent = (*it)->relativeLockParent())
			cerr << "  parent = " << parent->name() << endl;
		for (list<Optics*>::const_iterator cit = (*it)->relativeLockChildren().begin(); cit != (*it)->relativeLockChildren().end(); cit++)
			cerr << "  child = " << (*cit)->name() << endl;
	}
}

void OpticsBench::opticsPropertyChanged(int /*index*/)
{
	computeBeams();
}

/////////////////////////////////////////////////
// Beams

const Beam* OpticsBench::beam(int index) const
{
	return m_beams[index];
}

void OpticsBench::setInputBeam(const Beam& beam)
{
	if (m_optics.size() == 0)
		addOptics(new CreateBeam(180e-6, 10e-3, 1., "w0"), 0);

	CreateBeam* createBeam = dynamic_cast<CreateBeam*>(m_optics[0]);
	createBeam->setBeam(beam);
	computeBeams();
}

void OpticsBench::setBeam(const Beam& beam, int index)
{
	*m_beams[index] = beam;
	computeBeams(index, true);
}

const Beam* OpticsBench::axis(int index) const
{
	if (index == 0)
		return beam(0);
	else
		return beam(index - 1);
}

double OpticsBench::sensitivity(int index) const
{
	return m_sensitivity[index];
}

void OpticsBench::updateExtremeBeams()
{
	if (nOptics() > 0)
	{
		m_beams[0]->setStart(m_beams[0]->rectangleIntersection(m_boundary)[0]);
		m_beams[nOptics()-1]->setStop(m_beams[nOptics()-1]->rectangleIntersection(m_boundary)[1]);
	}

	vector<double> targetBoundaries = m_targetBeam.rectangleIntersection(m_boundary);
	m_targetBeam.setStart(targetBoundaries[0]);
	m_targetBeam.setStop(targetBoundaries[1]);
}

void OpticsBench::computeBeams(int changedIndex, bool backwards)
{
	if (m_optics.size() == 0)
		return;

	if (backwards)
	{
		for (int i = changedIndex + 1; i < nOptics(); i++)
			*m_beams[i] = m_optics[i]->image(*m_beams[i-1]);
		for (int i = changedIndex - 1; i >= 0; i--)
			*m_beams[i] = m_optics[i+1]->antecedent(*m_beams[i+1]);
		CreateBeam* createBeam = dynamic_cast<CreateBeam*>(m_optics[0]);
		createBeam->setBeam(*m_beams[0]);
	}
	else
	{
		if (changedIndex == 0)
		{
			Beam beam(wavelength());
			*m_beams[0] = m_optics[0]->image(beam);
		}

		for (int i = ::max(changedIndex, 1); i < nOptics(); i++)
			*m_beams[i] = m_optics[i]->image(*m_beams[i-1]);
	}

	for (int i = 0; i < nOptics(); i++)
	{
		if (i != 0)
			m_beams[i]->setStart(m_optics[i]->position() + m_optics[i]->width());
		if (i < nOptics()-1)
			m_beams[i]->setStop(m_optics[i+1]->position());
	}
	updateExtremeBeams();

	OpticsFunction function(m_optics, m_wavelength);
	function.setOverlapBeam(*m_beams.back());
	function.setCheckLock(false);

	m_sensitivity = function.curvature(function.currentPosition())/2.;

	bool spherical = true;
	for (int i = 0; i < nOptics(); i++)
		if (m_optics[i]->orientation() != Spherical)
		{
			spherical = false;
			break;
		}
	bool wasSpherical = isSpherical();
	m_beamSpherical = spherical;

	bool oneD = true;
	for (int i = 0; i < nOptics(); i++)
	{
		double angle = fmod(fabs(m_beams[i]->angle()), M_PI);
		if ((angle > Utils::epsilon) && (angle < M_PI - Utils::epsilon))
		{
			oneD = false;
			break;
		}
	}
	bool was1D = is1D();
	m_1D = oneD;

	detectCavities();

	if (wasSpherical ^ isSpherical())
		emit(onOpticsBenchSphericityChanged());

	if (was1D ^ is1D())
		emit(onOpticsBenchDimensionalityChanged());

	if (backwards)
		emit(onOpticsBenchDataChanged(0, nOptics()-1));
	else
		emit(onOpticsBenchDataChanged(changedIndex, nOptics()-1));

	setModified(true);
}

pair<Beam*, double> OpticsBench::closestPosition(const Point& point, int preferedSide) const
{
	double bestPosition = 0.;
	double bestDistance = 1e300;
	Beam* bestBeam = 0;

	for (int i = 0; i < nOptics(); i++)
	{
		Point coord = m_beams[i]->beamCoordinates(point);
		double newDistance = coord.y();

//		if (newDistance > bestDistance)
//			continue;

		if (coord.x() < m_beams[i]->start())
			newDistance = Utils::distance(point, m_beams[i]->absoluteCoordinates(m_beams[i]->start()));
		else if (coord.x() > m_beams[i]->stop())
			newDistance = Utils::distance(point, m_beams[i]->absoluteCoordinates(m_beams[i]->stop()));

		double rho = fabs(newDistance/bestDistance) - 1.;
		if ((rho < -epsilon) ||
			((fabs(rho) < epsilon) && (intSign(coord.y())*preferedSide > 0)))
		{
			bestPosition = coord.x();
			bestDistance = newDistance;
			bestBeam = m_beams[i];
		}
	}

	return pair<Beam*, double>(bestBeam, bestPosition);
}

/////////////////////////////////////////////////
// Magic waist

void OpticsBench::setTargetBeam(const Beam& beam)
{
	m_targetBeam = beam;
	m_targetBeam.setWavelength(m_wavelength);

	if ((m_targetBeam.orientation() == Ellipsoidal) && (m_targetOrientation == Spherical))
	{
		m_targetOrientation = m_targetBeam.orientation();
		emit(onOpticsBenchSphericityChanged());
	}

	emit(onOpticsBenchTargetBeamChanged());

	setModified(true);
}

void OpticsBench::setTargetOverlap(double targetOverlap)
{
	m_targetOverlap = targetOverlap;
	emit(onOpticsBenchTargetBeamChanged());

	setModified(true);
}

void OpticsBench::setTargetOrientation(Orientation orientation)
{
	if ((orientation != Spherical) && (orientation != Ellipsoidal))
		orientation = Spherical;

	if (orientation == Spherical)
	{
		m_targetBeam.setWaist(m_targetBeam.waist(), Spherical);
		m_targetBeam.setWaistPosition(m_targetBeam.waistPosition(), Spherical);
	}

	bool wasSpherical = isSpherical();
	m_targetOrientation = orientation;
	if (wasSpherical ^ isSpherical())
		emit(onOpticsBenchSphericityChanged());

	emit(onOpticsBenchTargetBeamChanged());

	setModified(true);
}

bool OpticsBench::magicWaist()
{
	OpticsFunction function(m_optics, m_wavelength);
	function.setOverlapBeam(m_targetBeam);
	function.setCheckLock(true);

	vector<int> opticsMovable;
	for (int i = 0; i < nOptics(); i++)
		if (!optics(i)->absoluteLock() && !optics(i)->relativeLockParent())
			opticsMovable.push_back(i);

	if (opticsMovable.empty())
		return false;

	const int nTry = 500000;
	bool found = false;

	/// @bug this has to change !
	const double minPos = m_boundary.x1();
	const double maxPos = m_boundary.x2();

	vector<double> positions = function.currentPosition();

	for (int i = 0; i < nTry; i++)
	{
		/// @todo find a suitable RNG
		// Randomly moves a random optics
		int index = rand() % nOptics();
		double position = double(rand())/double(RAND_MAX)*(maxPos - minPos) + minPos;
		positions[index] = position;
//		positions = function.localMaximum(positions);

		/// @bug 2D magic waist
		// Check waist
		Beam beam = function.beam(positions);
		if (Beam::overlap(beam, m_targetBeam) > m_targetOverlap)
		{
			cerr << "found waist : " << beam << " // try = " << i << endl;
			found = true;
			break;
		}
	}

	positions = function.localMaximum(positions);

	if (found)
	{
		for (unsigned int i = 0; i < positions.size(); i++)
			m_optics[i]->setPosition(positions[i], true);
		sort(m_optics.begin() + 1, m_optics.end(), less<Optics*>());
		computeBeams();
    }
	else
		cerr << "Beam not found !!!" << endl;

	return found;
}

bool OpticsBench::localOptimum()
{
	OpticsFunction function(m_optics, m_wavelength);
	function.setOverlapBeam(m_targetBeam);
	function.setCheckLock(true);
	vector<double> positions = function.localMaximum(function.currentPosition());

	if (!function.optimizationSuccess())
		return false;

	for (unsigned int i = 0; i < positions.size(); i++)
		m_optics[i]->setPosition(positions[i], true);
	sort(m_optics.begin() + 1, m_optics.end(), less<Optics*>());
	computeBeams();

	return true;
}
