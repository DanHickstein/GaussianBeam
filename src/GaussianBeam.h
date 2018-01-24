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

#ifndef GAUSSIANBEAM_H
#define GAUSSIANBEAM_H

#include <complex>
#include <vector>

#include "Utils.h"

namespace Property
{
enum  Type {BeamPosition = 0, BeamRadius, BeamDiameter, BeamCurvature, BeamGouyPhase, BeamDistanceToWaist,
            BeamParameter, BeamWaist, BeamWaistPosition, BeamRayleigh, BeamDivergence, Index,
            OpticsType, OpticsPosition, OpticsRelativePosition, OpticsProperties, OpticsName,
            OpticsLock, OpticsSensitivity, OpticsAngle, OpticsOrientation};
}

enum  Orientation {Spherical = 0, Horizontal = 1,  Vertical = 2, Ellipsoidal = 3};

Orientation quadrature(Orientation orientation);

/**
* @brief This class represents a Gaussian beam.
* A beam is defined by an origin (2 coordinates in the plane) and the angle between the wave vector
* and the canonical basis of the plane. All varying beam properties are defined according to a single coordinate
* that corresponds to the algebraic distance between the origin and a point on the beam axis.
* The Gaussian properties of the beam are defined by its wavelength, waist and waist position on the vertical
* and horizontal axis. In all get function, the default orientation is Horizontal and in all set function the default
* orientation is Spherical ("default" means "no argment given" or "unvalid argument given, i.e. ellispoidal")
*/
class Beam
{
public:
	/// Default constructor
	Beam();
	/// Constructor
	Beam(double wavelength);
	/// Constructor
	Beam(double waist, double waistPosition, double wavelength, double index = 1., double M2 = 1.);
	/// Construct a Gaussian beam from a given complex beam parapeter @p q at position @p z
	Beam(const std::complex<double>& q, double z, double wavelength, double index = 1., double M2 = 1.);
	/// Comparison operator
	bool operator==(const Beam& other) const;

public:

	// Intrinsic properties
	/// @return the beam wavelength
	double wavelength() const;
	/// Set the beam wavelength to @p wavelength
	void setWavelength(double wavelength);
	/// @return the waist position
	double waistPosition(Orientation orientation = Horizontal) const;
	/// Set the waist position to @p waistPosition
	void setWaistPosition(double waistPosition, Orientation orientation = Spherical);
	/// @return the waist radius at 1/e²
	double waist(Orientation orientation = Horizontal) const;
	/// Set the waist radius at 1/e² to @p waist
	void setWaist(double waist, Orientation orientation = Spherical);
	/// @return the beam divergence (half angle)
	double divergence(Orientation orientation = Horizontal) const;
	/// Set the beam divergence to @p divergence
	void setDivergence(double divergence, Orientation orientation = Spherical);
	/// @return the Rayleigh range
	double rayleigh(Orientation orientation = Horizontal) const;
	/// Set the rayleigh range to @p rayleigh
	void setRayleigh(double rayleigh, Orientation orientation = Spherical);
	/// @return the index of the medium in which the beam propagates
	double index() const;
	/// Set the index of the medium in which the beam propagates to @p index
	void setIndex(double index);
	/// @return the beam quality factor M²
	double M2() const;
	/// Set the beam quality factor M² to @p M2
	void setM2(double M2);

	// Aspect functions
	/// @return true if the beam is spherical
	bool isSpherical() const;
	/// @return the orientation of the beam: Spherical or Ellipsoidal
	Orientation orientation() const;
	/// Make the beam spherical according to properties on orientation @p orientation
	void makeSpherical(Orientation orientation = Horizontal);

	// Position dependent properties
	/// @return the beam radius at 1/e² at position @p z
	double radius(double z, Orientation orientation = Horizontal) const;
	/// @return the derivative of the beam radius at 1/e² at position @p z
	double radiusDerivative(double z, Orientation orientation = Horizontal) const;
	/// @return the second derivative of the beam radius at 1/e² at position @p z
	double radiusSecondDerivative(double z, Orientation orientation = Horizontal) const;
	/// @return the curvature radius of the beam at position @p z
	double curvature(double z, Orientation orientation = Horizontal) const;
	/// @return the Gouy phase at position @p z
	double gouyPhase(double z, Orientation orientation = Horizontal) const;
	/// @return the beam complex parameter \f$ q = (z-z_w) + iz_0 \f$ at position @p z
	std::complex<double> q(double z, Orientation orientation = Horizontal) const;
	/// Set the complex beam parameter
	void setQ(std::complex<double> q, double z, Orientation orientation = Spherical);

	// Geometrical properties
	/// @return the start of the beam along the propagation axis, from the origin
	double start() const;
	/// Set the start of the beam along the propagation axis
	void setStart(double start);
	/// @return the stop of the beam along the propagation axis, from the origin
	double stop() const;
	/// Set the end of the beam along the propagation axis
	void setStop(double stop);
	/// Rotate the beam by an angle @p angle around position @p pivot
	void rotate(double pivot, double angle);
	/// @return the position for the origin of this beam in the plane
	Utils::Point origin() const;
	/// @return the angle between the wave vector and the plane abscissa
	double angle() const;
	/**
	* @return the beam coordinates of absolute point @p point
	* The first beam coordinate is the position of the orthogonal projection of @p point on the beam axis
	* The second beam coordinate is the algebraic distance of @p point to the beam (negative if on the right, positive if on the left
	*/
	Utils::Point beamCoordinates(const Utils::Point& point) const;
	/// @return the absolute coordinates of beam coordiantes ( @p position , @p distance )
	Utils::Point absoluteCoordinates(double position, double distance = 0) const;
	/// @return the extreme positions of the intersection of the beam and rectangle @p rect
	std::vector<double> rectangleIntersection(const Utils::Rect& rect) const;
	/// @return the abscissa (in beam coordinates) of the intersections of the 1/e² radius of the beam and
	/// a line with origin @p position and slope @p slope. The upper intersection comes first.
	std::pair<double, double> angledBoundaries(double position, double slope, Orientation orientation = Horizontal) const;

	// Physical properties
	/**
	* Compute the intensity overlap between beams @p beam1 and @p beam2 at position @p z
	* This overlap does not depend on @p z if both beams have the same wavelength,
	* hence the default value for z
	* @todo this is only implented for beams of identical wavelength and copropagating
	*/
	static double overlap(const Beam& beam1, const Beam& beam2, double z = 0., Orientation = Ellipsoidal);
	/**
	* Tell whether the two given beams share the same optical axis, in the same direction
	*/
	static bool copropagating(const Beam& beam1, const Beam& beam2);

private:
	inline double zred(double z, Orientation orientation) const;
	void init();

private:
	// Don't forget to update the == operator when adding properties
	// Optical properties
	std::pair<double, double> m_waist;
	std::pair<double, double> m_waistPosition;
	double m_wavelength;
	double m_index;
	double m_M2;
	// Geometrical properties
	Utils::Point m_origin;
	double m_angle;
	double m_start;
	double m_stop;

	// Aspect cache
	mutable bool m_sphericalWaist, m_sphericalPosition;
};

std::ostream& operator<<(std::ostream& out, const Beam& beam);

#endif
