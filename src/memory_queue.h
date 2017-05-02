/**************************************************
* 
* Memory-based message queue
*
*/

#ifndef MEMORY_QUEUE_H_
#define MEMORY_QUEUE_H_

#include <queue>
#include <string>
#include <utility>
#include <time.h>
using namespace std;

#include "message_queue.h"

class MemoryQueue : public MessageQueue {
public:
	MemoryQueue() { len = 0; };
    bool put_message(string msg) { mq.push(msg_t(msg, time(NULL)));	len += msg.size(); return true; };
    string get_message(void) { return mq.size() > 0 ? mq.front().first : string(""); };
    void remove_message(void) { if (mq.size() > 0) { len -= mq.front().first.size(); mq.pop(); } };
    size_t size(void) { return mq.size(); };
    size_t length(void) { return len; };
    unsigned int age(void) { return mq.size() > 0 ? time(NULL) - mq.front().second : 0; };
protected:
    typedef pair<string, time_t> msg_t;
	queue<msg_t> mq;
	size_t len;
};

#endif
