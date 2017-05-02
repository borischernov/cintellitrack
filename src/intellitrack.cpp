/*
 * intellitrack.cpp
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
using namespace std;

#include "serialport.h"
#include "nmea_packet.h"
#include "nmea_location_provider.h"
#include "location.h"
#include "config_file.h"
#include "gsm_modem.h"
#include "location_manager.h"
#include "communication_manager.h"
#include "wifi_manager.h"
#include "log_file.h"
#include "bin_packet.h"
#include "platform.h"

void signal_handler(int sig);
void do_exit(void);
void info_screen(void);
void do_test(void);

int main(int argc, char *argv[]) {


	if (argc != 2) {
		fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
		exit(1);
	}

	// Set up signals handling
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	if (!strcmp(argv[1], "test")) {
		do_test();
		exit(0);
	}

	ConfigFile *config = ConfigFile::get_instance();
	if (!config->open_config(argv[1]))
		LOG_ERROR("MAIN", "Error opening config file");

	ConfigFile::HashType group;
	config->get_group("general", &group, true);

	string log_file = group["log"];
	if (!log_file.empty()) LogFile::get_instance()->open_log((char *)log_file.c_str());

	bool daemonize = stob(group["daemonize"], false);
	LOG_DEBUG("MAIN", daemonize ? "Will daemonize" : "Will run in foreground");

	pid_t pid, sid;

	if (daemonize) {
		LOG_DEBUG("MAIN", "Daemonizing ...");

		pid = fork();
		if (pid < 0) {
			LOG_ERROR("MAIN", "Failed to fork");
			exit(1);
		}

		if (pid > 0) {
			LOG_DEBUG("MAIN", "Parent exiting");
			exit(0);
		} else {
			LOG_DEBUG("MAIN", "In child");
		}

		umask(0);

		sid = setsid();
		if (sid < 0) {
			LOG_ERROR("MAIN", "Failed to setsid");
			exit(1);
		}
/*
		LOG_DEBUG("MAIN", "About to close standard handles");
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
*/
	}

	if (Platform::get_instance()) Platform::get_instance()->startup();
	CommunicationManager::get_instance();
	LocationManager::get_instance();

	if (daemonize) {
		LOG_DEBUG("MAIN", "Main daemon thread sleeping");
		while(true) sleep(60);
	}
	else
		info_screen();

	return 0;
}

void info_screen(void) {
	Location l;
	int rssi;
	CommunicationManager *cm = CommunicationManager::get_instance();

	while (true) {
		l = LocationManager::get_instance()->get_location();
		rssi = GsmModem::get_instance()->get_rssi();

		printf("\x1B[2J"); // Clear screen

		if (l.has_fix) {
			printf("Provider: %s\n", l.provider == 0 ? "GPS" : "NET");
			printf("Fix: %lf / %lf\n", l.latitude, l.longitude);
			if (l.has_speed)
				printf("Speed: %.2lf m/s \n", l.speed);
			else
				puts("Speed: None");

			char buffer[64];
			struct tm *timeinfo = localtime(&l.time);
			strftime(buffer, sizeof(buffer), "%d.%m.%y %H:%M:%S", timeinfo);
			printf("Fix Time: %s\n", buffer);
		} else {
			printf("No Fix\n");
		}

		if (rssi >= 0)
			printf("RSSI: %d\n", rssi);
		else
			puts("RSSI: Unknown");

		printf("Queue size: %u\n", cm->get_queue_size());
		printf("Messages sent: %u\n", cm->get_messages_sent());

		sleep(30);
	}

}

void do_exit(void) {
	LOG_DEBUG("MAIN", "Exiting ...");
	LocationManager::destroy_instance();
	CommunicationManager::destroy_instance();
	ConfigFile::destroy_instance();
	LogFile::destroy_instance();
}

void signal_handler(int sig) {

    switch(sig) {
        case SIGHUP:
            LOG_DEBUG("MAIN","Received SIGHUP signal.");
            break;
        case SIGTERM:
            LOG_DEBUG("MAIN","Received SIGTERM signal.");
            do_exit();
            break;
        default:
        	LOG_DEBUG("MAIN", "Unhandled signal (%d) %s", strsignal(sig));
            do_exit();
            break;
    }
}

void do_test(void) {
	NmeaPacket packet;

	packet.set_data("$PSRF156,23,1,0*09");
//	packet.set_data("$GPRMC,23,1,0*09");
	int ptype = packet.get_type();
	printf("Packet type: %d\n", ptype);
}
