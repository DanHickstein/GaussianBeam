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

#include "GaussianBeam.h"
#include "Utils.h"

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace Utils;

Orientation quadrature(Orientation orientation)
{
	if (orientation == Horizontal)
		return Vertical;

	if (orientation == Vertical)
		return Horizontal;

	cerr << "Invalid orientation in quadrature(" << orientation << ")" << endl;
	return orientation;
}

Beam::Beam()
{
	init();
}

Beam::Beam(double wavelength)
{
	init();
	setWavelength(wavelength);
}

Beam::Beam(double waist, double waistPosition, double wavelength, double index, double M2)
{
	init();
	setWavelength(wavelength);
	setIndex(index);
	setM2(M2);
	setWaist(waist);
	setWaistPosition(waistPosition);
}

Beam::Beam(const complex<double>& q, double z, double wavelength, double index, double M2)
{
	init();
	setWavelength(wavelength);
	setIndex(index);
	setM2(M2);
	setQ(q, z);
}

void Beam::init()
{
	m_wavelength = 461e-9;
	m_index = 1.;
	m_M2 = 1.;
	setWaist(1e-4);
	setWaistPosition(0.);
	m_angle = 0.;
	m_start = 0.;
	m_stop = 0.;
	m_sphericalWaist = true;
	m_sphericalPosition = true;
}

/////////////
// Properties

double Beam::waistPosition(Orientation orientation) const
{
	if (orientation == Vertical)
		return m_waistPosition.second;

	return m_waistPosition.first;
}

void Beam::setWaistPosition(double waistPosition, Orientation orientation)
{
	if (orientation != Vertical)
		m_waistPosition.first = waistPosition;

	if (orientation != Horizontal)
		m_waistPosition.second = waistPosition;

	m_sphericalPosition = (Beam::waistPosition(Horizontal) == Beam::waistPosition(Vertical));
}

double Beam::waist(Orientation orientation) const
{
	if (orientation == Vertical)
		return m_waist.second;

	return m_waist.first;
}

void Beam::setWaist(double waist, Orientation orientation)
{
	if (orientation != Vertical)
		m_waist.first = waist;

	if (orientation != Horizontal)
		m_waist.second = waist;

	m_sphericalWaist = (Beam::waist(Horizontal) == Beam::waist(Vertical));
}

double Beam::divergence(Orientation orientation) const
{
	if (waist(orientation) == 0.)
		return 0.;

	return atan(m_wavelength*m_M2/(m_index*M_PI*waist(orientation)));
}

void Beam::setDivergence(double divergence, Orientation orientation)
{
	if ((divergence > 0.) && (divergence < M_PI/2.))
		setWaist(m_wavelength*m_M2/(m_index*M_PI*tan(divergence)), orientation);
}

double Beam::rayleigh(Orientation orientation) const
{
	if (m_wavelength == 0.)
		return 0.;

	return m_index*M_PI*sqr(waist(orientation))/(m_wavelength*m_M2);
}

void Beam::setRayleigh(double rayleigh, Orientation orientation)
{
	if (rayleigh > 0.)
		setWaist(sqrt(rayleigh*m_wavelength*m_M2/(m_index*M_PI)), orientation);
}

double Beam::wavelength() const
{
	return m_wavelength;
}

void Beam::setWavelength(double wavelength)
{
	m_wavelength = wavelength;
}

double Beam::index() const
{
	return m_index;
}

void Beam::setIndex(double index)
{
	if (index > 0.)
		m_index = index;
}

double Beam::M2() const
{
	return m_M2;
}

void Beam::setM2(double M2)
{
	if (M2 >= 1.)
		m_M2 = M2;
}

////////////////////////////////
// Aspect

bool Beam::isSpherical() const
{
	return (m_sphericalWaist && m_sphericalPosition);
}

Orientation Beam::orientation() const
{
	return isSpherical() ? Spherical : Ellipsoidal;
}

void Beam::makeSpherical(Orientation orientation)
{
	setWaist(waist(orientation), Spherical);
	setWaistPosition(waistPosition(orientation), Spherical);
}

////////////////////////////////
// Position dependent properties

double Beam::radius(double z, Orientation orientation) const
{
	return waist(orientation)*sqrt(1. + sqr(zred(z, orientation)));
}

double Beam::radiusDerivative(double z, Orientation orientation) const
{
	return waist(orientation)/rayleigh(orientation)/sqrt(1. + 1./sqr(zred(z, orientation)));
}

double Beam::radiusSecondDerivative(double z, Orientation orientation) const
{
	return waist(orientation)/sqr(rayleigh(orientation))/pow(1. + sqr(zred(z, orientation)), 1.5);
}

double Beam::curvature(double z, Orientation orientation) const
{
	return (z - waistPosition(orientation))*(1. + 1./sqr(zred(z, orientation)));
}

double Beam::gouyPhase(double z, Orientation orientation) const
{
	return atan(zred(z, orientation));
}

void Beam::setQ(complex<double> q, double z, Orientation orientation)
{
	setRayleigh(q.imag(), orientation);
	setWaistPosition(z - q.real(), orientation);
}

complex<double> Beam::q(double z, Orientation orientation) const
{
	return complex<double>(z - waistPosition(orientation), rayleigh(orientation));
}

/////////////////////////
// Geometrical properties

double Beam::start() const
{
	return m_start;
}

void Beam::setStart(double start)
{
	m_start = start;
}

double Beam::stop() const
{
	return m_stop;
}

void Beam::setStop(double stop)
{
	m_stop = stop;
}

Point Beam::origin() const
{
	return m_origin;
}

