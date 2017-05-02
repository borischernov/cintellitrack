/*
 * nmea_packet.h
 *
 * NmeaPacket - class for validating and decoding NMEA 0183 packets
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#ifndef NMEA_PACKET_H_
#define NMEA_PACKET_H_

#include <string>
using namespace std;

class NmeaPacket {
private:
	string packet;

public:
	enum PacketType {Other, GPGGA, GPGLL, GPGSA, GPGSV, GPRMC, GPVTG};

	NmeaPacket() {};
	NmeaPacket(char *pkt) { this->set_data(pkt); };
	NmeaPacket(string pkt) { this->set_data(pkt); };
	void set_data(char *pkt) { this->packet = pkt; };
	void set_data(string pkt) { this->packet = pkt; };
	bool is_valid(void);
	NmeaPacket::PacketType get_type(void);
	string get_string(int idx);
	int get_int(int idx);
	float get_float(int idx);
	double get_double(int idx);
	char get_char(int idx);
};

#endif /* NMEA_PACKET_H_ */
