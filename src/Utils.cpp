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

#include "Utils.h"

#include <cmath>

using namespace std;

/////////////////////////////////////////////////
// Extend vector operations

vector<double> operator+(const vector<double>& v, double r)
{
	vector<double> result = v;
	for (vector<double>::iterator it = result.begin(); it != result.end(); it++)
		(*it) += r;
	return result;
}

vector<double> operator-(const vector<double>& v, double r)
{
	vector<double> result = v;
	for (vector<double>::iterator it = result.begin(); it != result.end(); it++)
		(*it) -= r;
	return result;
}

vector<double> operator*(const vector<double>& v, double r)
{
	vector<double> result = v;
	for (vector<double>::iterator it = result.begin(); it != result.end(); it++)
		(*it) *= r;
	return result;
}

vector<double> operator/(const vector<double>& v, double r)
{
	vector<double> result = v;
	for (vector<double>::iterator it = result.begin(); it != result.end(); it++)
		(*it) /= r;
	return result;
}

vector<double>& operator+=(vector<double>& v1, const vector<double>& v2)
{
	for (unsigned int i = 0; i < ::min(v1.size(), v2.size()); i++)
		v1[i] += v2[i];
	return v1;
}

vector<double>& operator-=(vector<double>& v1, const vector<double>& v2)
{
	for (unsigned int i = 0; i < ::min(v1.size(), v2.size()); i++)
		v1[i] -= v2[i];
	return v1;
}

vector<double> operator+(const vector<double>& v1, const vector<double>& v2)
{
	vector<double> result = v1;
	result += v2;
	return result;
}

vector<double> operator-(const vector<double>& v1, const vector<double>& v2)
{
	vector<double> result = v1;
	result -= v2;
	return result;
}

vector<double>& operator<<(vector<double>& v, double r)
{
	v.push_back(r);
	return v;
}

namespace Utils
{

double scalar(const vector<double>& v1, const vector<double>& v2)
{
	double result = 0;

	for (unsigned int i = 0; i < ::min(v1.size(), v2.size()); i++)
		result += v1[i]*v2[i];

	return result;
}

double norm(const std::vector<double>& v1)
{
	return sqrt(scalar(v1, v1));
}

double distance(const std::vector<double>& v1, const std::vector<double>& v2)
{
	return norm(v2 - v1);
}

Point& operator+=(Point& p1, const Point& p2)
{
	p1.translate(p2);
	return p1;
}

Point operator+(const Point& p1, const Point& p2)
{
	Point result = p1;
	result.translate(p2);
	return result;
}

bool Point::operator==(const Point& other) const
{
	return ((m_x == other.m_x) &&
	        (m_y == other.m_y));
}

double distance(const Point& p1, const Point& p2)
{
	return sqrt(sqr(p2.x() - p1.x()) + sqr(p2.y() - p1.y()));
}

} // namespace Utils
