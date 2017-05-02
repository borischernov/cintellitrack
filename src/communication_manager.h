/*
 * communication_manager.h
 *
 *  Created on: 10/10/2011
 *      Author: boris
 */

#ifndef COMMUNICATION_MANAGER_H_
#define COMMUNICATION_MANAGER_H_

#include <string>
using namespace std;

#include "message_queue.h"
#include "gsm_modem.h"
#include "location.h"
#include "wifi_manager.h"
#include "mutex.h"

class CommunicationManager : public threaded_class {
protected:
	MessageQueue *mqueue;
	GsmModem *modem;
	WifiManager *wifi;
	Mutex mutex;
	const static char *module;
	static CommunicationManager *instance;
	size_t messages_sent;
	string ussd_prefix;
	string ussd_suffix;
	string ussd_response_ok;
	unsigned int retry_interval;
	string scmd;
	unsigned int max_message_length;
	unsigned int max_aggregation_time;
	volatile bool waiting_dlr;
	Location base_location;
	unsigned int max_diff_locations;
	unsigned int diff_locations;

	string encode_location(Location l);
	string encode_diff_location(Location l);
	string prepare_status_msg(void);
	void send_queue(void);
	static void ussd_callback(string msg) { CommunicationManager::get_instance()->incoming_ussd(msg); }
	void incoming_ussd(string msg);
	void process_command(string cmd);
	void thread_body(void);

    static void *exec_shell(void *cmd);

public:
	static CommunicationManager *get_instance(void) {
		if (instance == NULL) instance = new CommunicationManager();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

	CommunicationManager();
	~CommunicationManager();
	bool send_location_update(Location l);
	size_t get_queue_size(void) { return mqueue->size(); }
	size_t get_messages_sent(void) { return messages_sent; }
};


#endif /* COMMUNICATION_MANAGER_H_ */
