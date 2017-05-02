/*
 * serialport.cpp
 *
 * Interface for interaction with serial port
 *
 *
 *  Created on: 06/10/2011
 *  Author: boris
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "log_file.h"
#include "serialport.h"

SerialPort::SerialPort(void) {
	this->fd = 0;
}

SerialPort::~SerialPort() {
	this->close_port();
}

bool SerialPort::open_port(char *port) {
	if (this->fd != 0) this->close_port();

    this->fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
    	fd = 0;
    	return false;
    }
	fcntl(fd, F_SETFL, 0);
	return true;
}

void SerialPort::close_port(void) {
	if (this->fd != 0) {
		close(this->fd);
		this->fd = 0;
	}
}

int SerialPort::set_mode(uint32_t baudrate, int databits, int stopbits, Parity parity, int flowctl, int timeout) {
    struct termios options;
    speed_t brcodes[] = {B1200, B1800, B2400, B4800, B9600, B57600, B115200};
    uint32_t brates[] = {1200, 1800, 2400, 4800, 9600, 57600, 115200};
    uint br;

    if (this->fd == 0) return -1;

    for (br = 0; br < sizeof(brates); br++)
    	if (brates[br] == baudrate)
    		break;

    if (br == sizeof(brates)) return -1;
    if (databits != 7 and databits != 8) return -1;
    if (stopbits != 1 and stopbits != 2) return -1;

    if (tcgetattr(this->fd, &options) == -1) return -1;

    cfsetispeed(&options, brcodes[br]);
    cfsetospeed(&options, brcodes[br]);

    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~CSIZE;
    options.c_cflag |= databits == 7 ? CS7 : CS8;

    if (stopbits == 1)
    	options.c_cflag &= ~CSTOPB;
    else
    	options.c_cflag |= CSTOPB;

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= parity;

    if (flowctl)
        options.c_cflag |= CRTSCTS;
    else
    	options.c_cflag &= ~CRTSCTS;

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = timeout;

    options.c_lflag = 0;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) == -1) return -1;

    return 0;
}

int SerialPort::write_bytes(char *buf, size_t size) {
    if (this->fd == 0) return -1;
//    LOG_DEBUG("SERIAL", "<< %s", buf);
    size_t res = write(this->fd, buf, size);
    fsync(this->fd);
    return res;
}

int SerialPort::write_string(string buf) {
	return this->write_bytes((char *)buf.c_str(), buf.length());
}

int SerialPort::read_bytes(char *buf, size_t size) {
    if (this->fd == 0) return -1;
	return read(this->fd, buf, size);
}

string SerialPort::read_string(char delimiter, char start_with) {
	char c;
	string buf = "";

	if (start_with) {
		do
			if (this->read_bytes(&c, 1) != 1) return buf;
		while (c != start_with);
		buf += c;
	}

	while (c != delimiter) {
		if (this->read_bytes(&c, 1) != 1) break;
		buf += c;
	}

//    if (buf.size() > 0) LOG_DEBUG("SERIAL", ">> %s", buf.c_str());

    return buf;
}




