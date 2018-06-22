/**
* @file thread_handler.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the thread_handler class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __THREAD_HANDLER_H__
#define __THREAD_HANDLER_H__

#include <pthread.h>

class work_queue;

/**
*	Thread handler.
*	Class to handle thread execution.
*/
class thread_handler
{
private:
	void		*(*m_p_func_ptr)(void *p, int t);	//!< The default function the thread calls when it pops a job from the queue
	int			m_i_thread_num;						//!< Thread number of the thread running this class
	work_queue	*m_pc_work_queue;					//!< Working queue with jobs
	int			m_i_status;							//!< 1 -> Running, 0 -> Idle
	int			m_i_detached;						//!< 1 -> Detached, 0 -> Not detached (default)
	pthread_t	m_t_id;								//!< Thread ID

public:

	/**
	*	Constructor.
	*	@param i_thread_num The ID of the thread.
	*	@param pc_work_queue The work queue class from which to fetch the job.
	*/
	thread_handler(int i_thread_num, work_queue *pc_work_queue);
	
	/**
	*	Destructor.
	*/
	~thread_handler();
	
	/**
	*	Run the thread.
	*	Actual runing function of the thread
	*/
	void		*run_thread();

	/**
	*	Start the thread.
	*	It will wait for the jobs and then start processing
	*/
	int			start_thread();

	/**
	*	Wait for thread to finish.
	*	This is actually pthread's join() function
	*/
	int			wait_till_thread_finished();

	/**
	*	Detach the thread
	*/
	int			detach_thread();

	/**
	*	Register default function. 
	*/
	void		set_default_function(void *(*p_func_ptr)(void *, int)){m_p_func_ptr = p_func_ptr;}

};

#endif // __THREAD_HANDLER_H__