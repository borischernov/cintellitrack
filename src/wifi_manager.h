/*
 * wifi_manager.h
 *
 *  Created on: 22/10/2011
 *      Author: boris
 */

#ifndef WIFI_MANAGER_H_
#define WIFI_MANAGER_H_

#include <string>
#include <vector>
using namespace std;

#include "wifi_network.h"
#include "threaded_class.h"

class WifiManager : public threaded_class {
private:

	const static char *module;
	vector<WifiNetwork> network_list;
	vector<KnownWifiNetwork> known_networks;
	string interface;
	string connect_script;
	unsigned int search_period;
	unsigned int check_period;
	int current_network;

	static bool ss_compare(WifiNetwork n1, WifiNetwork n2) { return n1.signal_quality > n2.signal_quality; };
	void thread_body(void);
	int scan(void);
	bool connection_ok(void);
	bool connect(WifiNetwork net, string key);
	string key_for_network(WifiNetwork net);

public:
	WifiManager();
	~WifiManager() {stop_thread();};
	bool is_connected(void) { return current_network >= 0; };
	void up(void);
	void down(void);
};


#endif /* WIFI_MANAGER_H_ */
