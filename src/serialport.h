/*
 * serialport.h
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <string>
using namespace std;

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

class SerialPort {

public:
	enum Parity {None = 0, Odd = PARENB | PARODD, Even = PARENB};

	SerialPort(void);
	~SerialPort(void);

	bool open_port(char *port);
	bool is_open(void) { return fd != 0; };
	int set_mode(uint32_t baudrate, int databits, int stopbits, Parity parity, int flowctl, int timeout);
	int write_bytes(char *buf, size_t size);
	int write_string(string buf);
	int read_bytes(char *buf, size_t size);
	string read_string(char delimiter = '\n', char start_with = char(0));
	void close_port(void);

private:
	int fd;

};

#endif /* SERIALPORT_H_ */
