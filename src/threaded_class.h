/*
 * threaded_class.h
 *
 * Generic class with thread and mutex implementation
 * based on http://blog.emptycrate.com/node/270
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#ifndef THREADED_CLASS_H_
#define THREADED_CLASS_H_

#include <pthread.h>
#include <stdio.h>

class threaded_class
{
protected:
    volatile bool m_stoprequested;
    volatile bool m_running;
    pthread_t m_thread;

    // This is the static class function that serves as a C style function pointer
    // for the pthread_create call
    static void *launch_thread(void *obj)
    {
    	((threaded_class *)(obj))->thread_body();
        return NULL;
    }

public:
    threaded_class()
        : m_stoprequested(false), m_running(false) { }

    virtual void start_thread(void)
    {
        if (m_running) return;
        m_running = true;
        m_stoprequested = false;
        pthread_create(&m_thread, 0, &threaded_class::launch_thread, this);
    }

    void stop_thread()
    {
        if (!m_running) return;
        m_running = false;
        m_stoprequested = true;
        pthread_join(m_thread, NULL);
    }

    virtual void thread_body(void) = 0;
};


#endif /* THREADED_CLASS_H_ */
