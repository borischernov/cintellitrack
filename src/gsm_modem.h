/*
 * gsm_modem.h
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#ifndef GSM_MODEM_H_
#define GSM_MODEM_H_

#include <string>
using namespace std;

#include "threaded_class.h"
#include "serialport.h"
#include "config_file.h"
#include "mutex.h"

class GsmModem : public threaded_class {
public:
	enum CmdStatus {None, OK, Error, Idle};
protected:
	static GsmModem *instance;
	SerialPort port;
	Mutex mutex;
	string command_response;
	volatile CmdStatus command_status;
	string init_string;
	string post_init_string;
	string pin_code;
	int rssi;
	bool do_encode_pdu;
	static const char *module;
	void (*ussd_callback)(string);

	void thread_body(void);
	string encode_pdu(string str);
	string decode_pdu(string pdu);
	void process_info(string info);
	void process_notification(string info);
	GsmModem::CmdStatus run_command(string command, string *response);
	bool enter_pin(void);
public:
	static GsmModem *get_instance(void) {
		if (instance == NULL) instance = new GsmModem();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

	GsmModem();
	~GsmModem();
	bool init(void);
	bool send_ussd(string ussd);
	bool pin_status(void);
	int get_rssi(void) { return rssi; };
	void set_ussd_callback(void (*cb)(string)) { ussd_callback = cb; };
};

#endif /* GSM_MODEM_H_ */
