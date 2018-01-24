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

#ifndef NAMES_H
#define NAMES_H

#include <src/GaussianBeam.h>
#include <src/Optics.h>
#include <gui/Unit.h>

#include <QString>
#include <QMap>
#include <QApplication>

namespace Property
{
	extern QMap<Property::Type, QString> fullName;
	extern QMap<Property::Type, QString> shortName;
	extern QMap<Property::Type, UnitType> unit;
}

namespace OrientationName
{
	extern QMap<Orientation, QString> fullName;
	extern QMap<Orientation, QString> codedName;
}

namespace OpticsName
{
	extern QMap<OpticsType, QString> fullName;
	extern QMap<OpticsType, QString> codedName;
}

/// Called by main() to fill name maps
void initNames(QApplication* app);
/// Break a string in two lines of minimal length
QString breakString(QString string);

#endif
