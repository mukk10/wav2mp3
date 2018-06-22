/**
* @file pthread_queue.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the pthread_queue class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <work_queue.h>
#include <work_item.h>
#include <thread_handler.h>
#include <pthread_queue.h>
#include <pthread.h>
#include <stdio.h>
#include <cstdlib>

pthread_queue::pthread_queue()
{
	m_i_num_threads = 0;
	m_i_num_jobs = 0;
	m_i_curr_job = 0;
	m_pc_work_queue = NULL;
	m_ppc_fixed_work_queue = NULL;
}

void pthread_queue::make_thread_pool(int i_num_threads, int i_num_jobs)
{
	m_b_fixed_assign = false;
	if(m_i_num_threads)	// There are alive threads, so delete them first
	{
		for(int i=0;i<m_i_num_threads;i++)
			delete m_ppc_thread_handler[i];
		delete [] m_ppc_thread_handler;

		delete m_pc_work_queue;

		if(m_ppc_fixed_work_queue)
		{
			for(int i=0;i<m_i_num_threads+1;i++)
				delete m_ppc_fixed_work_queue[i];
			delete [] m_ppc_fixed_work_queue;
		}

		for(int i=0;i<m_i_num_jobs;i++)
			delete m_ppc_work_item[i];
		delete [] m_ppc_work_item;
	}

	m_i_num_threads = i_num_threads;	// The manager will do one of the jobs
	m_i_num_jobs = i_num_jobs;
	m_i_curr_job = 0;

	m_pc_work_queue = new work_queue(m_i_num_jobs);

	m_ppc_thread_handler = new thread_handler*[m_i_num_threads];
	for(int i=0;i<m_i_num_threads;i++)
		m_ppc_thread_handler[i] = new thread_handler(i,m_pc_work_queue);
	
	// Start the threads
	for(int i=0;i<m_i_num_threads;i++)
		m_ppc_thread_handler[i]->start_thread();

	m_ppc_work_item = new work_item*[m_i_num_jobs];
	for(int i=0;i<m_i_num_jobs;i++)
		m_ppc_work_item[i] = new work_item();
}

void pthread_queue::make_thread_pool_fixed(int i_num_threads,int i_num_jobs)
{
	m_b_fixed_assign = true;
	if(m_i_num_threads)	// There are alive threads, so delete them first
	{
		for(int i=0;i<m_i_num_threads;i++)
			delete m_ppc_thread_handler[i];
		delete[] m_ppc_thread_handler;

		if(m_pc_work_queue)
			delete m_pc_work_queue;

		for(int i=0;i<m_i_num_threads;i++)
			delete m_ppc_fixed_work_queue[i];
		delete [] m_ppc_fixed_work_queue;

		for(int i=0;i<m_i_num_jobs;i++)
			delete m_ppc_work_item[i];
		delete[] m_ppc_work_item;
	}

	m_i_num_threads = i_num_threads;
	m_i_num_jobs = i_num_jobs;
	m_i_curr_job = 0;

	// Every thread will have its own personal job queue.
	m_ppc_fixed_work_queue = new work_queue*[m_i_num_threads];
	for(int i=0;i<m_i_num_threads;i++)
		m_ppc_fixed_work_queue[i] = new work_queue(1+m_i_num_jobs/m_i_num_threads);	// A thread can be assigned multiple jobs

	m_ppc_thread_handler = new thread_handler*[m_i_num_threads];
	for(int i=0;i<m_i_num_threads;i++)
		m_ppc_thread_handler[i] = new thread_handler(i,m_ppc_fixed_work_queue[i]);	// The last queue is not assigned to any thread

	// Start the threads
	for(int i=0;i<m_i_num_threads;i++)
		m_ppc_thread_handler[i]->start_thread();

	m_ppc_work_item = new work_item*[m_i_num_jobs];
	for(int i=0;i<m_i_num_jobs;i++)
		m_ppc_work_item[i] = new work_item();
}

void pthread_queue::register_function(int i_thread_num, void *(*p_func_ptr)(void *, int))
{
	if(i_thread_num >= m_i_num_threads)
	{
		fprintf(stderr, "File %s Line %d: ERROR Thread number is more than available threads.\n", 
			__FILE__,  __LINE__);
		exit(1);
	}
	m_ppc_thread_handler[i_thread_num]->set_default_function(p_func_ptr);
}

int pthread_queue::add_to_job_queue(void *(*p_func_ptr)(void *, int), void *p_args)
{
	// Check if queue full, if yes, then wait
	if(m_i_curr_job == m_i_num_jobs)
	{
		if(m_b_fixed_assign)
		{
			for(int i=0;i<m_i_num_threads;i++)
				m_ppc_fixed_work_queue[i]->wait_for_queue_empty();	// Check every independent queue
		}
		else
			m_pc_work_queue->wait_for_queue_empty(); 

		m_i_curr_job = 0;
	}

	m_ppc_work_item[m_i_curr_job]->m_i_item_num = m_i_curr_job;
	m_ppc_work_item[m_i_curr_job]->m_p_args = p_args;
	m_ppc_work_item[m_i_curr_job]->m_p_func_ptr = p_func_ptr;

	// Add the jobs to the queue
	if(m_b_fixed_assign)
		m_ppc_fixed_work_queue[m_i_curr_job%(m_i_num_threads+1)]->add_to_job(m_ppc_work_item[m_i_curr_job]);
	else
		m_pc_work_queue->add_to_job(m_ppc_work_item[m_i_curr_job]);

	m_i_curr_job++;

	return 0;
}

void pthread_queue::wait_queue_done()
{
	// Wait until all threads are done 
	if(m_b_fixed_assign)
	{
		for(int i=0;i<m_i_num_threads;i++)
			m_ppc_fixed_work_queue[i]->wait_for_queue_empty();	// Check every independent queue
	}
	else
		m_pc_work_queue->wait_for_queue_empty();

	m_i_curr_job = 0;
}

int pthread_queue::get_job_time(int i_job_num)
{
	return m_ppc_work_item[i_job_num]->m_i_thread_time;
}

int pthread_queue::get_thread_id_for_job_id(int i_job_num)
{
	return m_ppc_work_item[i_job_num]->m_i_thread_num;
}

pthread_queue::~pthread_queue()
{
	if(m_i_num_threads)	// There are alive threads, so delete them first
	{
		for(int i=0;i<m_i_num_threads;i++)
			delete m_ppc_thread_handler[i];
		delete [] m_ppc_thread_handler;

		if(m_ppc_fixed_work_queue)
		{
			for(int i=0;i<m_i_num_threads+1;i++)
				delete m_ppc_fixed_work_queue[i];
			delete [] m_ppc_fixed_work_queue;
		}

		for(int i=0;i<m_i_num_jobs;i++)
			delete m_ppc_work_item[i];
		delete [] m_ppc_work_item;

		if(m_pc_work_queue)
			delete m_pc_work_queue;
	}
}
