/**
* @file wave_read.h
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief Read wave file.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#ifndef __WAVE_READ_H__
#define	__WAVE_READ_H__

#include <iostream>
#include <fstream>

typedef struct _wave_header
{
	char		group_id[4];
	int			file_size;
	char		wave[4];
	char		subchunk1_id[4];
	int			subchunk1_size;
	short int	audio_format;
	short int	num_channels;
	int			sample_rate;
	int			bitrate;
	short int	block_align;
	short int	bits_per_sample;
	char		chunk2_id[4];
	int			chunk2_size;
} wave_header;

class wave_read
{
private:
	wave_header			*m_ps_wave_header;				//!< Wave header
	FILE				*m_f_wave_file;					//!< Wave file 
	char				*m_pc_file_name;				//!< Wave file name
	int					m_i_sanity_pass;				//!< 1- Sane wave file, 0- otherwise
	unsigned char		*m_pc_header_buffer;			//!< Wave header buffer
	int					m_i_bytes_per_sample;			//!< Number of bytes per sample
	int					m_i_total_samples;				//!< Total number of data samples
	int					m_i_buff_size_in_bytes;			//!< Size of the internal buffer in bytes
	unsigned char		*m_pc_buffer;					//!< Buffer to hold sample data

	/**
	*	Fill wav header.
	*/
	void		fill_wave_header();
public:

	/**
	*	Constructor.
	*/
	wave_read();

	/**
	*	Wave read constructor.
	*	Get the wav header informaiton.
	*	@param pc_wave_file Name of the wav file.
	*/
	wave_read(char *pc_wave_file);
	
	/**
	*	Wave read destructor.
	*/
	~wave_read();

	/**
	*	Display Wave info.
	*/
	void	display_wave_info();

	/**
	*	Get wav header.
	*	@return wave_header structure.
	*/
	wave_header	*get_wave_header(){return m_ps_wave_header;}

	/**
	*	Fill Wave buffer.
	*	@param ppi_pcm_buffer Buffer to fill with PCM data from reading the wav file. User allocates
	*	this buffer as int and it is filled by this function. First array is the left and the second is
	*	the right channel.
	*	@param i_samples Number of samples to read.
	*	@return The number of bytes actually read from the file. 
	*/
	int		fill_wave_buffer(int **ppi_pcm_buffer, int i_samples);	

	/**
	*	Fill Wave buffer.
	*	@param ppv_pcm_buffer Buffer to fill with PCM data from reading the wav file. User allocates
	*	this buffer as short and it is filled by this function. First array is the left and the second is
	*	the right channel.
	*	@param i_samples Number of samples to read.
	*	@return The number of bytes actually read from the file. 
	*/
	int		fill_wave_buffer(short **ppi_pcm_buffer, int i_samples);

	/**
	*	Sanity check of the current wav file.
	*	@return 0: All clear, otherwise: problem
	*/
	int		sanity_check(){return sanity_check(m_ps_wave_header);}

	/**
	*	Sanity check.
	*	ps_wave_header wave_header structure.
	*	@return 0: All clear, otherwise: problem
	*/
	int		sanity_check(wave_header *ps_wave_header);

	/**
	*	Initialize.
	*	Initialize the memory et/c. of the wave read class. 
	*	@param i_buff_size_in_bytes Size of the buffer allocated for internal reading. The number of samples
	*	read at once from the wave file should be less than this value.
	*	@param pc_wave_file Name of the wave file.
	*/
	void	init(char *pc_wave_file, int i_buff_size_in_bytes);
};

#endif	// __WAVE_READ_H__