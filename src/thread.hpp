#ifndef THREAD_HPP_INCLUDED
#define THREAD_HPP_INCLUDED

#include "SDL.h"
#include "SDL_thread.h"

// Threading primitives wrapper for SDL_Thread.
//
// This module defines primitives for wrapping C++ around SDL's threading
// interface
namespace threading
{

// Threading object.
//
// This class defines threading objects. One such object represents a 
// thread and admits killing and joining on threads. Intended to be
// used for manipulating threads instead of poking around with SDL_Thread
// calls.
class thread
{
public:
	// Construct a new thread to start executing the function 
     	// pointed to by f. The void* data will be passed to f, to
   	// facilitate passing of parameters to f.
	//
	// \param f the function at which the thread should start executing
	// \param data passed to f
	// 
	// \pre f != NULL
	explicit thread(int (*f)(void*), void* data=NULL);

	// Destroy the thread object. This is done by waiting on the
	// thread with the join() operation, thus blocking until the
	// thread object has finished its operation.
	~thread();

	// Kill the thread. If the thread has already been killed, this
	// is a no-op.
	void kill();

	// Join (wait) on the thread to finish. When the thread finishes,
	// the function will return. calling wait() on an already killed
	// thread is a no-op.
	void join();
private:
	thread(const thread&);
	void operator=(const thread&);

	SDL_Thread* thread_;
};

// Binary mutexes.
//
// Implements an interface to binary mutexes. This class only defines the
// mutex itself. Locking is handled through the friend class lock,
// and monitor interfacing through condition variables is handled through
// the friend class condition.
class mutex
{
public:
	mutex();
	~mutex();

	friend class lock;
	friend class condition;

private:
	mutex(const mutex&);
	void operator=(const mutex&);

	SDL_mutex* const m_;
};

// Binary mutex locking.
//
// Implements a locking object for mutexes. The creation of a lock
// object on a mutex will lock the mutex as long as this object is
// not deleted.
class lock
{
public:
	// Create a lock object on the mutex given as a parameter to
	// the constructor. The lock will be held for the duration
	// of the object existence. 
	// If the mutex is already locked, the constructor will 
	// block until the mutex lock can be acquired. 
	//
	// \param m the mutex on which we should try to lock.
	explicit lock(mutex& m);
	// Delete the lock object, thus releasing the lock aquired
	// on the mutex which the lock object was created with.
	~lock();
private:
	lock(const lock&);
	void operator=(const lock&);

	mutex& m_;
};

// Condition variable locking.
//
// Implements condition variables for mutexes. A condition variable
// allows you to free up a lock inside a critical section
// of the code and regain it later. Condition classes only make
// sense to do operations on, if one already aquired a mutex.
class condition
{
public:
	condition();
	~condition();

	// Wait on the condition. When the condition is met, you
	// have a lock on the mutex and can do work in the critical
	// section. When the condition is not met, wait blocks until
	// the condition is met and atomically frees up the lock on
	// the mutex. One will automatically regain the lock when the
	// thread unblocks. 
	//
	// If wait returns false we have an error. In this case one cannot
	// assume that he has a lock on the mutex anymore. 
	// 
	// \param m the mutex you wish to free the lock for
	// \returns true: the wait was successful, false: an error occurred
	//
	// \pre You have already aquired a lock on mutex m
	// 
	bool wait(const mutex& m);
	// wait on the condition with a timeout. Basically the same as the
	// wait() function, but if the lock is not aquired before the 
	// timeout, the function returns with an error.
	//
	// \param m the mutex you wish free the lock for. 
	// \param timeout the allowed timeout in milliseconds (ms)
	// \returns true: success, false: an error occurred
	//
	// \todo This function cannot check for timeout, which is to
	// check on the SDL constant SDL_MUTEX_TIMEDOUT. Thus this 
	// function cannot check if the error was due to something malformed
	// or if the time ran out.
	bool wait_timeout(const mutex& m, unsigned int timeout);
	// signal the condition and wake up one thread waiting on the 
	// condition. If no thread is waiting, notify_one() is a no-op.
	// Does not unlock the mutex.
	//
	// \todo SDL_CondSignal can return an error. This is never checked
	void notify_one();

	// signal all threads waiting on the condition and let them contend
	// for the lock. This is often used when varying resource amounts are
	// involved and you do not know how many processes might continue.
	// The function should be used with care, especially if many threads are
	// waiting on the condition variable.
	//	
	// \todo SDL_CondBroadcast can return an error. This is never checked
	void notify_all();

private:
	condition(const condition&);
	void operator=(const condition&);

	SDL_cond* const cond_;
};

}

#endif
