//				Package : threadtests
// prodcons.cc			Created : 6/95 tjr
//
// Copyright (C) AT&T Laboratories Cambridge 1995
//
// Demonstrates the use of condition variables for signalling between
// two "producer" threads and three "consumer" threads.  Also demonstrates
// the use of timed waits.
//

#include <omniORB4/CORBA_sysdep.h> // for HAVE_STD

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif
#include <stdlib.h>
#include <omnithread.h>

static void producer(void*);
static void consumer(void*);

omni_mutex     m;
omni_condition full_cond(&m);
omni_condition empty_cond(&m);
int            empty_flag = 1;
const char*    message;

static const char* msgs[] = { "wibble", "wobble", "jelly", "plate" };


int main(int argc, char** argv)
{
    cout << "main: creating producer1\n";

    omni_thread::create(producer,(void*)"producer1");

    cout << "main: creating producer2\n";

    omni_thread::create(producer,(void*)"producer2");

    cout << "main: creating consumer1\n";

    omni_thread::create(consumer,(void*)"consumer1");

    cout << "main: creating consumer2\n";

    omni_thread::create(consumer,(void*)"consumer2");

    cout << "main: creating consumer3\n";

    consumer((void*)"consumer3");

    return 0;
}

static int random_l()
{
    static omni_mutex rand_mutex;
    rand_mutex.lock();
    int i = rand();
    rand_mutex.unlock();
    return i;
}

static void consumer(void* arg)
{
    char *name = (char *)arg;
    unsigned long s, n;

    while (1) {
	m.lock();

	omni_thread::get_time(&s,&n,0,500000000); // 1/2 second from now

	while (empty_flag) {
	    cout << name << ": waiting for message\n";

	    if (!full_cond.timedwait(s,n)) {
		cout << name << ": timed out, trying again\n";
		omni_thread::get_time(&s,&n,0,500000000);
	    }
            else if (empty_flag) {
		cout << name << ": woken but message already comsumed\n";
	    }
	}

	cout << name << ": got message: '" << message << "'\n";

	empty_flag = 1;

	empty_cond.signal();

	m.unlock();

	omni_thread::sleep(random_l() % 2, 1000000 * (random_l() % 1000));
    }
}

static void producer(void* arg)
{
    char *name = (char *)arg;

    while (1) {
	m.lock();

	while (!empty_flag) {
	    cout << name << ": having to wait for consumer\n";
	    empty_cond.wait();
	}

	message = msgs[random_l() % 4];
	empty_flag = 0;

	full_cond.signal();

	cout << name << ": put message: '" << message << "'\n";

	m.unlock();

	omni_thread::sleep(random_l() % 2, 1000000 * (random_l() % 500) + 500);
    }
}
