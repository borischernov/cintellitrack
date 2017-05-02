/*
 * ezdog_paltform.cpp
 *
 *  Created on: 15/04/2012
 *      Author: boris
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/rtc.h>
#include "ezdog_platform.h"
#include "log_file.h"

const char *EzdogPlatform::module = "EZDOG";

void EzdogPlatform::startup(void) {
}

void EzdogPlatform::start_gps(void) {
	gpio_set_in(GPS_PWRMON, 0);		// PWR Mon
	gpio_out(GPS_ONOFF, 0);
	gpio_out(LED_ORANGE, 0);
	for (;;) {
		LOG_DEBUG(module, "Waiting for GPS to become on");
		sleep(5);
		if (gpio_in(GPS_PWRMON)) break;
		gpio_out(LED_ORANGE, 1);
		LOG_DEBUG(module, "GPS is off, cycling power");
		gpio_out(GPS_ONOFF, 1);
		sleep(1);
		gpio_out(GPS_ONOFF, 0);
		gpio_out(LED_ORANGE, 0);
	}
	gpio_out(LED_ORANGE, 1);
	LOG_DEBUG(module, "GPS has switched on");
}

void EzdogPlatform::stop_gps(void) {
	gpio_out(GPS_ONOFF, 1);
}

void EzdogPlatform::start_gsm(void) {
	gpio_out(GSM_ONOFF, 1);
}

void EzdogPlatform::stop_gsm(void) {
	gpio_out(GSM_ONOFF, 0);
}

void EzdogPlatform::on_gps_data(bool has_fix) {
	gpio_out(LED_ORANGE, has_fix ? 0 : 1); // Orange LED
}

bool EzdogPlatform::gpio_out(int num, int val) {
	char buf[32];
	int f;

	snprintf(buf, sizeof(buf), "/dev/at91sam9260_gpio.%d", num);

	if ((f = open(buf, O_RDWR)) == -1)
		return false;
	if (write(f, val ? "O1" : "O0", 2) != 2) {
		close(f);
		return false;
	}
	if (close(f) == -1)
		return false;
	return true;
}

bool EzdogPlatform::gpio_set_in(int num, int val) {
	char buf[32];
	int f;

	snprintf(buf, sizeof(buf), "/dev/at91sam9260_gpio.%d", num);

	if ((f = open(buf, O_RDWR)) == -1)
		return false;
	if (write(f, val ? "I1" : "I0", 2) != 2) {
		close(f);
		return false;
	}
	if (close(f) == -1)
		return false;
	return true;
}

int EzdogPlatform::gpio_in(int num) {
	char buf[32];
	int f;

	snprintf(buf, sizeof(buf), "/dev/at91sam9260_gpio.%d", num);

	if ((f = open(buf, O_RDWR)) == -1)
		return -1;
	if (read(f, buf, 2) != 2) {
		close(f);
		return -1;
	}
	if (close(f) == -1)
		return -1;

	return buf[1] == '0' ? 0 : 1;
}
/*
bool EzdogPlatform::set_clock(void) {
	int rtc;
	struct rtc_time rtc_tm;
	time_t time;

	if ((rtc = open("/dev/rtc0", O_RDWR)) == -1)
		return false;
	if (ioctl(rtc, RTC_RD_TIME, &rtc_tm) == -1) {
		close(rtc);
		return false;
	}
	close(rtc);

	time = mktime((struct tm *)&rtc_tm);

	LOG_DEBUG(module, "Setting system time from RTC: %s", ctime(&time));

	return stime(&time) != -1;
}
*/

void EzdogPlatform::set_time(time_t tme) {
	if (time(NULL) > 946684800L)
		return;

	LOG_DEBUG(module, "Setting system time : %s", ctime(&tme));

	stime(&tme);
}
