/*
 * wifi_network.h
 *
 *  Created on: 23/10/2011
 *      Author: boris
 */

#ifndef WIFI_NETWORK_H_
#define WIFI_NETWORK_H_

#include <string>
using namespace std;

class WifiNetwork {
public:
	enum Encryption { Unknown, None, WEP, WPA, WPA2 };
	string bssid;
	string essid;
	Encryption encryption;
	unsigned int channel;
	unsigned int signal_quality;
	WifiNetwork() {};
	bool is_valid(void) { return bssid.size() == 17 && essid.size() > 0 && signal_quality > 0 && encryption != Unknown && channel > 0; };
	void clear(void) { bssid = string(""); essid = string(""); signal_quality = 0; encryption = Unknown; channel = 0; };
};

class KnownWifiNetwork {
public:
	string bssid;
	string essid;
	string key;
	bool is_valid() { return !key.empty() && (!essid.empty() || bssid.size() == 17); };
	void clear() { bssid = string(""); essid = string(""); key = string(""); };
};

#endif /* WIFI_NETWORK_H_ */