double Beam::angle() const
{
	return m_angle;
}

void Beam::rotate(double pivot, double angle)
{
	double l = 2.*pivot*sin(angle/2.);

	m_origin += Point(l*sin(m_angle + angle/2.), -l*cos(m_angle + angle/2.));
	m_angle = fmodPos(m_angle + angle, 2.*M_PI);
}

Point Beam::beamCoordinates(const Point& point) const
{
	return Point(
		 cos(m_angle)*(point.x() - m_origin.x()) + sin(m_angle)*(point.y() - m_origin.y()),
		-sin(m_angle)*(point.x() - m_origin.x()) + cos(m_angle)*(point.y() - m_origin.y()));
}

Point Beam::absoluteCoordinates(double position, double distance) const
{
	return m_origin + Point(
		position*cos(m_angle) - distance*sin(m_angle),
		position*sin(m_angle) + distance*cos(m_angle));
}

vector<double> Beam::rectangleIntersection(const Utils::Rect& rect) const
{
	vector<double> intersections;

	if (cos(m_angle) != 0.)
		intersections << (rect.x1() - m_origin.x())/cos(m_angle) << (rect.x2() - m_origin.x())/cos(m_angle);
	else
		intersections << -Utils::infinity << +Utils::infinity;

	if (sin(m_angle) != 0.)
		intersections << (rect.y1() - m_origin.y())/sin(m_angle) << (rect.y2() - m_origin.y())/sin(m_angle);
	else
		intersections << -Utils::infinity << +Utils::infinity;

	sort(intersections.begin(), intersections.end());
	intersections.pop_back();
	intersections.erase(intersections.begin());
	return intersections;
}

pair<double, double> Beam::angledBoundaries(double position, double slope, Orientation orientation) const
{
	pair<double, double> result = pair<double, double>(-Utils::infinity, Utils::infinity);

	const double z0 = rayleigh(orientation);
	const double w0 = waist(orientation);
	double b = sqr(w0/(slope*z0));
	double pr = zred(position, orientation);
	double delta = b*(1. + sqr(pr) - b);

	if (delta < 0.)
		return result;

	/// @todo case b = 1

	double r = z0*sqrt(delta)/(1.-b);
	double l = waistPosition() + z0*pr/(1.-b);

	result.first = l + sign(slope)*r;
	result.second = l - sign(slope)*r;

	return result;
}

//////////////////////
// static computations

double Beam::overlap(const Beam& beam1, const Beam& beam2, double z, Orientation orientation)
{
	if ((orientation != Ellipsoidal) || (beam1.isSpherical() && beam2.isSpherical()))
	{
	//	double w1 = beam1.radius(z);
	//	double w2 = beam2.radius(z);
	//	double w12 = sqr(beam1.radius(z));
	//	double w22 = sqr(beam2.radius(z));
	//	double k1 = 2.*M_PI/beam1.wavelength();
	//	double k2 = 2.*M_PI/beam2.wavelength();
	//	double R1 = beam1.curvature(z);
	//	double R2 = beam2.curvature(z);
		double zred1 = beam1.zred(z, orientation);
		double zred2 = beam2.zred(z, orientation);
		double rho = sqr(beam1.radius(z, orientation)/beam2.radius(z, orientation));

		//double eta = 4./sqr(w1*w2)/(sqr(1./sqr(w1) + 1./sqr(w2)) + sqr((k1/R1 - k2/R2)/2.));
		//double eta = 4./(w12*w22)/(sqr(1./w12 + 1./w22) + sqr(zred1/w12 - zred2/w22));
		double eta = 4.*rho/(sqr(1. + rho) + sqr(zred1 - zred2*rho));

		return eta;
	}
	else
		return sqrt(overlap(beam1, beam2, z, Horizontal)*overlap(beam1, beam2, z, Vertical));

	return 0.;
}

bool Beam::copropagating(const Beam& beam1, const Beam& beam2)
{
	double deltaX = beam1.origin().x() - beam2.origin().x();
	double deltaY = beam1.origin().y() - beam2.origin().y();
	double deltaAngle = fmodPos(beam1.angle() - beam2.angle(), 2.*M_PI);

//	cerr << " angles = " << beam1.angle() << " " << beam2.angle() << " " << deltaAngle << endl;
//	cerr << " criterion = " << deltaOrigin[1] - tan(beam1.angle())*deltaOrigin[0] << endl;

	if (   (   (deltaAngle < Utils::epsilon)
		    || (deltaAngle > (2.*M_PI - Utils::epsilon)))
		&& (fabs(deltaY - tan(beam1.angle())*deltaX) < epsilon))
		return true;

	return false;
}

ostream& operator<<(ostream& out, const Beam& beam)
{
	out << "Waist = (" << beam.waist(Horizontal) << "," << beam.waist(Vertical)
	    << ")  Waist position = (" << beam.waistPosition(Horizontal) << "," << beam.waistPosition(Vertical) << ")";
	return out;
}

bool Beam::operator==(const Beam& other) const
{
	return ((m_waist         == other.m_waist        ) &&
	        (m_waistPosition == other.m_waistPosition) &&
	        (m_wavelength    == other.m_wavelength   ) &&
	        (m_index         == other.m_index        ) &&
	        (m_M2            == other.m_M2           ) &&
	        (m_origin        == other.m_origin       ) &&
	        (m_angle         == other.m_angle        ) &&
	        (m_start         == other.m_start        ) &&
	        (m_stop          == other.m_stop         ));
}

//////////
// Private

inline double Beam::zred(double z, Orientation orientation) const
{
	return (z - waistPosition(orientation))/rayleigh(orientation);
}

