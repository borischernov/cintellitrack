/*
 * log_file.h
 *
 *  Created on: 10/10/2011
 *      Author: boris
 */

#ifndef LOG_FILE_H_
#define LOG_FILE_H_

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "mutex.h"

class LogFile {
public:
	enum Severity {Debug, Info, Warning, Error};
private:
	FILE *logfile;
	Severity min_severity;
	static LogFile *instance;
	Mutex mutex;
public:
	static LogFile *get_instance(void) {
		if (instance == NULL) instance = new LogFile();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

	LogFile(void) : logfile(NULL), min_severity(Debug) {};

	~LogFile() { close_log(); };

	bool open_log(char *log_file) {
		close_log();
		return (logfile = fopen(log_file, "a"));
	}

	void close_log(void) {
		if (logfile) fclose(logfile);
		logfile = NULL;
	}

	void log(const char *module, Severity severity, const char *fmt, ...);

	void set_level(Severity level) { min_severity = level; };
};

#define LOG_DEBUG(module, format, args...)  \
   LogFile::get_instance()->log(module, LogFile::Debug, format, ## args)

#define LOG_INFO(module, format, args...)  \
   LogFile::get_instance()->log(module, LogFile::Info, format, ## args)

#define LOG_ERROR(module, format, args...)  \
   LogFile::get_instance()->log(module, LogFile::Error, format, ## args)

#endif /* LOG_FILE_H_ */
