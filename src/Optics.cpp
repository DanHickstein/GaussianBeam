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

#include "Optics.h"
#include "Utils.h"

#include <iostream>
#include <typeinfo>
#include <cmath>

using namespace std;

Optics::Optics(OpticsType type, double position, string name)
	: m_position(position)
	, m_width(0.)
	, m_orientation(Spherical)
	, m_angle(0.)
	, m_name(name)
	, m_absoluteLock(false)
	, m_relativeLockParent(0)
	, m_type(type)
	, m_rotable(false)
{
}

Optics::Optics(const Optics& optics)
	: m_position    (optics.m_position    )
	, m_width       (optics.m_width       )
	, m_orientation (optics.m_orientation )
	, m_angle       (optics.m_angle       )
	, m_name        (optics.m_name        )
	, m_absoluteLock(optics.m_absoluteLock)
	, m_relativeLockParent(0)
	, m_type        (optics.m_type        )
	, m_rotable     (optics.m_rotable     )
{}

Optics::~Optics()
{
	// Detach all children
	for (list<Optics*>::iterator it = m_relativeLockChildren.begin(); it != m_relativeLockChildren.end(); it++)
		(*it)->m_relativeLockParent = 0;

	relativeUnlock();
}

bool Optics::isOrientable() const
{
	return isOrientable(Horizontal) || isOrientable(Vertical) || isOrientable(Ellipsoidal);
}

void Optics::setOrientation(Orientation orientation)
{
	if (isOrientable(orientation))
		m_orientation = orientation;
	else
		cerr << "Error in Optics::setOrientation : the optics is not orientable along this orientation" << endl;
}

/////////////////////////////////////////////////
// Locking functions

void Optics::setAbsoluteLock(bool absoluteLock)
{
	if (absoluteLock)
		relativeUnlock();

	m_absoluteLock = absoluteLock;
}

bool Optics::relativeLockedTo(const Optics* const optics) const
{
	return relativeLockRoot()->isRelativeLockDescendant(optics);
}

bool Optics::relativeLockTo(Optics* optics)
{
	if (relativeLockedTo(optics))
		return false;

	if (m_relativeLockParent)
		relativeUnlock();

	m_relativeLockParent = optics;
	optics->m_relativeLockChildren.push_back(this);

	m_absoluteLock = false;
	return true;
}

bool Optics::relativeUnlock()
{
	if (!m_relativeLockParent)
		return false;

	m_relativeLockParent->m_relativeLockChildren.remove(this);
	m_relativeLockParent = 0;

	return true;
}

Optics* Optics::relativeLockRoot()
{
	if (!m_relativeLockParent)
		return this;
	else
		return m_relativeLockParent->relativeLockRoot();
}

const Optics* Optics::relativeLockRoot() const
{
	if (!m_relativeLockParent)
		return this;
	else
		return m_relativeLockParent->relativeLockRoot();
}

bool Optics::isRelativeLockDescendant(const Optics* const optics) const
{
	if (optics == this)
		return true;

	for (list<Optics*>::const_iterator it = m_relativeLockChildren.begin(); it != m_relativeLockChildren.end(); it++)
		if ((*it)->isRelativeLockDescendant(optics))
			return true;

	return false;
}

void Optics::moveDescendant(double distance)
{
	setPosition(position() + distance, false);

	for (list<Optics*>::iterator it = m_relativeLockChildren.begin(); it != m_relativeLockChildren.end(); it++)
		(*it)->moveDescendant(distance);
}

void Optics::setPosition(double position, bool respectLocks)
{
	setPosition(position, respectLocks, respectLocks);
}

void Optics::setPosition(double position, bool respectAbsoluteLock, bool respectRelativeLock)
{
	if (relativeLockTreeAbsoluteLock() && respectAbsoluteLock)
		return;

	if (respectRelativeLock)
		relativeLockRoot()->moveDescendant(position - Optics::position());
	else
		m_position = position;
}

bool Optics::operator==(const Optics& other) const
{
	bool sameRelativeParent = true;
	if (m_relativeLockParent && other.m_relativeLockParent)
		sameRelativeParent = (*m_relativeLockParent == *other.m_relativeLockParent);

	return (m_position     == other.m_position    ) &&
	       (m_width        == other.m_width       ) &&
	       (m_orientation  == other.m_orientation ) &&
	       (m_angle        == other.m_angle       ) &&
	       (m_name         == other.m_name        ) &&
	       (m_absoluteLock == other.m_absoluteLock) &&
	       (m_type         == other.m_type        ) &&
	       sameRelativeParent;
}

/////////////////////////////////////////////////
// CreateBeam class

