/*
 * communication_manager.cpp
 *
 *  Created on: 10/10/2011
 *      Author: boris
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
using namespace std;

#include "location_manager.h"
#include "wifi_manager.h"
#include "communication_manager.h"
#include "message_queue.h"
#include "memory_queue.h"
#include "log_file.h"
#include "location.h"
#include "config_file.h"
#include "sysutils.h"
#include "bin_packet.h"

const char *CommunicationManager::module = "COMM_MANAGER";
CommunicationManager *CommunicationManager::instance = NULL;

CommunicationManager::CommunicationManager() {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	config->get_group("communication_manager", &group, true);
	ussd_prefix = group["ussd_prefix"];
	ussd_suffix = group["ussd_suffix"];
	ussd_response_ok = group["ussd_response_ok"];
	retry_interval = stou(group["retry_interval"], 60);
	max_message_length = stou(group["max_message_length"], 130);
	max_aggregation_time = stou(group["max_aggregation_time"], 120);
	max_diff_locations = stou(group["max_diff_locations"], 10);

	config->get_group("message_queue", &group, true);
	string qtype = group["type"];
	if (!qtype.compare("memory")) {
		LOG_DEBUG(module, "Using memory queue");
		mqueue = (MessageQueue *)(new MemoryQueue());
	} else {
		LOG_ERROR(module, "Unknown message queue type; using memory queue");
		mqueue = (MessageQueue *)(new MemoryQueue());
	}
	waiting_dlr = false;
	diff_locations = 0;

	LOG_DEBUG(module, "Starting modem");
	modem = GsmModem::get_instance();
	if (!modem->init()) LOG_DEBUG(module, "Error initializing modem");
	modem->set_ussd_callback(CommunicationManager::ussd_callback);
	messages_sent = 0;

	LOG_DEBUG(module, "Starting WiFi manager");
	wifi = new WifiManager();
}

CommunicationManager::~CommunicationManager() {
	LOG_DEBUG(module, "Destroying CommunicationManager");
	stop_thread();
	delete mqueue;
	delete wifi;
	GsmModem::destroy_instance();
}

string CommunicationManager::encode_location(Location l) {
	BinPacket p;
	p.add_unsigned(1, 4);
	p.add_unsigned(l.provider, 1);
	p.add_signed(long(l.latitude * 1000000U), 28);
	p.add_signed(long(l.longitude * 1000000U), 29);
	p.add_unsigned((unsigned int)(l.has_speed ? l.speed : 127), 7);
	p.add_unsigned((unsigned long)l.time, 31);
	p.add_unsigned((unsigned long)l.mode, 2);
	return p.to_string();
}

string CommunicationManager::encode_diff_location(Location l) {
	BinPacket p;
	double d;
	unsigned long t;
	p.add_unsigned(2, 4);
	p.add_unsigned(base_location.time & 0xF, 4);
	d = (l.latitude - base_location.latitude) * 1000000U;
	if (abs(long(d)) >= 16384) return string("");
	p.add_signed(d, 15);
	d = (l.longitude - base_location.longitude) * 1000000U;
	if (abs(long(d)) >= 16384) return string("");
	p.add_signed(d, 15);
	t = l.time - base_location.time;
	if (t > 8192) return string("");
	p.add_unsigned(t, 13);
	p.add_unsigned((unsigned int)(l.has_speed ? l.speed : 127), 7);
	p.add_unsigned((unsigned long)l.mode, 2);
	return p.to_string();
}

bool CommunicationManager::send_location_update(Location l) {
	string msg;
	mutex.lock();
	if (base_location.has_fix && diff_locations < max_diff_locations && (msg = encode_diff_location(l)) != "")
		diff_locations++;
	else {
		msg = encode_location(l);
		diff_locations = 0;
		base_location = l;
	}
	mqueue->put_message(msg);
	mutex.unlock();
	start_thread();
	return true;
}

void CommunicationManager::incoming_ussd(string msg) {
	if (starts_with(msg, "cmd:")) {
		process_command(msg.substr(4, string::npos));
	} else if (msg == ussd_response_ok) {
		LOG_DEBUG(module, "Message delivery confirmed");
		waiting_dlr = false;
		messages_sent++;
	}
}

void CommunicationManager::process_command(string cmd) {
	if (starts_with(cmd, "shell:")) {								// Execute shell command
		scmd = hex_decode(cmd.substr(6, string::npos));
		LOG_DEBUG(module, "Shell command requested: [%s]", scmd.c_str());
		pthread_t cmd_thread;
		pthread_create(&cmd_thread, 0, &CommunicationManager::exec_shell, (void *)scmd.c_str());
	} else if (cmd == "locrequest") { 								// Location request
		LOG_DEBUG(module, "Location requested");
		Location loc = LocationManager::get_instance()->get_location();
		if (loc.has_fix) send_location_update(loc);
	} else if (starts_with(cmd, "set:distol:")) { 					// Set distance tolerance
		double t = stod(cmd.substr(11, string::npos), 0);
		LOG_DEBUG(module, "Distance tolerance set request: %lf", t);
		LocationManager::get_instance()->set_distance_tolerance(t);
	} else if (starts_with(cmd, "set:timtol:")) { 					// Set time tolerance
		time_t t = stoul(cmd.substr(11, string::npos), 0);
		LOG_DEBUG(module, "Time tolerance set request: %lu", t);
		LocationManager::get_instance()->set_time_tolerance(t);
	} else if (cmd == "sysinfo") { 								// Sysinfo request
		LOG_DEBUG(module, "Sysinfo request received");
		mqueue->put_message(prepare_status_msg());
		start_thread();
	} else if (cmd == "wifi:up") { 								// Wifi Up
		LOG_DEBUG(module, "WiFi up request");
		wifi->up();
	} else if (cmd == "wifi:down") { 								// Wifi Down
		LOG_DEBUG(module, "WiFi down request");
		wifi->down();
	}
}

void CommunicationManager::thread_body(void) {
	string buf;
	LOG_DEBUG(module, "Starting thread");
	while (!m_stoprequested && mqueue->size() > 0) {
		while (!m_stoprequested && mqueue->size() > 0 && (mqueue->length() >= max_message_length || mqueue->age() >= max_aggregation_time)) {
			LOG_DEBUG(module, "Filling the buffer");
			// Fill buffer with messages
			mutex.lock();
			for(buf = ""; mqueue->size() > 0 && (buf.size() + mqueue->get_message().size() <= max_message_length); mqueue->remove_message())
				buf += mqueue->get_message();
			mutex.unlock();
			buf = ussd_prefix + buf + ussd_suffix;
			LOG_DEBUG(module, "Sending message: %s", buf.c_str());
			// Send buffer
			for(waiting_dlr = true; waiting_dlr; ) {
				modem->send_ussd(buf);
				for(unsigned int t = retry_interval; waiting_dlr && t > 0; t--)
					sleep(1);
			}
		}
		sleep(5);
	}
	LOG_DEBUG(module, "Exiting thread");
	m_running = false;
}

string CommunicationManager::prepare_status_msg(void) {
	BinPacket p;
	LocationManager *lm = LocationManager::get_instance();
	p.add_unsigned(15, 4);
	p.add_unsigned(1, 4);
	p.add_unsigned((unsigned int)(lm->get_num_satellites()), 5);
	p.add_unsigned(lm->get_location().has_fix ? 1: 0, 1);
	p.add_unsigned(wifi->is_connected() ? 1 : 0, 1);
	p.add_unsigned((unsigned long)(uptime()), 20);
	p.add_unsigned((unsigned int)(modem->get_rssi()), 7);
	return p.to_string();
}

void *CommunicationManager::exec_shell(void *cmd) {
	system((char *)cmd);
	return NULL;
}
