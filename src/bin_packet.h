/*
 * bin_packet.h
 *
 *  Created on: 21/11/2011
 *      Author: boris
 */

#ifndef BIN_PACKET_H_
#define BIN_PACKET_H_

#include <string>
using namespace std;

class BinPacket {
private:
	string buf;
	unsigned int chrbuf;
	unsigned int bitcnt;
	static const char *alpha;
public:
	BinPacket() { chrbuf = bitcnt = 0; };
	void add_unsigned(unsigned long val, unsigned int bits);
	void add_signed(long val, unsigned int bits);
	string to_string(void);
};

#endif /* BIN_PACKET_H_ */
