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

#ifndef UNIT_H
#define UNIT_H

#include <QString>

enum UnitType {UnitPosition, UnitFocal, UnitWaist, UnitRayleigh, UnitWavelength,
               UnitDivergence, UnitCurvature, UnitHRange, UnitVRange, UnitABCD,
               UnitWidth, UnitPhase, UnitAngle, UnitLess};

class Unit
{
public:
	Unit(int power, QString unitString);
	Unit(UnitType type);

public:
	QString string(bool space = true) const;
	double multiplier() const;
	double divider() const;

private:
	QChar prefix() const;

private:
	int m_power;
	QString m_unitString;
};

#endif
