/**
* @file work_queue.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the work_queue class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __WORK_QUEUE_H__
#define __WORK_QUEUE_H__

#include <pthread.h>

class work_item;

/**
*	Work queue.
*	A work queue is for all the threads/jobs. Each threads pops a job from this queue.
*/
class work_queue
{
private:
	work_item			**m_ppc_work_item_queue;	// A queue for work items
	int					m_i_queue_size;				// Size of the queue
	int					m_i_curr_queue_size;		// Current size of the queue
	int					m_i_curr_rd_loc;			// Read location in the work queue
	int					m_i_curr_w_loc;				// Write location in the work queue
	int					m_i_pending_jobs;			// Total Pending jobs
	pthread_mutex_t		m_t_mutex;					// Mutex for inserting and removing the job
	pthread_cond_t		m_t_job_avail_cond;			// Condition variable if a job is available in the queue
	pthread_cond_t		m_t_queue_empty_cond;		// Condition variable if the job queue is empty
public:

	/**
	*	Constructor.
	*/
	work_queue(int i_queue_size = 1);

	/**
	*	Destructor.
	*/
	~work_queue();

	/**
	*	Add to the back of the job queue.
	*	Before writing, always check if there is space in the queue
	*	0 means successful and otherwise is unsuccessful.
	*/
	int					add_to_job(work_item *pc_work_item);

	/**
	*	Extract from the front of the job queue.
	*	If no job is available, the function will be suspended in wait state
	*	I.e. this is a blocking function.
	*/
	work_item			*get_next_job();

	/**
	*	Extract from the front of the job queue.
	*	If no job is available, the function will return a NULL pointer, without being
	*	suspended in the wait state.
	*/
	work_item			*get_next_job_no_wait();

	/**
	*	Get current work items written.
	*/
	int					get_num_jobs_in_queue();

	/**
	*	Increment job pending signal for the job.
	*	Should be set by the working thread.
	*/	
	void				inc_num_jobs_in_process();

	/**
	*	Job done signal.
	*	Should be called by the working thread.
	*/
	void				set_job_done();

	/**
	*	Wait until the job queue is empty.
	*	It is better to sleep a little before calling this function.
	*/
	void				wait_for_queue_empty();
};

#endif // __WORK_QUEUE_H__