CreateBeam::CreateBeam(double waist, double waistPosition, double index, string name)
	: Optics(CreateBeamType, 0., name)
{
	m_beam.setWaist(waist);
	m_beam.setWaistPosition(waistPosition);
	m_beam.setIndex(index);

	setRotable();
}

const Beam* CreateBeam::beam() const
{
	return &m_beam;
}


void CreateBeam::setBeam(const Beam& beam)
{
	m_beam = beam;
	if (!m_beam.isSpherical())
		setOrientation(Ellipsoidal);
}

Beam CreateBeam::image(const Beam& inputBeam, const Beam& /*opticalAxis*/) const
{
	Beam outputBeam = m_beam;
	outputBeam.setWavelength(inputBeam.wavelength());
	outputBeam.rotate(0, angle());
	if (orientation() == Spherical)
		outputBeam.makeSpherical();

	return outputBeam;
}

Beam CreateBeam::antecedent(const Beam& outputBeam, const Beam& opticalAxis) const
{
	return image(outputBeam, opticalAxis);
}

bool CreateBeam::operator==(const Optics& other) const
{
	if (!this->Optics::operator==(other))
		return false;

	// This cast will always work because Optics::operator== checked types
	const CreateBeam& otherCreateBeam = static_cast<const CreateBeam&>(other);
	return (m_beam == otherCreateBeam.m_beam);
}

/////////////////////////////////////////////////
// Lens class

void Lens::setFocal(double focal)
{
	if (focal != 0.)
		m_focal = focal;
}

/////////////////////////////////////////////////
// FlatMirror class

Beam FlatMirror::image(const Beam& inputBeam, const Beam& opticalAxis) const
{
	double relativeAngle = angle() + opticalAxis.angle() - inputBeam.angle();

	if ((relativeAngle > M_PI/2.) && (relativeAngle < 3.*M_PI/2.))
		return inputBeam;

	Beam result = ABCD::image(inputBeam, opticalAxis);

	double rotationAngle = fmod(2.*relativeAngle + M_PI, 2.*M_PI);
	result.rotate(position(), rotationAngle);
	return result;
}

Beam FlatMirror::antecedent(const Beam& outputBeam, const Beam& opticalAxis) const
{
	Beam result = ABCD::antecedent(outputBeam, opticalAxis);

	/// @bug rotate this beam

	return result;
}

/////////////////////////////////////////////////
// CurvedMirror class

void CurvedMirror::setCurvatureRadius(double curvatureRadius)
{
	if (curvatureRadius != 0.)
		m_curvatureRadius = curvatureRadius;
}

/////////////////////////////////////////////////
// Dielectric class

void Dielectric::setIndexRatio(double indexRatio)
{
	if (indexRatio > 0.)
		m_indexRatio = indexRatio;
}

/////////////////////////////////////////////////
// CurvedInterface class

void CurvedInterface::setSurfaceRadius(double surfaceRadius)
{
	if (surfaceRadius != 0.)
		m_surfaceRadius = surfaceRadius;
}

/////////////////////////////////////////////////
// ABCD class

void ABCD::forward(const Beam& inputBeam, Beam& outputBeam, Orientation orientation) const
{
	complex<double> q = inputBeam.q(position(), orientation);
	q = (A(orientation)*q + B(orientation)) / (C(orientation)*q + D(orientation));
	outputBeam.setQ(q, position() + width(), orientation);
}

Beam ABCD::image(const Beam& inputBeam, const Beam& /*opticalAxis*/) const
{
	Beam outputBeam = inputBeam;
	outputBeam.setIndex(inputBeam.index()*indexJump());

	if ((orientation() == Spherical) && (inputBeam.isSpherical()))
		forward(inputBeam, outputBeam, Spherical);
	else
	{
		forward(inputBeam, outputBeam, Horizontal);
		forward(inputBeam, outputBeam, Vertical);
	}

	return outputBeam;
}

void ABCD::backward(const Beam& outputBeam, Beam& inputBeam, Orientation orientation) const
{
	complex<double> q = outputBeam.q(position() + width(), orientation);
	q = (B(orientation) - D(orientation)*q) / (C(orientation)*q - A(orientation));
	inputBeam.setQ(q, position(), orientation);
}

Beam ABCD::antecedent(const Beam& outputBeam, const Beam& /*opticalAxis*/) const
{
	Beam inputBeam = outputBeam;
	inputBeam.setIndex(outputBeam.index()/indexJump());

	if ((orientation() == Spherical) && (outputBeam.isSpherical()))
		backward(outputBeam, inputBeam, Spherical);
	else
	{
		backward(outputBeam, inputBeam, Horizontal);
		backward(outputBeam, inputBeam, Vertical);
	}

	return inputBeam;
}

