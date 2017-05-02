/*
 * nmea_packet.h
 *
 * NmeaPacket - class for validating and decoding NMEA 0183 packets
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#include <stdio.h>
#include "nmea_packet.h"

bool NmeaPacket::is_valid(void) {
	size_t cspos, i;
	int csum, psum;

	if (this->packet[0] != '$') return false;

	cspos = this->packet.find('*');
	if (cspos == string::npos) return false;

	for (csum = 0, i = 1; i < cspos; i++ )
		csum ^= this->packet[i];

	sscanf(this->packet.substr(cspos + 1).c_str(), "%X", &psum);

	return psum == csum;
}

NmeaPacket::PacketType NmeaPacket::get_type(void) {
	NmeaPacket::PacketType types[] = {GPGGA, GPGLL, GPGSA, GPGSV, GPRMC, GPVTG, Other};
	const char *headers[] = {"GPGGA", "GPGLL", "GPGSA", "GPGSV", "GPRMC", "GPVTG"};
	unsigned int i;

	for (i = 0; types[i] != Other; i++)
		if (this->packet.substr(1, 5).compare(headers[i]) == 0)
			return types[i];

	return Other;
}

string NmeaPacket::get_string(int idx) {
	size_t start = 0, end;

	// beginning of the field
	for (;idx > 0; idx--) {
		start = this->packet.find(',', start + 1);
		if (start == string::npos) return NULL;
	}

	// end of the field
	end = this->packet.find(',', start + 1);
	if (end == string::npos) end = this->packet.find('*', start + 1);
	if (end == string::npos) return NULL;

	return this->packet.substr(start + 1, end - start - 1);
}

int NmeaPacket::get_int(int idx) {
	string fld = this->get_string(idx);
	int res = 0;

	sscanf(fld.c_str(), "%d", &res);
	return res;
}

float NmeaPacket::get_float(int idx) {
	string fld = this->get_string(idx);
	float res = 0;

	sscanf(fld.c_str(), "%f", &res);
	return res;
}

double NmeaPacket::get_double(int idx) {
	string fld = this->get_string(idx);
	double res = 0;

	sscanf(fld.c_str(), "%lf", &res);
	return res;
}

char NmeaPacket::get_char(int idx) {
	return this->get_string(idx)[0];
}
