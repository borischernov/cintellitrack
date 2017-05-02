/*
 * config_file.cpp
 *
 *  Created on: 07/10/2011
 *      Author: boris
 */

#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
using namespace std;

#include "config_file.h"
#include "utils.h"

ConfigFile *ConfigFile::instance = NULL;

/**********************************************************************************
 * Get next line from config
 * Return values
 *
 */

ConfigFile::ParseRes ConfigFile::next_line(ConfigPair *cpair) {
	char buf[256];
	char *val;

	do {
		// Get string
		if (!fgets(buf, sizeof(buf), this->config)) return ConfigFile::ReadError;
		this->curr_line++;
		rtrim(buf);
		if (buf[0] == 0) return ConfigFile::Empty;
	} while(buf[0] == '#');

	if (!(val = strchr(buf, '=')) || val == buf) {
		this->set_error("Invalid config line");
		return ConfigFile::ParseError;
	}
	*val++ = 0;

	trim(buf);
	trim(val);

	cpair->first = string(buf);
	cpair->second = string(val);

	return ConfigFile::OK;
}

ConfigFile::ParseRes ConfigFile::get_group(string group_name, ConfigFile::HashType *group, bool rewnd) {
	ConfigPair cpair;
	ParseRes res;

	if (!config) {
		this->set_error("Config file is not open");
		return ConfigFile::ReadError;
	}

	if (rewnd) rewind();

	for(;;)  // Position to the beginning of the group
	{
		while((res = this->next_line(&cpair)) == ConfigFile::Empty);
		if (res != ConfigFile::OK) return res;

		if (cpair.first.compare("group")) {
			this->set_error("Group statement expected");
			return ConfigFile::ParseError;
		}

		if (!cpair.second.compare(group_name)) break;

		while((res = this->next_line(&cpair)) == ConfigFile::OK);
		if (res != ConfigFile::Empty) return res;
	}


	for(group->clear(); (res = this->next_line(&cpair)) == ConfigFile::OK; group->insert(cpair))
		if (!cpair.first.compare("group")) {
			this->set_error("Unexpected group statement");
			return ConfigFile::ParseError;
		}

	return res == ConfigFile::ParseError ? ConfigFile::ParseError : ConfigFile::OK;
}
