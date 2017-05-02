/*
 * gsm_modem.cpp
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#include <stdio.h>

#include "utils.h"
#include "gsm_modem.h"
#include "log_file.h"
#include "config_file.h"
#include "platform.h"

const char *GsmModem::module = "GSM_MODEM";
GsmModem *GsmModem::instance = NULL;

GsmModem::GsmModem(void) {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	config->get_group("gsm_modem", &group, true);

	string port_name = group["port"];
	if (port_name.empty()) port_name = "/dev/ttyS0";

	unsigned long baudrate = stoul(group["baudrate"], 9600);
	unsigned int databits = stou(group["databits"], 8);
	unsigned int stopbits = stou(group["stopbits"], 1);
	unsigned int timeout = stou(group["timeout"], 2);
	bool flowctl = stob(group["flowcontrol"], true);
	do_encode_pdu = stob(group["encode_pdu"], true);

	init_string = group["init_string"];
	if (init_string.empty()) init_string = "ATZ";

	post_init_string = group["post_init_string"];
	if (post_init_string.empty()) post_init_string = "AT+CUSD=1";

	pin_code = group["pin_code"];
	if (pin_code.empty()) pin_code = "0000";

	command_status = GsmModem::Idle;

	if (Platform::get_instance()) Platform::get_instance()->start_gsm();

	if (port.open_port((char *)port_name.c_str())) {
		LOG_DEBUG(module, "Opened port %s", port_name.c_str());
		port.set_mode(baudrate, databits, stopbits, SerialPort::None, flowctl, timeout * 10);
		start_thread();
		LOG_DEBUG(module, "Started modem thread");
	} else {
		LOG_ERROR(module, "Can't open port %s", port_name.c_str());
	}

	rssi = -1;
	ussd_callback = NULL;
}

GsmModem::~GsmModem() {
	stop_thread();
	port.close_port();
}

GsmModem::CmdStatus GsmModem::run_command(string command, string *response) {
	CmdStatus ret;

	LOG_DEBUG(module, "Command: %s", command.c_str());

	mutex.lock();

	command_response = "";
	command_status = GsmModem::None;

	port.write_string(command + "\x0A");

	for (int c=0; c < 3 && command_status == GsmModem::None; c++)
		sleep(1);

	LOG_DEBUG(module, "Status: %d Response: %s", command_status, command_response.c_str());

	ret = command_status;
	command_status = GsmModem::Idle;

	if (response) *response = command_response;

	mutex.unlock();

	return ret;
}

bool GsmModem::init(void) {
	int c;

	for (c = 5; run_command(init_string, NULL) != GsmModem::OK && c > 0; c--)
		sleep(2);
	if (!c) return false;

	sleep(2);
	if (!enter_pin()) return false;

	sleep(2);
	return run_command(post_init_string, NULL) == GsmModem::OK;
}

void GsmModem::thread_body(void) {
	string line;
	command_response = "";

	while (!m_stoprequested) {
		char *ptr = (char *)(port.read_string(char('\x0A')).c_str());
		rtrim(ptr);
		line = ptr;
		if (line.empty()) continue;

		switch (line[0]) {
			case '^':     // additional info
				LOG_DEBUG(module, "INFO: [%s]", line.c_str());
				process_info(line);
				break;
			case '+': 	  // notification
				if (command_status == GsmModem::None)
				{
					LOG_DEBUG(module, ">> [%s]", line.c_str());
					command_response += line + "|";
				}
				else
					LOG_DEBUG(module, "NOTIFICATION: [%s]", line.c_str());
					process_notification(line);
				break;
			default:	  // command or response
				LOG_DEBUG(module, ">> [%s]", line.c_str());
				command_response += line + "|";
				if (!line.compare("OK"))
					command_status = GsmModem::OK;
				else if (line.find("ERROR") != string::npos ||
						 line.find("COMMAND NOT SUPPORT") != string::npos )
					command_status = GsmModem::Error;
				break;
		}

	}
}

bool GsmModem::pin_status(void) {
	string resp;

	if (run_command("AT+CPIN?", &resp) != GsmModem::OK) return false;
	return resp.find("READY") != string::npos;
}

bool GsmModem::enter_pin(void) {

	if (pin_status()) return true;

	if (run_command("AT+CPIN=\"" + pin_code + "\"", NULL) != GsmModem::OK) return false;

	return pin_status();
}

bool GsmModem::send_ussd(string ussd) {
	string str = do_encode_pdu ? (encode_pdu(ussd) + ",15") : ("\"" + ussd + "\"");
	return run_command("AT+CUSD=1," + str, NULL) == GsmModem::OK;
}

string GsmModem::encode_pdu(string str) {
	string res = "";
	char buf[8];
	unsigned int b = 0, s = 0;

	for(size_t c = 0; c < str.length(); c++) {
		b += ((unsigned int)str[c] & 0x7F) << s;
		s += 7;
		if (s > 7) {
			s -= 8;
			sprintf(buf, "%02X", b & 0xFF);
			res += buf;
			b >>= 8;
		}	
	}
	if (s) {
		sprintf(buf, "%02X", b & 0xFF);
		res += buf;
	}

	return res;
}

string GsmModem::decode_pdu(string pdu) {
	unsigned int b = 0, s = 0, h = 0;
	string res = "";

	for (size_t c = 0; c < pdu.length(); c+=2) {
		sscanf(pdu.substr(c, 2).c_str(), "%X", &h);
		b += h << s;
		s += 8;
		while (s > 6) {
			res += (char)(b & 0x7F);
			b >>= 7;
			s -= 7;
		}
	}
	if (s && b >= 0x20)
		res += (char)(b & 0x7F);		
	return res;	
}

void GsmModem::process_info(string info) {
	size_t datapos = info.find(":");
	if (datapos == string::npos) {
		LOG_ERROR(module, "Unknown info format");
		return;
	}
	string info_type = info.substr(1, datapos - 1);
	string info_data = info.substr(datapos + 1, string::npos);

	if (!info_type.compare("RSSI")) {
		rssi = (int)stou(info_data);
	}
}

void GsmModem::process_notification(string line) {
//+CUSD: 0,"D074192E0FCFD3EA7A7B0EA2A7CB6B50183EA7CBC3E4307DEE6A",15
	size_t datapos = line.find(":");
	if (datapos == string::npos) {
		LOG_ERROR(module, "Unknown notification format");
		return;
	}
	string notif_type = line.substr(0, datapos);
	string notif_data = line.substr(datapos + 1, string::npos);

	if (!notif_type.compare("+CUSD")) {
		unsigned int enc = stou(get_csv_field(notif_data, 2), 0);
		if (enc != 15) {
			LOG_ERROR(module, "Unknown USSD message encoding: %u", enc);
			return;
		}

		string pdu = get_csv_field(notif_data, 1);
		string msg = do_encode_pdu ? decode_pdu(pdu.substr(1, pdu.size() - 2)) : pdu.substr(1, pdu.size() - 2);
		LOG_DEBUG(module, "Decoded USSD message: [%s]", msg.c_str());
		if (ussd_callback != NULL) ussd_callback(msg);
	}
}
