/*
 * config_file.h
 *
 *  Created on: 07/10/2011
 *      Author: boris
 */

#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

#include <stdio.h>
#include <map>
#include <string>
#include <utility>
using namespace std;

#include "utils.h"

class ConfigFile {
private:
	static ConfigFile *instance;
public:
	static ConfigFile *get_instance(void) {
		if (instance == NULL) instance = new ConfigFile();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

	typedef map<string, string> HashType;
	typedef pair<string, string> ConfigPair;
	enum ParseRes {OK, Empty, ReadError, ParseError};

	ConfigFile(void) { this->config = NULL; };
	~ConfigFile() { this->close_config(); };
	bool open_config(char * path) { this->close_config(); this->curr_line = 0; return (this->config = fopen(path, "r")) != NULL; };
	void close_config(void) { if (this->config) { fclose(this->config); this->config = NULL; } };
	void rewind(void) { fseek(this->config, 0, SEEK_SET); this->curr_line = 0; };
	ParseRes get_group(string group_name, HashType *group, bool rewnd = true);
	string get_error(void) { return err_message + string(" at line ") + itos(this->err_line); };
	int get_current_line(void) { return curr_line; };

private:
	FILE *config;
	int err_line;
	int curr_line;
	string err_message;

	ParseRes next_line(ConfigPair *cpair);
	void set_error(string msg) { this->err_line = this->curr_line; this->err_message = msg; };
};


#endif /* CONFIG_FILE_H_ */
