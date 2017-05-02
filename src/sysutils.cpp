/*
 * sysutils.cpp
 *
 *  Created on: 01/11/2011
 *      Author: boris
 */

#include "sysutils.h"

#include <sys/sysinfo.h>

long uptime(void) {
	struct sysinfo info;

	sysinfo(&info);

	return info.uptime;
}
