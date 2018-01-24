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

#ifndef UTILS_H
#define UTILS_H

#include "Delegate.h"
#include <vector>
#include <set>
#include <cmath>

inline double sqr(double x)
{
	return x*x;
}

inline double sign(double x)
{
	return x < 0. ? -1. : 1.;
}

inline int intSign(double x)
{
	return x < 0. ? -1 : 1;
}

inline double fmodPos(double x, double m)
{
	double result = fmod(x, m);
	if (x < 0.)
		return result + m;

	return result;
}

std::vector<double> operator+(const std::vector<double>& v, double r);
std::vector<double> operator-(const std::vector<double>& v, double r);
std::vector<double> operator*(const std::vector<double>& v, double r);
std::vector<double> operator/(const std::vector<double>& v, double r);
inline std::vector<double> operator+(double r, const std::vector<double>& v) { return v + r; }
inline std::vector<double> operator*(double r, const std::vector<double>& v) { return v * r; }
std::vector<double>& operator+=(std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double>& operator-=(std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double> operator+(const std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double> operator-(const std::vector<double>& v1, const std::vector<double>& v2);
std::vector<double>& operator<<(std::vector<double>& v, double r);

namespace Utils
{
	static const double infinity = 1e100;
	static const double epsilon  = 1e-10;

	double scalar(const std::vector<double>& v1, const std::vector<double>& v2);
	double norm(const std::vector<double>& v1);
//	double distance(const std::vector<double>& v1, const std::vector<double>& v2);

	/// Point
	class Point
	{
	public:
		/// Constructors
		Point() { m_x = m_y = 0.; }
		Point(double x, double y) { m_x = x; m_y = y; }

	public:
		double x() const { return m_x; }
		double y() const { return m_y; }
		void translate(const Point& point) { m_x += point.x(); m_y += point.y(); }
		bool operator==(const Point& other) const;

	private:
		double m_x, m_y;
	};

	Point& operator+=(Point& p1, const Point& p2);
	Point operator+(const Point& p1, const Point& p2);
	double distance(const Point& p1, const Point& p2);

	/// Rect
	class Rect
	{
	public:
		/// Constructors
		Rect() { m_x1 = m_y1 = m_x2 = m_y2 = 0.; }
		Rect(double x1, double y1, double x2, double y2) { m_x1 = x1; m_y1 = y1; m_x2 = x2; m_y2 = y2; }

	public:
		double x1() const { return m_x1; }
		double y1() const { return m_y1; }
		double x2() const { return m_x2; }
		double y2() const { return m_y2; }
		void setX1(double x1) { m_x1 = x1; }
		void setY1(double y1) { m_y1 = y1; }
		void setX2(double x2) { m_x2 = x2; }
		void setY2(double y2) { m_y2 = y2; }
		double width() const { return fabs(m_x2 - m_x1); }
		double height() const { return fabs(m_y2 - m_y1); }

	private:
		double m_x1, m_y1, m_x2, m_y2;
	};

	/// Signal
	template<class Param>
	class Signal
	{
	private:
		typedef std::set<Delegate1<Param> > DelegateList;
		typedef typename DelegateList::iterator DelegateIterator;
		DelegateList m_delegateList;

	public:
		template<class X, class Y> void connect(Y* obj, void (X::*func)(Param p))          { m_delegateList.insert(MakeDelegate(obj, func)); }
//		template<class X, class Y> void connect(Y* obj, void (X::*func)(Param p) const)    { m_delegateList.insert(MakeDelegate(obj, func)); }
		template<class X, class Y> void disconnect(Y* obj, void (X::*func)(Param p))       { m_delegateList.erase(MakeDelegate(obj, func)); }
//		template<class X, class Y> void disconnect(Y* obj, void (X::*func)(Param p) const) { m_delegateList.erase(MakeDelegate(obj, func)); }
		void emit(Param p) const
		{
			for (DelegateIterator it = m_delegateList.begin(); it != m_delegateList.end(); ++it)
				(*it)(p);
		}

	};

}

#endif
