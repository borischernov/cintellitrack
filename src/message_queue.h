/**********************************************************
* 
* Message Queue
*
*/

#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include "config_file.h"

class MessageQueue {
public:
	virtual bool put_message(string msg) = 0;
	virtual string get_message(void) = 0;
	virtual void remove_message(void) = 0;
	virtual size_t size(void) = 0;
	virtual size_t length(void) = 0;
	virtual unsigned int age(void) = 0;
};

#endif