bool ABCD::operator==(const Optics& other) const
{
	if (!this->Optics::operator==(other))
		return false;

	// This cast will always work because Optics::operator== checked types
	const ABCD& otherABCD = static_cast<const ABCD&>(other);

	return (A(Horizontal) == otherABCD.A(Horizontal)) &&
	       (B(Horizontal) == otherABCD.B(Horizontal)) &&
	       (C(Horizontal) == otherABCD.C(Horizontal)) &&
	       (D(Horizontal) == otherABCD.D(Horizontal)) &&
	       (A(Vertical)   == otherABCD.A(Vertical)  ) &&
	       (B(Vertical)   == otherABCD.B(Vertical)  ) &&
	       (C(Vertical)   == otherABCD.C(Vertical)  ) &&
	       (D(Vertical)   == otherABCD.D(Vertical)  );
}

/////////////////////////////////////////////////
// GenericABCD class

GenericABCD::GenericABCD(const ABCD& abcd) : ABCD(abcd)
{
	setType(GenericABCDType);

	if (abcd.orientation() == Spherical)
		setABCD(abcd.A(Spherical), abcd.B(Spherical), abcd.C(Spherical), abcd.D(Spherical), Spherical);
	else
	{
		setABCD(abcd.A(Horizontal), abcd.B(Horizontal), abcd.C(Horizontal), abcd.D(Horizontal), Horizontal);
		setABCD(abcd.A(Vertical),   abcd.B(Vertical),   abcd.C(Vertical),   abcd.D(Vertical),   Vertical);
	}
}

GenericABCD::GenericABCD(double A, double B, double C, double D, double width, double position, std::string name)
	: ABCD(GenericABCDType, position, name)
{
	setWidth(width);
	setABCD(A, B, C, D);
}

void GenericABCD::setABCD(double A, double B, double C, double D, Orientation orientation)
{
	setA(A, orientation);
	setB(B, orientation);
	setC(C, orientation);
	setD(D, orientation);
}

double GenericABCD::getCoefficient(const double coefficient[], Orientation orientation) const
{
	if ((orientation == Spherical) && (GenericABCD::orientation() == Ellipsoidal))
		cerr << "Wrong orientation for GenericABCD::getCoefficient" << endl;

	if ((orientation == Spherical) || (orientation == Horizontal) || (GenericABCD::orientation() == Spherical))
		return coefficient[0];
	else
		return coefficient[1];
}

void GenericABCD::setCoefficient(double coefficient[], double value, Orientation orientation)
{
	if (orientation == Spherical)
		coefficient[0] = coefficient[1] = value;
	else if (orientation == Horizontal)
		coefficient[0] = value;
	else if (orientation == Vertical)
		coefficient[1] = value;
	else
		cerr << "Wrong orientation for GenericABCD::setCoefficient" << endl;

	if (orientation != Spherical)
		setOrientation(Ellipsoidal);
}

void GenericABCD::mult(const ABCD& abcd, Orientation orientation)
{
	setA(A(orientation)*abcd.A(orientation) + B(orientation)*abcd.C(orientation), orientation);
	setB(A(orientation)*abcd.B(orientation) + B(orientation)*abcd.D(orientation), orientation);
	setC(C(orientation)*abcd.A(orientation) + D(orientation)*abcd.C(orientation), orientation);
	setD(C(orientation)*abcd.B(orientation) + D(orientation)*abcd.D(orientation), orientation);
}

GenericABCD& GenericABCD::operator*=(const ABCD& abcd)
{
	if ((orientation() == Spherical) && (abcd.orientation() == Spherical))
		mult(abcd, Spherical);
	else
	{
		mult(abcd, Horizontal);
		mult(abcd, Vertical);
	}

	/// @todo check if the two objects are adjacent ?
	setWidth(width() + abcd.width());

	return *this;
}

GenericABCD operator*(const ABCD& abcd1, const ABCD& abcd2)
{
	GenericABCD r(abcd1);
	r *= abcd2;
	return r;
}

ostream& operator<<(ostream& out, const ABCD& abcd)
{
	if (abcd.orientation() == Spherical)
		out << "A = " << abcd.A(Spherical) << " B = " << abcd.B(Spherical) << " C = " << abcd.C(Spherical) << " D = " << abcd.D(Spherical);
	else
	{
		out << "Ah = " << abcd.A(Horizontal) << " Bh = " << abcd.B(Horizontal) << " Ch = " << abcd.C(Horizontal) << " Dh = " << abcd.D(Horizontal);
		out << "Av = " << abcd.A(Vertical  ) << " Bv = " << abcd.B(Vertical  ) << " Cv = " << abcd.C(Vertical  ) << " Dv = " << abcd.D(Vertical  );
	}

	return out;
}
