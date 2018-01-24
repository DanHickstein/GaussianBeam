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

#include "Function.h"
#include "Utils.h"

#include <cmath>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////
// Function class

Function::Function()
{
}

vector<double> Function::gradient(const vector<double>& x) const
{
	double epsilon = 1e-6;
	vector<double> xlocal = x;
	vector<double> grad(x.size());

	for (unsigned int i = 0; i < x.size(); i++)
	{
		xlocal[i] += epsilon;
		grad[i] = (value(xlocal) - value(x))/epsilon;
		xlocal[i] -= epsilon;
	}

	return grad;
}

vector<double> Function::curvature(const vector<double>& x) const
{
	double epsilon = 1e-6;
	vector<double> xlocal = x;
	vector<double> curv(x.size());

	for (unsigned int i = 0; i < x.size(); i++)
	{
		curv[i] = -2.*value(x);
		xlocal[i] += epsilon;
		curv[i] += value(xlocal);
		xlocal[i] -= 2.*epsilon;
		curv[i] += value(xlocal);
		xlocal[i] += epsilon;
		curv[i] /= sqr(epsilon);
	}

	return curv;
}

void Function::setLine(const vector<double>& point, const vector<double>& direction) const
{
	m_linePoint = point;
	m_lineDirection = direction;
}

vector<double> Function::lineParametric(double x) const
{
	return m_linePoint + m_lineDirection*x;
}

double Function::lineValue(double x) const
{
	return (m_min ? 1.: -1.)*value(lineParametric(x));
}

vector<double> Function::lineExtremum(const vector<double>& x, const vector<double>& u, bool min) const
{
	m_success = true;
	setExtremumType(min);
	setLine(x, u);

	double start = 0.;
	double stop = 1.;
	cerr << "  Initial guess " << start << " to  " << stop << endl;
	vector<double> bracket = bracketMinimum(start, stop);
	cerr << "  Improved guess " << bracket[0] << " to " << bracket[2] << " by " << bracket[1] << endl;

	double xmin = brent(bracket);
	cerr << "  Minimum at " << xmin << endl;
	cerr << "  Min val = " << lineValue(xmin) << " " << lineValue(xmin + 0.01) << " " << lineValue(xmin - 0.01) << endl;
	return lineParametric(xmin);
}

vector<double> Function::localExtremum(const vector<double>& x, bool min) const
{
	// Conjugate gradient algorithm

	static const int maxIter = 250;
	m_success = true;
	setExtremumType(min);
	vector<double> position = x;
	vector<double> direction(x.size(), 0.);
	vector<double> oldGrad;
	int i;

	for (i = 0; (value(position) < 0.99999) && (i < maxIter); i++)
	{
		cerr << "Iteration " << i << endl;
		// Compute the optimization direction
		vector<double> grad = gradient(position);
		double beta = 0.;
		if (!oldGrad.empty())
		{
			//beta = Utils::scalar(grad, grad)/Utils::scalar(oldGrad, oldGrad); // Fletcher-Reeves
			beta = Utils::scalar(grad, grad - oldGrad)/Utils::scalar(oldGrad, oldGrad); // Polak-Ribière
			//beta = Utils::scalar(grad, grad - oldGrad)/Utils::scalar(direction, grad - oldGrad); // Hestenes-Stiefel
			beta = ::max(0., beta);
		}
		direction =  beta*direction - grad;
		oldGrad = grad;

		cerr << " Direction : " << direction[0] << " " << direction[1] << " " << direction[2] << endl;

		position = lineMaximum(position, direction);
		for (vector<double>::iterator it = position.begin(); it != position.end(); it++)
			cerr << " Pos " << (*it) << endl;
		cerr << " Value : " << value(position) << endl;
	}

	if (i == maxIter)
		m_success = false;

	return position;
}

vector<double> Function::absoluteExtremum(bool min) const
{
	m_success = true;
	setExtremumType(min);
	/// @todo
	return vector<double>(2);
}

