/**
* @file wave_to_mp3.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief Read wave file.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __WAVE_TO_MP3_H__
#define	__WAVE_TO_MP3_H__

#include <iostream>
#include <fstream>

class wave_read;

class wave_to_mp3
{
private:
	wave_read			*m_pc_wave_read;				//!< Wave reader
	FILE				*m_f_mp3_file;					//!< MP3 file name
	int					m_iBytesPerSample;				//!< Number of bytes per sample
	int					m_iTotalSamples;				//!< Total number of data samples
	int					**m_ppi_pcm_buffer;				//!< Buffer holding PCM samples
	int					m_i_samples_per_itr;			//!< Samples to read from wave file per iteration
	int					m_i_vbr_quality;				//!< Quality of the encoding, 0: highest, 9: lowest

	/**
	*	Free internal memory.
	*/
	void	free_memory();

	/**
	*	Allocate internal memory.
	*/
	void	allocate_memory();

public:

	/**
	*	Constructor.
	*	If you use this constructor, then you must call the init function.
	*/
	wave_to_mp3();

	/**
	*	Destructor.
	*/
	~wave_to_mp3();

	/**
	*	Initialize.
	*	@param pc_wave_file Name of the input wave file.
	*	@param pc_mp3_file Name of the output mp3 file.
	*/
	void	init(char *pc_wave_file, char *pc_mp3_file);

	/**
	*	Display Wave info.
	*/
	void	display_wave_info();

	/**
	*	Sanity check.
	*	@return 0: All clear, otherwise: problem
	*/
	int		sanity_check();

	/**
	*	Encode wave file.
	*	Convert the PCM based wave file to mp3.
	*/
	void	encode_wave();

	/**
	*	Set quality.
	*	0: highest, 9: lowest.
	*/
	void	set_quality(int i_vbr_quality){m_i_vbr_quality = i_vbr_quality;}

	/**
	*	Get quality.
	*/
	int		get_quality(){return m_i_vbr_quality;}
};

#endif	// __WAVE_TO_MP3_H__