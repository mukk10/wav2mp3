/**
* @file work_queue.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the work_queue class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <work_queue.h>
#include <work_item.h>
#include <stdio.h>
#include <cstdlib>

work_queue::work_queue(int i_queue_size)
{
	m_i_queue_size = i_queue_size;
	m_i_curr_queue_size = 0;
	m_i_curr_w_loc = 0;
	m_i_curr_rd_loc = 0;
	m_i_pending_jobs = 0;
	m_ppc_work_item_queue = new work_item*[i_queue_size];

	pthread_mutex_init(&m_t_mutex, NULL);
	pthread_cond_init(&m_t_job_avail_cond, NULL);
	pthread_cond_init(&m_t_queue_empty_cond, NULL);
}

work_queue::~work_queue()
{
	delete [] m_ppc_work_item_queue;

	pthread_mutex_destroy(&m_t_mutex);
	pthread_cond_destroy(&m_t_job_avail_cond);
	pthread_cond_destroy(&m_t_queue_empty_cond);
}

int work_queue::add_to_job(work_item *pc_work_item)
{
	pthread_mutex_lock(&m_t_mutex);
	if(m_i_curr_queue_size == m_i_queue_size)	// No space in the queue
	{
		pthread_mutex_unlock(&m_t_mutex);
		return 1;
	}
	else	// Space in the queue available
	{
		m_ppc_work_item_queue[m_i_curr_w_loc] = pc_work_item;
		m_i_curr_w_loc = (m_i_curr_w_loc+1) % m_i_queue_size;	// Circular buffer
		m_i_curr_queue_size++;
		pthread_cond_signal(&m_t_job_avail_cond);
		pthread_mutex_unlock(&m_t_mutex);
		return 0;
	}
}

work_item *work_queue::get_next_job()
{
	pthread_mutex_lock(&m_t_mutex);
	work_item *pc_work_item = NULL;
	while(m_i_curr_queue_size == 0)	// Wait until a job gets available, otherwise, directly process the job
		pthread_cond_wait(&m_t_job_avail_cond, &m_t_mutex);

	pc_work_item = m_ppc_work_item_queue[m_i_curr_rd_loc];
	m_i_curr_rd_loc = (m_i_curr_rd_loc+1) % m_i_queue_size;
	m_i_curr_queue_size--;

	pthread_mutex_unlock(&m_t_mutex);
	return pc_work_item;
}

work_item *work_queue::get_next_job_no_wait()
{
	work_item *pc_work_item = NULL;
	pthread_mutex_lock(&m_t_mutex);
	
	if(m_i_curr_queue_size > 0)	// Job available
	{
		pc_work_item = m_ppc_work_item_queue[m_i_curr_rd_loc];
		m_i_curr_rd_loc = (m_i_curr_rd_loc+1) % m_i_queue_size;
		m_i_curr_queue_size--;
	}
	
	pthread_mutex_unlock(&m_t_mutex);
	return pc_work_item;
}

int work_queue::get_num_jobs_in_queue()
{
	pthread_mutex_lock(&m_t_mutex);
	int iWrittenItems = m_i_curr_queue_size;
	pthread_mutex_unlock(&m_t_mutex);
	return iWrittenItems;
}

void work_queue::inc_num_jobs_in_process()
{
	pthread_mutex_lock(&m_t_mutex);
	m_i_pending_jobs++;
	pthread_mutex_unlock(&m_t_mutex);
}

void work_queue::set_job_done()
{
	pthread_mutex_lock(&m_t_mutex);
	m_i_pending_jobs--;
	if(m_i_pending_jobs == 0 && m_i_curr_queue_size == 0)
	{
		int iRetVal = pthread_cond_signal(&m_t_queue_empty_cond);
		if(iRetVal != 0)
		{
			fprintf(stderr, "File %s Line %d: ERROR Conditional variable not set.\n", 
					__FILE__,  __LINE__);
			exit(1);
		}
	}
	pthread_mutex_unlock(&m_t_mutex);
}

void work_queue::wait_for_queue_empty()
{
	// @todo Maybe I can insert a conditional wait statement here and
	// get rid of too much testing of this function.
	pthread_mutex_lock(&m_t_mutex);
	while(m_i_pending_jobs > 0 || m_i_curr_queue_size > 0)	// Wait for job to finish
		pthread_cond_wait(&m_t_queue_empty_cond,&m_t_mutex);	// Wait should come before signal pthread
	pthread_mutex_unlock(&m_t_mutex);
}