/*
 * wifi_manager.cpp
 *
 *  Created on: 22/10/2011
 *      Author: boris
 */

#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>
using namespace std;

#include "wifi_manager.h"
#include "log_file.h"
#include "config_file.h"
#include "utils.h"

const char *WifiManager::module = "WIFI_MANAGER";

WifiManager::WifiManager() {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	if (config->get_group("wifi_manager", &group, true) != ConfigFile::OK) {
		LOG_DEBUG(module, "No wifi_manager configuration group found; not starting.");
		return;
	}

	interface = group["interface"];
	if (interface.empty()) interface = "wlan0";

	connect_script = group["connect_script"];
	if (connect_script.empty()) connect_script = "/opt/intellitrack/bin/wifi_connect.sh";

	search_period = stou(group["search_period"], 30);
	check_period = stou(group["check_period"], 3);

	bool autostart = stob(group["autostart"], false);

			KnownWifiNetwork net;
	for (bool rewind = true; config->get_group("wifi_network", &group, rewind) == ConfigFile::OK; rewind = false, net.clear()) {
		net.bssid = group["bssid"];
		net.essid = group["essid"];
		net.key = group["key"];
		if (net.is_valid()) {
			known_networks.push_back(net);
			LOG_DEBUG(module, "Known network loaded: %s (%s)", net.essid.c_str(), net.bssid.c_str());
		} else {
			LOG_ERROR(module, "Invalid known network record: %s (%s)", net.essid.c_str(), net.bssid.c_str());
		}
	}


	current_network = -1;
	if (autostart)
		start_thread();
}

int WifiManager::scan(void) {
	char buf[256];
	FILE *p;
	WifiNetwork net;
	char *pos;

	sprintf(buf, "iwlist %s scan 2>&1", interface.c_str());
	if (!(p = popen(buf, "r"))) return -1;

	network_list.clear();

	LOG_DEBUG(module, "Scanning for networks");
	fgets(buf, sizeof(buf), p);
	if (!strstr(buf, "Scan completed")) {
		LOG_DEBUG(module, "iwlist error: %s", buf);
		pclose(p);
		return -1;
	}
	for (; fgets(buf, sizeof(buf), p); ) {
		trim(buf);
		if (strstr(buf, "Cell") == buf) {
			if (net.is_valid())
				network_list.push_back(net);
			net.clear();
		}
		if ((pos = strstr(buf, "Address:"))) {
			net.bssid = string(pos + 9);
		} else if ((pos = strstr(buf, "Quality="))) {
			sscanf(pos + 8, "%u", &net.signal_quality);
		} else if ((pos = strstr(buf, "Encryption key:off"))) {
			net.encryption = WifiNetwork::None;
		} else if ((pos = strstr(buf, "WPA Version"))) {
			net.encryption = WifiNetwork::WPA;
		} else if ((pos = strstr(buf, "WPA2 Version"))) {
			net.encryption = WifiNetwork::WPA2;
		} else if ((pos = strstr(buf, "ESSID:"))) {
			buf[strlen(buf) - 1] = 0;
			net.essid = string(pos + 7);
		} else if ((pos = strstr(buf, "Channel:"))) {
			sscanf(pos + 8, "%u", &net.channel);
		}
	}
	if (net.is_valid())
		network_list.push_back(net);
	pclose(p);

	sort(network_list.begin(), network_list.end(), ss_compare);

	return network_list.size();
}

bool WifiManager::connect(WifiNetwork net, string key) {
	if (net.encryption != WifiNetwork::None and key.size() == 0) return false;
	LOG_DEBUG(module, "Connecting to %s (%s)", net.essid.c_str(), net.bssid.c_str());

	char buf[256];
	char *enctypes[] = {(char*)"", (char*)"none", (char *)"wep", (char *)"psk", (char *)"psk2"};
	sprintf(buf, "/bin/sh %s %s %d '%s' '%s' '%s' '%s' ",
			connect_script.c_str(), interface.c_str(), net.channel, net.essid.c_str(), net.bssid.c_str(),
			enctypes[net.encryption], key.c_str());
	LOG_DEBUG(module, "Connect command: %s", buf);
	system(buf);

	return true;
}

bool WifiManager::connection_ok(void) {
	FILE *p;
	char buf[256];
	char *pos;
	int rp = 0;

	if (!(p = popen("ping -c3 -q -w5 www.google.com", "r"))) return false;
	while (fgets(buf, sizeof(buf), p))
		if ((pos = strstr(buf, "transmitted"))) {
			sscanf(pos + 13, "%d", &rp);
			break;
		}
	pclose(p);
	LOG_DEBUG(module,"Connection check: %d/3 packets received", rp);

	return rp > 0;
}

string WifiManager::key_for_network(WifiNetwork net) {
	for (size_t c = 0; c < known_networks.size(); c++)
		if (net.bssid == known_networks[c].bssid || net.essid == known_networks[c].essid)
			return known_networks[c].key;
	return string("");
}

void WifiManager::thread_body(void) {

	LOG_DEBUG(module, "Starting WiFi manager thread");

	while (!m_stoprequested) {

		for(current_network = -1; !m_stoprequested && current_network == -1; sleep(search_period)) {		// Network search
			scan();
			for(size_t c = 0; c < network_list.size(); c++ ) {
				WifiNetwork net = network_list[c];
				string key;

				if (net.encryption != WifiNetwork::None) {
					key = key_for_network(net);
					if (key.empty()) continue;
					LOG_DEBUG(module, "Known network found: %s (%s)", net.essid.c_str(), net.bssid.c_str());
				} else {
					LOG_DEBUG(module, "Open network found: %s (%s)", net.essid.c_str(), net.bssid.c_str());
				}

				connect(net, key);

				if (connection_ok()) {
					current_network = c;
					break;
					LOG_DEBUG(module, "Successfully connected; switching to check mode.");
				}
			}
		}

		while(!m_stoprequested && connection_ok()) sleep(check_period);

		LOG_DEBUG(module, "Connection lost; switching to search mode.");

	}

	LOG_DEBUG(module, "Finalizing WiFi manager thread");
}

void WifiManager::up(void) {
	start_thread();
}

void WifiManager::down(void) {
	current_network = -1;
	stop_thread();
}
