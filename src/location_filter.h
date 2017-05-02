/*
 * location_filter.h
 *
 *  Created on: 12/11/2011
 *      Author: boris
 */

#ifndef LOCATION_FILTER_H_
#define LOCATION_FILTER_H_

#include "location.h"

class LocationFilter {
	private:
		double v1, v2, q1, q2;
		double p;
		Location loc;
		const static char *module;
	public:
		LocationFilter(void);
		LocationFilter(double iv1, double iv2, double iq1, double iq2, double p0);
		Location filter(Location l);
};

#endif /* LOCATION_FILTER_H_ */
