/**
* @file wave_read.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief Read wav class.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <wave_read.h>
#include <cstdlib>
#include <cstring>

using namespace std;

wave_read::wave_read()
{
	m_ps_wave_header = new wave_header;
	m_pc_header_buffer = new unsigned char[sizeof(wave_header)];
	m_pc_buffer = NULL;
	m_f_wave_file = NULL;
}

void wave_read::init(char *pc_wave_file, int i_buff_size_in_bytes)
{
	m_pc_file_name = pc_wave_file;
	
	if(m_f_wave_file) fclose(m_f_wave_file);
	if(!(m_f_wave_file = fopen(pc_wave_file, "rb")))
	{
		fprintf(stderr, "File %s Line %d: ERROR Cannot open wave file %s to read.\n",
			__FILE__, __LINE__, pc_wave_file);
		exit(1);
	}
		
	// @todo Will need to fix this in case there are more chunks in the header
	if(fread(m_pc_header_buffer,sizeof(wave_header),1,m_f_wave_file) <= 0)
	{
		fprintf(stderr, "File %s Line %d: ERROR Could not read header.\n", __FILE__,  __LINE__);
		exit(1);
	}

	fill_wave_header();
	if(sanity_check())
	{
		fprintf(stderr, "File %s Line %d: WARNING Header not compliant.\n", __FILE__,  __LINE__);
	}

	m_i_bytes_per_sample = m_ps_wave_header->bits_per_sample/8;
	m_i_total_samples =  m_ps_wave_header->chunk2_size / (m_ps_wave_header->num_channels * m_ps_wave_header->bits_per_sample/8);
	
	m_i_buff_size_in_bytes = i_buff_size_in_bytes;
	if(m_pc_buffer) delete [] m_pc_buffer;
	m_pc_buffer = new unsigned char[m_i_buff_size_in_bytes];
}

void wave_read::fill_wave_header()
{
	unsigned char *pcHeaderBuffer = m_pc_header_buffer;

	unsigned char pucBuffer[5];		// Read and compare buffer
	pucBuffer[4] = '\0';

	//*********************
	// Header
	// RIFF Chunk ID
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	strncpy(m_ps_wave_header->group_id,(char *)pucBuffer,4);

	// Chunk size
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	m_ps_wave_header->file_size = pucBuffer[0] + (pucBuffer[1]<<8) + (pucBuffer[2]<<16)
		+ (pucBuffer[3]<<24);

	// WAVE
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	strncpy(m_ps_wave_header->wave,(char *)pucBuffer,4);

	//***********************
	// Format chunk
	// fmt
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	strncpy(m_ps_wave_header->subchunk1_id,(char *)pucBuffer,4);

	// Chunk size
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	m_ps_wave_header->subchunk1_size = pucBuffer[0] + (pucBuffer[1]<<8) + (pucBuffer[2]<<16)
		+ (pucBuffer[3]<<24);

	// Audio fmt
	memcpy(pucBuffer,pcHeaderBuffer,2);
	pcHeaderBuffer += 2;
	m_ps_wave_header->audio_format = pucBuffer[0] + (pucBuffer[1]<<8);

	// Number of channels
	memcpy(pucBuffer,pcHeaderBuffer,2);
	pcHeaderBuffer += 2;
	m_ps_wave_header->num_channels = pucBuffer[0] + (pucBuffer[1]<<8);

	// Sample rate
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	m_ps_wave_header->sample_rate = pucBuffer[0] + (pucBuffer[1]<<8) + (pucBuffer[2]<<16)
		+ (pucBuffer[3]<<24);

	// Byte rate = num_channels * sample_rate * bits_per_sample/8
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	m_ps_wave_header->bitrate = pucBuffer[0] + (pucBuffer[1]<<8) + (pucBuffer[2]<<16)
		+ (pucBuffer[3]<<24);

	// Block align =  num_channels * bits_per_sample/8
	memcpy(pucBuffer,pcHeaderBuffer,2);
	pcHeaderBuffer += 2;
	m_ps_wave_header->block_align = pucBuffer[0] + (pucBuffer[1]<<8);

	// Bits per sample
	memcpy(pucBuffer,pcHeaderBuffer,2);
	pcHeaderBuffer += 2;
	m_ps_wave_header->bits_per_sample = pucBuffer[0] + (pucBuffer[1]<<8);

	//***************************
	// Data chunck
	// ID
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	strncpy(m_ps_wave_header->chunk2_id,(char *)pucBuffer,4);

	// Size
	memcpy(pucBuffer,pcHeaderBuffer,4);
	pcHeaderBuffer += 4;
	m_ps_wave_header->chunk2_size = pucBuffer[0] + (pucBuffer[1]<<8) + (pucBuffer[2]<<16)
		+ (pucBuffer[3]<<24);

}

int	wave_read::sanity_check(wave_header *ps_wave_header)
{
	int iRet = 0;
	m_i_sanity_pass = 1;
	if(strncmp((char *)ps_wave_header->group_id, "RIFF", 4))
	{
		fprintf(stderr, "File %s Line %d: WARNING RIFF fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 1;
	}
	
	if(strncmp((char *)ps_wave_header->wave, "WAVE", 4))
	{
		fprintf(stderr, "File %s Line %d: WARNING WAVE fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 2;
	}

	if(strncmp((char *)ps_wave_header->subchunk1_id, "fmt ", 4))
	{
		fprintf(stderr, "File %s Line %d: WARNING fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 3;
	}

	if(ps_wave_header->subchunk1_size != 16)
	{
		fprintf(stderr, "File %s Line %d: WARNING PCM fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 4;
	}
	
	if(m_ps_wave_header->audio_format != 1)
	{
		fprintf(stderr, "File %s Line %d: WARNING PCM fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 5;
	}

	if(strncmp(m_ps_wave_header->chunk2_id, "data", 4))
	{
		fprintf(stderr, "File %s Line %d: WARNING data fmt not detected.\n", __FILE__,  __LINE__);
		iRet = 6;
	}

	if(ps_wave_header->bitrate != ((ps_wave_header->num_channels * ps_wave_header->sample_rate * 
		ps_wave_header->bits_per_sample) >> 3)) 
	{
		fprintf(stderr, "File %s Line %d: WARNING byte_rate != num_channels * sample_rate * \
			bytes_per_sample.\n", __FILE__,  __LINE__);
		iRet = 7;
	}

	if(ps_wave_header->block_align != ((ps_wave_header->num_channels * ps_wave_header->bits_per_sample) >> 3))
	{
		fprintf(stderr, "File %s Line %d: WARNING block_align != num_channels *	bytes_per_sample.\n", __FILE__,  __LINE__);
		iRet = 8;
	}
	
	if(ps_wave_header->num_channels > 2)
	{
		fprintf(stderr, "File %s Line %d: WARNING More than two channels.\n", __FILE__,  __LINE__);
		iRet = 9;
	}

	if(iRet != 0)
		m_i_sanity_pass = 0;

	return iRet;
}

void wave_read::display_wave_info()
{
	fprintf(stdout,"file:                 %s\n", m_pc_file_name);
	fprintf(stdout,"group_id:             %.4s\n", m_ps_wave_header->group_id);
	fprintf(stdout,"file_size:            %d\n", m_ps_wave_header->file_size);
	fprintf(stdout,"wave:                 %.4s\n", m_ps_wave_header->wave);
	fprintf(stdout,"subchunk1_id:         %.4s\n", m_ps_wave_header->subchunk1_id);
	fprintf(stdout,"subchunk1_size:       %d\n", m_ps_wave_header->subchunk1_size);
	fprintf(stdout,"audio_format:         %d\n", m_ps_wave_header->audio_format);
	fprintf(stdout,"num_channels:         %d\n", m_ps_wave_header->num_channels);
	fprintf(stdout,"sample_rate:          %d\n", m_ps_wave_header->sample_rate);
	fprintf(stdout,"bitrate:              %d\n", m_ps_wave_header->bitrate);
	fprintf(stdout,"block_align:          %d\n", m_ps_wave_header->block_align);
	fprintf(stdout,"bits_per_sample:      %d\n", m_ps_wave_header->bits_per_sample);
	fprintf(stdout,"chunk2_id:            %.4s\n", m_ps_wave_header->chunk2_id);
	fprintf(stdout,"chunk2_size:          %d\n", m_ps_wave_header->chunk2_size);
	fprintf(stdout,"Sanity check:         %s\n", (m_i_sanity_pass? "PASSED": "FAILED"));
	fprintf(stdout,"\n\n");
}

int	wave_read::fill_wave_buffer(short **ppi_pcm_buffer, int i_samples)
{
	if(m_pc_buffer == NULL)
	{
		fprintf(stderr, "File %s Line %d: ERROR Call init function before using fill_wave_buffer function.\n", 
			__FILE__,  __LINE__);
		exit(1);	
	}

	const int i_max_int = (1 << (m_ps_wave_header->bits_per_sample-1)) - 1;
	
	int i_bytes_to_read = m_ps_wave_header->num_channels * m_ps_wave_header->bits_per_sample/8 * i_samples;
	if(i_bytes_to_read > m_i_buff_size_in_bytes)
	{
		fprintf(stderr, "File %s Line %d: ERROR Bytes to read %d more than internal buffer size %d.\n", 
			__FILE__,  __LINE__, i_bytes_to_read, m_i_buff_size_in_bytes);
		exit(1);	
	}

	int i_read_chars = fread(m_pc_buffer, sizeof(unsigned char), i_bytes_to_read, m_f_wave_file);
	int i_read_samples = i_read_chars/(m_ps_wave_header->num_channels * m_ps_wave_header->bits_per_sample/8);
	i_read_samples = min(i_read_samples, i_samples);

	unsigned int i_tmp;
	unsigned char *pc_buffer_ptr = m_pc_buffer;

	for(int i=0;i<i_read_samples;i++)
	{
		for(int j=0;j<m_ps_wave_header->num_channels;j++)
		{
			i_tmp = 0;
			for(int k=0;k<m_ps_wave_header->bits_per_sample;k+=8)
			{
				i_tmp += (*pc_buffer_ptr << k);
				pc_buffer_ptr++;
			}
			ppi_pcm_buffer[j][i] = (int(i_tmp) > i_max_int ? int(i_tmp) - (i_max_int<<1) : int(i_tmp));
		}
	}

	int i_bytes_read = pc_buffer_ptr - m_pc_buffer;

	return i_bytes_read;
}

int	wave_read::fill_wave_buffer(int **ppi_pcm_buffer, int i_samples)
{
	if(m_pc_buffer == NULL)
	{
		fprintf(stderr, "File %s Line %d: ERROR Call init function before using fill_wave_buffer function.\n", 
			__FILE__,  __LINE__);
		exit(1);	
	}

	const int i_max_int = (1 << (m_ps_wave_header->bits_per_sample-1)) - 1;
	
	int i_bytes_to_read = m_ps_wave_header->num_channels * m_ps_wave_header->bits_per_sample/8 * i_samples;
	if(i_bytes_to_read > m_i_buff_size_in_bytes)
	{
		fprintf(stderr, "File %s Line %d: ERROR Bytes to read %d more than internal buffer size %d.\n", 
			__FILE__,  __LINE__, i_bytes_to_read, m_i_buff_size_in_bytes);
		exit(1);	
	}

	int i_read_chars = fread(m_pc_buffer, sizeof(unsigned char), i_bytes_to_read, m_f_wave_file);
	int i_read_samples = i_read_chars/(m_ps_wave_header->num_channels * m_ps_wave_header->bits_per_sample/8);
	i_read_samples = min(i_read_samples, i_samples);

	unsigned int i_tmp;
	unsigned char *pc_buffer_ptr = m_pc_buffer;

	for(int i=0;i<i_read_samples;i++)
	{
		for(int j=0;j<m_ps_wave_header->num_channels;j++)
		{
			i_tmp = 0;
			for(int k=0;k<m_ps_wave_header->bits_per_sample;k+=8)
			{
				i_tmp += (*pc_buffer_ptr << k);
				pc_buffer_ptr++;
			}
			ppi_pcm_buffer[j][i] = (int(i_tmp) > i_max_int ? int(i_tmp) - (i_max_int<<1) : int(i_tmp));
		}
	}

	int i_bytes_read = pc_buffer_ptr - m_pc_buffer;

	return i_bytes_read;
}

wave_read::~wave_read()
{
	delete m_ps_wave_header;
	delete m_pc_header_buffer;
	if(m_pc_buffer) delete [] m_pc_buffer;
	if(m_f_wave_file) fclose(m_f_wave_file);
}