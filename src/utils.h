/*
 * utils.h
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
using namespace std;

void rtrim(char *);
void ltrim(char *);
void trim(char *);
void upcase(char *str);
void downcase(char *str);
bool starts_with(string str, string prefix);

string itos(int);
unsigned long stoul(string str, unsigned long deflt = 0);
unsigned int stou(string str, unsigned int deflt = 0);
float stof(string str, float deflt = 0);
double stod(string str, double deflt = 0);
bool stob(string str, bool deflt = false);

string get_csv_field(string str, int idx);

string hex_decode(string hex);

#endif /* UTILS_H_ */
