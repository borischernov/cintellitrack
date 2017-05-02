/*
 * Utility functions
 *
 * utils.cpp
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string>
using namespace std;

void rtrim(char *str) {
	for(size_t c = strlen(str) - 1; c >= 0 && strchr(" \r\n\t", str[c]); c--)
		str[c] = 0;
}

void ltrim(char *str) {
	size_t c;
	for(c = 0; c < strlen(str) && strchr(" \r\n\t", str[c]); c++);
	if (c) memmove(str, str + c, strlen(str) - c + 1);
}

void trim(char *str) {
	rtrim(str);
	ltrim(str);
}

void upcase(char *str) {
	for (size_t c = 0; c < strlen(str); c++)
		str[c] = toupper(str[c]);
}

void downcase(char *str) {
	for (size_t c = 0; c < strlen(str); c++)
		str[c] = tolower(str[c]);
}

string itos(int num) {
	char buf[64];
	sprintf(buf, "%d", num);
	return string(buf);
}

unsigned long stoul(string str, unsigned long deflt) {
	sscanf(str.c_str(), "%lu", &deflt);
	return deflt;
}

unsigned int stou(string str, unsigned int deflt) {
	sscanf(str.c_str(), "%u", &deflt);
	return deflt;
}

double stod(string str, double deflt) {
	sscanf(str.c_str(), "%lf", &deflt);
	return deflt;
}

double stof(string str, float deflt) {
	sscanf(str.c_str(), "%f", &deflt);
	return deflt;
}

bool stob(string str, bool deflt) {
	char *c = (char *)str.c_str();
	if (!*c) return deflt;
	downcase(c);
	return c[0] == 'y' || c[0] == '1' || !strcmp(c, "true");
}

string get_csv_field(string str, int idx) {
	size_t start = 0, end;

	// beginning of the field
	for (;idx > 0; idx--)
		if ((start = str.find(',', start + 1)) == string::npos) return string("");

	// end of the field
	if ((end = str.find(',', start + 1)) == string::npos) end = str.length();

	return str.substr(start + 1, end - start - 1);
}

bool starts_with(string str, string prefix) {
	return !str.substr(0, prefix.size()).compare(prefix);
}

string hex_decode(string hex) {
	string res = "";
	char h;
	for (size_t c = 0; c < hex.length(); c+=2) {
		sscanf(hex.substr(c, 2).c_str(), "%hhx", &h);
		res += h;
	}
	return res;
}
