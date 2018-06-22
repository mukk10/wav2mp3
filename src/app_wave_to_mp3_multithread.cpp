/**
* @file app_wave_to_mp3.cpp
* @author Muhammad Usman Karim Khan, karim.usman@yahoo.com
* @brief Convert wave file to mp3.
* Copyright 2017, Muhammad Usman Karim Khan, All rights reserved.
*/

#include <wave_to_mp3.h>
#include <pthread_queue.h>
#include <fstream>
#include <cstring>
#include <cstdlib>

#define		SOFTWARE_VERSION	"0.1"	//!< Release version
#define		QUEUE_LENGTH		50		//!< Maximum number of wave files processed concurrently in a batch

using namespace std;

typedef struct _thread_args
{
	char *pc_wave_file;
	char *pc_mp3_file;
	wave_to_mp3 **ppc_wave2mp3_objs;
} thread_args;

void show_usage(char *pc_prog_name)
{
	fprintf(stderr, "\nUsage: %s [-f file_name | -d directory] [-q quality] [-t threads] [-h]\n", pc_prog_name);
	fprintf(stderr, "-f file_name: wave file to convert into mp3\n");
	fprintf(stderr, "-d directory: directory path containing wave files which\n");
	fprintf(stderr, "              will all be converted into mp3 files.\n");
	fprintf(stderr, "              If this option is used, -f would be ignored.\n");
	fprintf(stderr, "-q quality:   MP3 quality, 0: highest (default), 9: lowest.\n");
	fprintf(stderr, "-t threads:   total number of threads to use.\n");
	fprintf(stderr, "-h:           show this help\n\n");
}

void *encode_to_mp3(void *p_args, int i_thread_id)
{
	thread_args *p_thread_args = (thread_args *)p_args;
	char *pc_wave_file = p_thread_args->pc_wave_file;
	char *pc_mp3_file = p_thread_args->pc_mp3_file;
	wave_to_mp3 *pc_wave2mp3 = p_thread_args->ppc_wave2mp3_objs[i_thread_id];

	pc_wave2mp3->init(pc_wave_file, pc_mp3_file);
	pc_wave2mp3->sanity_check();
	pc_wave2mp3->display_wave_info();
	pc_wave2mp3->encode_wave();
	return NULL;
}

int main(int argc, char **argv)
{
	fprintf(stderr, "Multithreaded wave to mp3 encoder version %s\n", SOFTWARE_VERSION);
	fprintf(stderr, "Copyright: Muhammad Usman Karim Khan <karim.usman@yahoo.com>\n\n");

	char pc_wave_file[1024];
	char pc_wave_dir[1024];
	int i_threads = 4;
	int i_quality = 0;

	wave_to_mp3 **ppc_wave2mp3 = NULL;
	pthread_queue *pc_thread_queue = new pthread_queue();
	thread_args **ppc_thread_args = NULL;

	int i_use_dir = 0;

	if(argc < 3)
	{
		show_usage(argv[0]);
		return 1;
	}

	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i], "-f") == 0)
			strcpy(pc_wave_file, argv[++i]);
		else if(strcmp(argv[i], "-d") == 0)
		{
			i_use_dir = 1;
			strcpy(pc_wave_dir, argv[++i]);
		}
		else if(strcmp(argv[i], "-q") == 0)
			i_quality = atoi(argv[++i]);
		else if(strcmp(argv[i], "-t") == 0)
			i_threads = atoi(argv[++i]);
		else if(strcmp(argv[i], "-h") == 0)
		{
			show_usage(argv[0]);
			return 1;
		}
		else
		{
			show_usage(argv[0]);
			return 1;
		}
	}

	if(i_use_dir == 1)
	{
		char pc_cmd[1024];

		// @todo need a better solution than using a system call. However, this is done only
		// once per program run, so it might be OK. 
#if defined _WIN32 || defined _WIN64
		sprintf(pc_cmd, "dir /b /s /a-d %s > wave_files.txt", pc_wave_dir);
#else
		sprintf(pc_cmd, "ls %s > wave_files.txt", pc_wave_dir);
#endif
		system(pc_cmd);
	}
	else
	{
		ofstream f_tmp("wave_files.txt");
		f_tmp << pc_wave_file << "\n";
		f_tmp.close();
	}

	// Threads
	// Number of threads = number of wave to mp3 convertors.
	ppc_wave2mp3 = new wave_to_mp3*[i_threads];
	for(int i=0;i<i_threads;i++)
	{
		ppc_wave2mp3[i] = new wave_to_mp3();
		ppc_wave2mp3[i]->set_quality(i_quality);
	}

	pc_thread_queue->make_thread_pool(i_threads, QUEUE_LENGTH);
	ppc_thread_args = new thread_args*[QUEUE_LENGTH];
	for(int i=0;i<QUEUE_LENGTH;i++)
	{
		ppc_thread_args[i] = new thread_args;
		ppc_thread_args[i]->pc_wave_file = new char[1024];
		ppc_thread_args[i]->pc_mp3_file = new char[1024];
		ppc_thread_args[i]->ppc_wave2mp3_objs = ppc_wave2mp3;
	}

	// Encoding
	// Use a batch of QUEUE_LENGTH wave files at a time to encode 
	ifstream f_wave_files("wave_files.txt");
	int i_encode_job = 0;
	while(f_wave_files >> ppc_thread_args[i_encode_job]->pc_wave_file)
	{
		char *pc_curr_wave_file = ppc_thread_args[i_encode_job]->pc_wave_file;
		char *pc_curr_mp3_file = ppc_thread_args[i_encode_job]->pc_mp3_file;

		int i_wave_file_len = strlen(pc_curr_wave_file);
		if(strncmp(pc_curr_wave_file+i_wave_file_len-4,".wav",4))
			continue;

		strcpy(pc_curr_mp3_file,"");
		strncat(pc_curr_mp3_file, pc_curr_wave_file, i_wave_file_len-4);
		strcat(pc_curr_mp3_file, ".mp3");

		pc_thread_queue->add_to_job_queue(encode_to_mp3, (void *)ppc_thread_args[i_encode_job]);
		if(++i_encode_job == QUEUE_LENGTH)	// Queue full, let the batch finish
		{
			pc_thread_queue->wait_queue_done();
			i_encode_job = 0;
		}
	}

	// Wait for queue to finish
	pc_thread_queue->wait_queue_done();

	f_wave_files.close();

	for(int i=0;i<i_threads;i++)
		delete ppc_wave2mp3[i];
	delete [] ppc_wave2mp3;

	
	for(int i=0;i<QUEUE_LENGTH;i++)
	{
		delete [] ppc_thread_args[i]->pc_wave_file;
		delete [] ppc_thread_args[i]->pc_mp3_file;
		delete ppc_thread_args[i];
	}
	delete [] ppc_thread_args;
	delete pc_thread_queue;

	return 0;
}
