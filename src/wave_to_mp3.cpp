/**
* @file wave_to_mp3.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief Convert wave file to mp3.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <wave_to_mp3.h>
#include <wave_read.h>
#include <lame.h>
#include <cstdlib>

#define		BUFF_SIZE_BYTES		8192	//!< Change this by testing

using namespace std;

wave_to_mp3::wave_to_mp3()
{
	m_f_mp3_file = NULL;
	m_pc_wave_read = NULL;
	m_ppi_pcm_buffer = NULL;
	m_i_vbr_quality = 0;
	m_pc_wave_read = new wave_read();
}

void wave_to_mp3::init(char *pc_wave_file, char *pc_mp3_file)
{
	if(m_f_mp3_file) fclose(m_f_mp3_file);

	if(!(m_f_mp3_file = fopen(pc_mp3_file, "wb")))
	{
		fprintf(stderr, "File %s Line %d: ERROR Cannot open mp3 file to write.", __FILE__, __LINE__);
		exit(1);
	}
	
	m_pc_wave_read->init(pc_wave_file, BUFF_SIZE_BYTES);

	m_iBytesPerSample = m_pc_wave_read->get_wave_header()->bits_per_sample/8;

	m_i_samples_per_itr = BUFF_SIZE_BYTES/(m_pc_wave_read->get_wave_header()->num_channels * 
		m_pc_wave_read->get_wave_header()->bits_per_sample/8);

	// @todo For performance, shouldn't be allocating memory all the time. Should do it at one time 
	// when the object is created.
	allocate_memory();
	
	return;
}

void wave_to_mp3::free_memory()
{
	if(m_ppi_pcm_buffer && m_ppi_pcm_buffer[0]) delete [] m_ppi_pcm_buffer[0];
	if(m_ppi_pcm_buffer && m_ppi_pcm_buffer[1]) delete [] m_ppi_pcm_buffer[1];
	if(m_ppi_pcm_buffer) delete [] m_ppi_pcm_buffer;
}

void wave_to_mp3::allocate_memory()
{
	free_memory();
	
	m_ppi_pcm_buffer = new int*[2];
	m_ppi_pcm_buffer[0] = new int[m_i_samples_per_itr];

	if(m_pc_wave_read->get_wave_header()->num_channels == 2)
		m_ppi_pcm_buffer[1] = new int[m_i_samples_per_itr];
	else
		m_ppi_pcm_buffer[1] = NULL;

}

int	wave_to_mp3::sanity_check()
{
	return m_pc_wave_read->sanity_check();
}

void wave_to_mp3::display_wave_info()
{
	m_pc_wave_read->display_wave_info();
}

void wave_to_mp3::encode_wave()
{
	int i_read_bytes;
	int i_write_bytes;

	lame_t lame = lame_init();
	lame_set_in_samplerate(lame, m_pc_wave_read->get_wave_header()->sample_rate);
	lame_set_VBR(lame, vbr_default);
	lame_set_VBR_q(lame, m_i_vbr_quality);
	lame_init_params(lame);
	unsigned char *pc_mp3_buffer = new unsigned char[BUFF_SIZE_BYTES];

	do
	{
		// Get PCM 
		switch(m_iBytesPerSample)
		{
		case sizeof(short):
			i_read_bytes = m_pc_wave_read->fill_wave_buffer((short **)m_ppi_pcm_buffer, m_i_samples_per_itr); 
			break;
		case sizeof(int):
			i_read_bytes = m_pc_wave_read->fill_wave_buffer((int **)m_ppi_pcm_buffer, m_i_samples_per_itr); 
			break;
		default:
			fprintf(stderr, "File %s Line %d: ERROR Unhandled bytes per sample.\n", 
			__FILE__,  __LINE__);
			exit(1);
		}
		
		// Compress PCM
		if(i_read_bytes == 0)
			i_write_bytes = lame_encode_flush(lame, pc_mp3_buffer, BUFF_SIZE_BYTES);
		else
		{
			switch(m_iBytesPerSample)
			{
			case sizeof(short):
				i_write_bytes = lame_encode_buffer(lame, (short *)m_ppi_pcm_buffer[0], (short *)m_ppi_pcm_buffer[1], 
					m_i_samples_per_itr, pc_mp3_buffer, BUFF_SIZE_BYTES);
				break;
			case sizeof(int):
				i_write_bytes = lame_encode_buffer_int(lame, (int *)m_ppi_pcm_buffer[0], (int *)m_ppi_pcm_buffer[1], 
					m_i_samples_per_itr, pc_mp3_buffer, BUFF_SIZE_BYTES);
				break;
			default:
				fprintf(stderr, "File %s Line %d: ERROR Unhandled bytes per sample.\n", 
					__FILE__,  __LINE__);
				exit(1);
			}
		}

		// Write to file
		fwrite(pc_mp3_buffer, sizeof(unsigned char), i_write_bytes, m_f_mp3_file);
	}while(i_read_bytes);
}

wave_to_mp3::~wave_to_mp3()
{
	free_memory();
	delete m_pc_wave_read;

	if(m_f_mp3_file) fclose(m_f_mp3_file);
}