/// @todo replace this by a random search
vector<double> Function::bracketMinimum(double start, double stop) const
{
	static const double golden = 1.618033988749894848;
	static const double epsilon = 1e-20;
	static const double gLimit = 100.;

	double ulim, fu;

	double a = start;
	double b = stop;

	double fa = lineValue(a);
	double fb = lineValue(b);
	if (fb > fa)
	{
		::swap(a, b);
		::swap(fb, fa);
	}

	double c = b + golden*(b - a);
	double fc = lineValue(c);

	while (fb > fc)
	{
		double r = (b - a)*(fb - fc);
		double q = (b - c)*(fb - fa);
		double u = b - ((b - c)*q - (b - a)*r)/(2.0*::max(double(fabs(q-r)), epsilon)*sign(q-r));
		ulim = b + gLimit*(c - b);
		if ((b - u)*(u - c) > 0.)
		{
			fu = lineValue(u);
			if (fu < fc)
			{
				a = b;
				b = u;
				break;
			}
			else if (fu > fb)
			{
				c = u;
				break;
			}
			u = c + golden*(c - b);
			fu = lineValue(u);
		}
		else if ((c - u)*(u - ulim) > 0.)
		{
			fu = lineValue(u);
			if (fu < fc)
			{
				b = c; c = u; u = c + golden*(c - b);
				fb = fc; fc = fu; fu = lineValue(u);
			}
		}
		else if ((u - ulim)*(ulim - c) >= 0.)
		{
			u = ulim;
			fu = lineValue(u);
		}
		else
		{
			u = c + golden*(c - b);
			fu = lineValue(u);
		}
		a = b; b = c; c = u;
		fa = fb; fb = fc; fc = fu;
	}

	vector<double> result;
	result.push_back(a);
	result.push_back(b);
	result.push_back(c);

	return result;
}

double Function::brent(const vector<double>& bracket) const
{
	static const double maxIter = 100;
	static const double cGolden = 0.3819660112501051518; // (3 - sqrt(5))/2
	static const double maxTolerance = 2e-4;
	static const double epsilon = 1e-10;

	double e = 0.;

	double a = ::min(bracket[0], bracket[2]);
	double b = ::max(bracket[0], bracket[2]);

	double d, fu, fv, fw, fx, u, v, w, x;
	x = w = v = bracket[1];
	fw = fv = fx = lineValue(x);

	for (int iter = 0; iter < maxIter; iter++)
	{
		double tolerance = maxTolerance*fabs(x) + epsilon;
		// Required acuracy reached
		if ((b - a)/2. < tolerance)
			return x;

		double xm = 0.5*(a + b);
		double p = 0., q = 0., r = 0.;

		if (fabs(e) > tolerance)
		{
			// Parabolic interpolation
			r = (x - w)*(fx - fv);
			q = (x - v)*(fx - fw);
			p = (x - v)*q - (x - w)*r;
			q = 2.*(q - r);
			if (q > 0.)
				p = -p;
			else
				q = -q;
		}

		if ((fabs(p) < fabs(.5*q*e)) && (p > q*(q-x)) && (p < q*(b-x)))
		{
			e = d;
			d = p / q;
			u = x + d;
			if ((0.5*(u - a) < tolerance) || (0.5*(b - u) < tolerance))
				d = sign(xm - x)*tolerance;
		}
		else
		{
			e = x < xm ? b-x : a-x;
			d = cGolden*e;
		}

		u = x + sign(d)*::max(fabs(d), tolerance);
		fu = lineValue(u);

		if (fu <= fx)
		{
			if (u < x)
				b = x;
			else
				a = x;
			v = w; w = x; x = u;
			fv = fw; fw = fx; fx = fu;
		}
		else
		{
			if (u < x)
				a = u;
			else
				b = u;
			if (fu <= fw || w == x)
			{
				v = w; w = u;
				fv = fw; fw = fv;
			}
			else if (fu <= fv || v == x || v == w)
			{
				v = u;
				fv = fu;
			}
		}
	}
	return x;
}
