# wav2mp3
Multithreade Wave to MP3 Encoder

## Software Workflow

Application *Top/Main* takes in the following arguments:
- Directory containing wave files
- Number of threads to use for encoding
- Quality of encoding

The *main* then pushes the workitems (wave files to
encode) in a *pthread queue*, which is just a FIFO to hold
job information (e.g., name and location of the wave file,
name and location of the output MP3 file).

The *thread handlers* then pop jobs out of the *queue* and
do the following tasks:
- Reach PCM from the wave files
- Encodes to MP3 via [Lame Encoder](www.mp3dev.org)
- Writes to the output MP3 file

## Build

On Linux, just hit `make`. 

## Usage

```
app*.exe [-f file_name | -d directory] [-t threads] \
	[-q quality] [-h]
```

e.g., 
```
app.exe -d /wave_files/ -t 8 -q 4
```

This will generate mp3 files (with the same names, but
extension .mp3 instead of .wav) from all the files with
extension .wav in `/wave_files/` directory. 

## Limitations and Known Issues

- Can only handle simple wave files, with a fixed header size
of [44 bytes](http://soundfile.sapp.org/doc/WaveFormat/).
- Only PCM data
- Disk might become the bottleneck and the number of parallel 
threads will not impact the performance. 

