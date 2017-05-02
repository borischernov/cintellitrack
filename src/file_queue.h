/*
 * file_queue.h
 *
 *  Created on: 24/10/2011
 *      Author: boris
 */

#ifndef FILE_QUEUE_H_
#define FILE_QUEUE_H_

#include "message_queue.h"

class FileQueue : public MessageQueue {
public:
	FileQueue() {};
    bool put_message(string msg);
    string get_message(void);
    void remove_message(void);
    size_t size(void);
protected:
	char *file;
};

#endif /* FILE_QUEUE_H_ */
