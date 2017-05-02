/*
 * log_file.cpp
 *
 *  Created on: 10/10/2011
 *      Author: boris
 */

#include "log_file.h"

LogFile *LogFile::instance = NULL;

void LogFile::log(const char *module, Severity severity, const char *fmt, ...) {
	if (severity < min_severity) return;

	time_t rawtime;
	struct tm * timeinfo;
	char buffer[64];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime (buffer, sizeof(buffer), "%d.%m.%y %H:%M:%S", timeinfo);

	const char *sevstr[] = {"DEBUG", "INFO ", "WARN ", "ERROR"};

	mutex.lock();

	fprintf(logfile, "%s [%s] {%s} ", buffer, sevstr[severity], module);

	va_list args;
	va_start (args, fmt);
	vfprintf (logfile, fmt, args);
	va_end (args);
	fprintf(logfile, "\n");

	fflush(logfile);

	mutex.unlock();
}

