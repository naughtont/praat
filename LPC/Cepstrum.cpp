/* Cepstrum.cpp
 *
 * Copyright (C) 1994-2012 David Weenink
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 djmw 20010514
 djmw 20020812 GPL header
 djmw 20080122 Version 1: float -> double
 djmw 20110304 Thing_new
*/

#include "Cepstrum.h"
#include "NUM2.h"
#include "Vector.h"

static double getValueAtSample (Cepstrum me, long isamp, long which, int units) {
	(void) units;
	double valsq = my z[1][isamp] * my z[1][isamp];
	if (which == 0) {
		return valsq;
	} else {
		// dB's reference is 1.
		return valsq == 0.0 ? -300.0 : 10.0 * log10 (valsq);
	}
}

Thing_implement (Cepstrum, Matrix, 2);

Cepstrum Cepstrum_create (double qmin, double qmax, long nq) {
	try {
		autoCepstrum me = Thing_new (Cepstrum);
		double dx = (qmax - qmin) / nq;

		Matrix_init (me.peek(), qmin, qmax, nq, dx, qmin + dx / 2, 1, 1, 1, 1, 1);
		return me.transfer();
	} catch (MelderError) {
		Melder_throw ("Cepstrum not created.");
	}
}

void _Cepstrum_draw (Cepstrum me, Graphics g, double qmin, double qmax, double minimum, double maximum, int power, int garnish) {
	int autoscaling = minimum >= maximum;

	Graphics_setInner (g);

	if (qmax <= qmin) {
		qmin = my xmin; qmax = my xmax;
	}

	long imin, imax;
	if (! Matrix_getWindowSamplesX (me, qmin, qmax, & imin, & imax)) {
		return;
	}
	autoNUMvector<double> y (imin, imax);

	for (long i = imin; i <= imax; i++) {
		y[i] = getValueAtSample (me, i, (power ? 1 : 0), 0);
	}

	if (autoscaling) {
		NUMvector_extrema (y.peek(), imin, imax, & minimum, & maximum);
	}

	for (long i = imin; i <= imax; i ++) {
		if (y[i] > maximum) {
			y[i] = maximum;
		} else if (y[i] < minimum) {
			y[i] = minimum;
		}
	}

	Graphics_setWindow (g, qmin, qmax, minimum, maximum);
	Graphics_function (g, y.peek(), imin, imax, Matrix_columnToX (me, imin), Matrix_columnToX (me, imax));

	Graphics_unsetInner (g);

	if (garnish) {
		Graphics_drawInnerBox (g);
		Graphics_textBottom (g, 1, L"Quefrency (s)");
		Graphics_marksBottom (g, 2, TRUE, TRUE, FALSE);
		Graphics_textLeft (g, 1, power ? L"Amplitude (dB)" : L"Amplitude");
		Graphics_marksLeft (g, 2, TRUE, TRUE, FALSE);
	}
}

void Cepstrum_drawLinear (Cepstrum me, Graphics g, double qmin, double qmax, double minimum, double maximum, int garnish) {
	_Cepstrum_draw (me, g, qmin, qmax, minimum, maximum, 0, garnish);
}

void Cepstrum_draw (Cepstrum me, Graphics g, double qmin, double qmax, double dBminimum, double dBmaximum, int garnish) {
	_Cepstrum_draw (me, g, qmin, qmax, dBminimum, dBmaximum, 1, garnish);
}

void Cepstrum_drawTiltLine (Cepstrum me, Graphics g, double qmin, double qmax, double dBminimum, double dBmaximum, double qstart, double qend, int method) {

	Graphics_setInner (g);

	if (qmax <= qmin) {
		qmin = my xmin; qmax = my xmax;
	}

	if (dBminimum >= dBmaximum) { // autoscaling
		long imin, imax;
		if (! Matrix_getWindowSamplesX (me, qmin, qmax, & imin, & imax)) {
			return;
		}
		long numberOfPoints = imax - imin + 1;
		dBminimum = dBmaximum = getValueAtSample (me, imin, 1, 0);
		for (long i = 2; i <= numberOfPoints; i++) {
			long isamp = imin + i - 1;
			double y = getValueAtSample (me, isamp, 1, 0);
			dBmaximum = y > dBmaximum ? y : dBmaximum;
			dBminimum = y < dBminimum ? y : dBminimum;
		}
	}

	Graphics_setWindow (g, qmin, qmax, dBminimum, dBmaximum);
	qend = qend == 0 ? qmax : qend;
	qstart = qstart < qmin ? qmin : qstart;
	qend = qend > qmax ? qmax : qend;

	double a, intercept;
	Cepstrum_fitTiltLine (me, qstart, qend, &a, &intercept, method);

	double y1 = a * qstart + intercept, y2 = a * qend + intercept;
	double lineWidth =  Graphics_inqLineWidth (g);
	Graphics_setLineWidth (g, 2);
	Graphics_line (g, qstart, y1, qend, y2);
	Graphics_setLineWidth (g, lineWidth);
	Graphics_unsetInner (g);
}

/* Fit line y = ax+b in [qmin,qmax] interval */
void Cepstrum_fitTiltLine (Cepstrum me, double qmin, double qmax, double *a, double *intercept, int method) {
	if (qmax <= qmin) {
		qmin = my xmin; qmax = my xmax;
	}

	long imin, imax;
	if (! Matrix_getWindowSamplesX (me, qmin, qmax, & imin, & imax)) {
		return;
	}
	long numberOfPoints = imax - imin + 1;
	autoNUMvector<double> y (1, numberOfPoints);
	autoNUMvector<double> x (1, numberOfPoints);
	for (long i = 1; i <= numberOfPoints; i++) {
		long isamp = imin + i - 1;
		x[i] = my x1 + (isamp - 1) * my dx;
		y[i] = getValueAtSample (me, isamp, 1, 0);
	}
	// fit a line through (x,y)'s
	NUMlineFit(x.peek(), y.peek(), numberOfPoints, a, intercept, method);
}


double Cepstrum_getPeakProminence (Cepstrum me, double search_lowestQuefrency, double search_highestQuefrency, int interpolation, double fit_lowestFrequency, double fit_highestFrequency, int fitmethod, double *qpeak) {
	double a, intercept, qpeakpos, peak;
	Cepstrum_fitTiltLine (me, fit_lowestFrequency, fit_highestFrequency, &a, &intercept, fitmethod);
	Vector_getMaximumAndX ((Vector) me, search_lowestQuefrency, search_highestQuefrency, 1, interpolation, &peak, &qpeakpos);
	double dBPeak = 20 * log10 (peak);
	if (qpeak != NULL) {
		*qpeak = qpeakpos;
	}
	return dBPeak - qpeakpos * a - intercept;
}

Matrix Cepstrum_to_Matrix (Cepstrum me) {
	try {
		autoMatrix thee = Thing_new (Matrix);
		my structMatrix :: v_copy (thee.peek());
		return thee.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": no Matrix created.");
	}
}

Cepstrum Matrix_to_Cepstrum (Matrix me, long row) {
	try {
		autoCepstrum thee = Cepstrum_create (my xmin, my xmax, my nx);
		if (row < 0) {
			row = my ny + 1 - row;
		}
		if (row < 1) {
			row = 1;
		}
		if (row > my ny) {
			row = my ny;
		}
		NUMvector_copyElements (my z[row], thy z[1], 1, my nx);
		return thee.transfer();
	} catch (MelderError) {
		Melder_throw (me, ": no Cepstrum created.");
	}
}

/* End of file Cepstrum.cpp */
