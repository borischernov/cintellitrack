/*
 * bin_backet.cpp
 *
 *  Created on: 21/11/2011
 *      Author: boris
 */

#include <string>
using namespace std;

#include "bin_packet.h"

const char *BinPacket::alpha = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:;";

void BinPacket::add_unsigned(unsigned long val, unsigned int bits) {
	unsigned int c;
	while (bits) {
		c = 6 - bitcnt < bits ? 6 - bitcnt : bits;
		chrbuf <<= c;
		bits -= c;
		bitcnt += c;
		chrbuf |= (val >> bits) & (0x3F >> (6-c));
		if (bitcnt == 6) {
			buf += alpha[chrbuf];
			chrbuf = bitcnt = 0;
		}
	}
}

void BinPacket::add_signed(long val, unsigned int bits) {
	add_unsigned(val < 0 ? 1U : 0U, 1);
	if (val < 0) val = -val;
	add_unsigned((unsigned long)val, bits - 1);
}

string BinPacket::to_string(void) {
	if (bitcnt)
		add_unsigned(0L, 6 - bitcnt);
	return buf;
}
