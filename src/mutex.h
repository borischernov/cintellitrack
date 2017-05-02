/*
 * mutex.h
 *
 *  Created on: 09/10/2011
 *      Author: boris
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

class Mutex {
private:
    pthread_mutex_t m_mutex;
public:
    Mutex() { pthread_mutex_init(&m_mutex, NULL); }
    ~Mutex() { pthread_mutex_destroy(&m_mutex); }
    void lock(void) { pthread_mutex_lock(&m_mutex); }
    void unlock(void) {	pthread_mutex_unlock(&m_mutex); }
};

#endif /* MUTEX_H_ */
