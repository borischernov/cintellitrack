/*
 * platform.cpp
 *
 *  Created on: 15/04/2012
 *      Author: boris
 */

#include <stdio.h>

#include "platform.h"
#include "ezdog_platform.h"
#include "config_file.h"
#include "log_file.h"

const char *Platform::module = "PLATFORM";
Platform *Platform::instance = NULL;

Platform *Platform::init_platform() {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	config->get_group("platform", &group, true);
	if (!group["type"].compare("ezdog")) {
		LOG_DEBUG(module, "Initializing ezDog platform");
		return new EzdogPlatform();
	} else {
		LOG_ERROR(module, "Unknown platform type: %s (at line %d)",
					group["type"].c_str(), config->get_current_line());
		return NULL;
	}

}
