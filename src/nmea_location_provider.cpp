/*
 * nmea_location_provider.cpp
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#include <string>
using namespace std;

#include "nmea_location_provider.h"
#include "nmea_packet.h"
#include "config_file.h"
#include "log_file.h"
#include "utils.h"
#include "platform.h"

const char *NmeaLocationProvider::module = "NMEA";

NmeaLocationProvider::NmeaLocationProvider(ConfigFile::HashType config) : LocationProvider(config) {
	string port_name = config["port"];
	if (port_name.empty()) port_name = "/dev/ttyS0";

	unsigned long baudrate = stoul(config["baudrate"], 9600);
	unsigned int databits = stou(config["databits"], 8);
	unsigned int stopbits = stou(config["stopbits"], 1);
	unsigned int timeout = stou(config["timeout"], 2);
	bool flowctl = stob(config["flowcontrol"], true);
	debug = stob(config["debug"], false);
	time_source = stob(config["time_source"], true);

	min_satellites = stou(config["min_satellites"], 4);
	max_hdop = stof(config["max_hdop"], 4);
	max_speed = stod(config["max_speed"], 180) / 3.6;

	num_satellites = 0;
	prev_num_satellites = 0;

	string llog = config["location_log"];
	if (llog.empty())
		location_log = NULL;
	else
		location_log = fopen(llog.c_str(), "a");

	if (Platform::get_instance()) Platform::get_instance()->start_gps();

	if (port.open_port((char *)port_name.c_str())) {
		port.set_mode(baudrate, databits, stopbits, SerialPort::None, flowctl, timeout * 10);
		LOG_DEBUG(module, "Opened port %s", port_name.c_str());
	} else {
		LOG_ERROR(module, "Error opening port %s", port_name.c_str());
	}

}

NmeaLocationProvider::~NmeaLocationProvider() {
	if (location_log) fclose(location_log);
	LOG_DEBUG(module, "Closing port");
	port.close_port();
}

Location NmeaLocationProvider::get_location(void) {
	NmeaPacket packet;
	string str;
	Location location(Location::GPS);

	if (!this->port.is_open()) return location;

	while (!location.has_fix) {
		str = this->port.read_string('\x0A', '$');
		if (str.length() == 0) return location;
		if (debug) LOG_DEBUG(module, "Got string: %s", str.c_str());

		packet.set_data(str);
		if (!packet.is_valid()) continue;

		NmeaPacket::PacketType ptype = packet.get_type();
		if (ptype == NmeaPacket::GPRMC) {	     // Recommended Minimum Navigation Information
			//		if (packet.get_char(2) == 'V') return location;					// Receiver warning
			location.set_time(packet.get_string(1), packet.get_string(9));
			location.set_latitude(packet.get_double(3), packet.get_char(4));
			location.set_longitude(packet.get_double(5), packet.get_char(6));
			if (location.longitude == 0.0) {
				if (Platform::get_instance()) Platform::get_instance()->on_gps_data(false);
				return location;
			}
			if (time_source && Platform::get_instance())
				Platform::get_instance()->set_time(location.time);
			location.set_speed_kts(packet.get_double(7));
			location.track = packet.get_float(8);
			location.has_fix = true;
			location.has_speed = true;
			if (Platform::get_instance()) Platform::get_instance()->on_gps_data(true);
		} else if (ptype == NmeaPacket::GPGGA) { // Global Positioning System Fix Data.
															 // Time, Position and fix related data for a GPS receiver
			prev_num_satellites = num_satellites;
			num_satellites = packet.get_int(7);
			hdop = packet.get_float(8);
			location.num_satellites = num_satellites;
			location.hdop = hdop;
		}

	}
	LOG_DEBUG(module, "Got fix: %lf, %lf, %lf, %f, %d", location.latitude, location.longitude, location.speed, location.hdop, location.num_satellites);
	if (location_log) {
		fputs((location.to_string() + "\n").c_str(), location_log);
		fflush(location_log);
	}

	return location;
}

bool NmeaLocationProvider::location_acceptable(Location l) {
	return l.num_satellites >= int(min_satellites) &&
	       l.hdop <= max_hdop &&
		   l.speed <= max_speed &&
		   prev_num_satellites == num_satellites;
}

void NmeaLocationProvider::thread_body(void) {
	Location new_location;
	while (!m_stoprequested) {
		new_location = get_location();
		if (!(new_location.has_fix && location_acceptable(new_location)))
			continue;
		current_location = kf.filter(new_location);
		callback(current_location);
	}
}

