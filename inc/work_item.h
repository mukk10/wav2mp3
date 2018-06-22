/**
* @file work_item.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief This file contains the work_item class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __WORK_ITEM_H__
#define __WORK_ITEM_H__

/**
*	Work item.
*	Stores a workitem in the jobs queue. Filled by the caller and poped by a thread.
*/
class work_item
{
public:
	void	*(*m_p_func_ptr)(void *p, int t);		//!< The function which must be called when the thread is run
	int		m_i_item_num;							//!< Current item number
	void	*m_p_args;								//!< Arguments array
	int		m_i_tot_args;							//!< Total arguments
	int		m_i_thread_time;						//!< Time consumed in msec by the work item to be processed
	int		m_i_mhz;								//!< Frequency at which the thread should be executed
	int		m_i_thread_num;							//!< Thread processing the current work item

	/**
	*	Constructor.
	*/
	work_item(){}

	/**
	*	Constructor.
	*	@param p_func_ptr Pointer to the function called by the thread
	*	@param i_item_num The item number/ID
	*	@param p_args A structure to hold the arguments, passed to p_func_ptr
	*	@param i_tot_args Total number of arguments.	*	
	*/
	work_item(void * (*p_func_ptr)(void*, int), int i_item_num, void *p_args, int i_tot_args):
		m_p_func_ptr(p_func_ptr), m_i_item_num(i_item_num), m_p_args(p_args), m_i_tot_args(i_tot_args){}

	/**
	*	Destructor.
	*/
	~work_item(){}
};

#endif	// __WORK_ITEM_H__