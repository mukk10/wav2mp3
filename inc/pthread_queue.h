/**
* @file pthread_queue.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the pthread_queue class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __PTHREAD_QUEUE_H__
#define __PTHREAD_QUEUE_H__

class thread_handler;
class work_queue;
class work_item;

/**
*	Pthread queue class.
*	Encapsulates all the functionality and exposes the interfaces.
*/
class pthread_queue
{
private:
	bool				m_b_fixed_assign;				//!< Denotes if the jobs are assigned to the same cores	
	int					m_i_num_threads;				//!< Number of threads
	work_queue			*m_pc_work_queue;				//!< Job queue
	work_queue			**m_ppc_fixed_work_queue;		//!< Job queue for fixed job to thread assignment
	thread_handler		**m_ppc_thread_handler;			//!< Threads to handle the job
	work_item			**m_ppc_work_item;				//!< Job descriptors 
	int					m_i_curr_job;					//!< Current job number
	int					m_i_num_jobs;					//!< Total number of jobs

public:

	/**
	*	Constructor.
	*/
	pthread_queue();

	/**
	*	Destructor.
	*/
	~pthread_queue();

	/**
	*	Make thread pool.
	*	A job can be fetched by any thread.
	*	@param i_num_threads The number of threads. Should be at least 2.
	*	@param i_num_jobs The number of jobs these threads will do.
	*/
	void				make_thread_pool(int i_num_threads, int i_num_jobs);

	/**
	*	Make thread pool.
	*	The jobs will always be assigned to the same threads, i.e. first job always goes to
	*	the first thread etc. This will be used when m_b_fixed_assign is 1.
	*	@param i_num_threads The number of threads. Should be at least 2.
	*	@param i_num_jobs The number of jobs these threads will do.
	*/
	void				make_thread_pool_fixed(int i_num_threads,int i_num_jobs);

	/**
	*	Add job to process.
	*	A return value of 1 denotes there is no space in the queue. A 0 denotes that the job was added.
	*	Preferably, the last job added to the queue should be the biggest job.
	*	@param p_func_ptr The pointer to the static callback function which returns void * and takes void * as argument
	*	@param p_args The input arguments to the function p_func_ptr
	*/
	int					add_to_job_queue(void *(*p_func_ptr)(void *, int), void *p_args);

	/**
	*	Wait for queue done.
	*/
	void				wait_queue_done();

	/**
	*	Get time for a job.
	*	The output is time in msec.
	*/
	int					get_job_time(int i_job_num);

	/**
	*	Thread processing the job.
	*	Get the thread that processed the job.
	*/
	int					get_thread_id_for_job_id(int i_job_num);

	/**
	*	Register function.
	*	Register the function with a thread such that whenever data is available in the queue, 
	*	the following function is called. 
	*	@param i_thread_num The thread number with which to associate the function.
	*	@param p_func_ptr The function to regsiter with the thread. However, note that the
	*	function pushed into the job queue will have a higher priority. Therefore, to use
	*	this function, push NULL function to the job queue. 
	*/
	void				register_function(int i_thread_num, void *(*p_func_ptr)(void *, int));
};

#endif // __PTHREAD_QUEUE_H__