/**
* @file thread_handler.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the thread_handler class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <thread_handler.h>
#include <work_item.h>
#include <work_queue.h>

thread_handler::thread_handler(int i_thread_num, work_queue *pc_work_queue):
	m_i_thread_num(i_thread_num), m_pc_work_queue(pc_work_queue)
{
	m_i_status = 0;
	m_i_detached = 0;
	m_p_func_ptr = NULL;
}

thread_handler::~thread_handler()
{
	if(m_i_status == 1 && m_i_detached == 0)
		pthread_detach(m_t_id);
	//if(m_i_status == 1)
		pthread_cancel(m_t_id);
}

static void* runThread(void* arg)
{
	return ((thread_handler*)arg)->run_thread();
}

void *thread_handler::run_thread()
{
	while(1)
	{
		// Remove an item from the queue
		work_item *pc_work_item = m_pc_work_queue->get_next_job();
		m_pc_work_queue->inc_num_jobs_in_process();
		pc_work_item->m_i_thread_num = m_i_thread_num;
		if(pc_work_item->m_p_func_ptr != NULL)		// Call the provided function
			pc_work_item->m_p_func_ptr(pc_work_item->m_p_args, m_i_thread_num);
		else	// Use the default registered function
			m_p_func_ptr(pc_work_item->m_p_args, m_i_thread_num);
		m_pc_work_queue->set_job_done();
	}
	pthread_exit((void*) 0);
}

int thread_handler::start_thread()
{
	int result = pthread_create(&m_t_id, NULL, runThread, this);
	if (result == 0)
		m_i_status = 1;

	return result;
}

int thread_handler::wait_till_thread_finished()
{
	int i_ret = -1;
	if(m_i_status == 1)	// Still running
	{
		i_ret = pthread_join(m_t_id, NULL);	// Wait for the thread to finish
		if(i_ret == 0)
			m_i_detached = 1;
	}
	return i_ret;
}

int thread_handler::detach_thread()
{
	int i_ret = -1;
	if (m_i_status == 1 && m_i_detached == 0) 
	{
		i_ret = pthread_detach(m_t_id);
		if (i_ret == 0) 
			m_i_detached = 1;
	}
	return i_ret;
}