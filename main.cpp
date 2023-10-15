#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#include "DFT.hpp"
#include <iostream>
#include <cmath> // implement round() later to avoid deps
#include <thread>

#define GLSL_AUDIO_START 0
#define GLSL_AUDIO_STOP 1
#define GLSL_AUDIO_NEXT_SONG 2
#define GLSL_AUDIO_PREV_SONG 3
#define BITS 8
#define DEBUG


/*TODO:

Playlists:
They will look like this:
{"Playlistname":[{name:"Songname",path:"absolutepath"},{...},{...}]}
To determine Intensity create a sum of n values and divide trough 
// https://www.matheretter.de/wiki/shazam-pulse-code-modulation
Audio Playing / visualizing:

For Audio Playing there will be a buffer of 16 minutes(4MB if 16bit and stereo) that will be filled by mp3 data
This can be changed with settings for more or less buffering

if you want to skip around in the song it will start filling the buffer from bytes at that location

location can be found by using bytes_per_frame and ms per frame.
Basically we calc the number of frames per second (frames * bytes_per_frame)
first second (8 bit mono) --> start 0  end 24 * 1152
second second --> start 24*1152  end  24 * 1152 * 2

Ok so the simplest way to visualize audio would be to just run a second program/thread, that reads from the alsa device and displays the currently played audio
This has the side effect of also visualizing audio, that does not come from the music player ..., but that should be fine for now.


https://www.mpg123.de/api/group__mpg123__input.shtml#ga072669ae9bde29eea8cffa4be10c7345
https://mpg123.de/api/group__mpg123__seek.shtml // seeking pos i.e frames
Instead of loading all mp3 data to memory its possible to just skip around in the mp123 stream with mpg123_seek.
However i would have to put ao_play in its own thread to be able to pause, skip ...
*/

bool musicplayed= false;
bool g_samplebuffer_alloced = false;
mpg123_handle * mh;
//size_t done;
int err;
int driver;
size_t buffer_size;
ao_device *dev;
ao_sample_format format;
int channels, encoding;
long rate;
long long unsigned int offset=0; // offset in bytes
unsigned int bytesperframe=0;
double fps=0; // MPEG3Frames per second
double rawfps=0; // UNROUNDED frames per second
long long unsigned int bytesread = 0;
bool paused=false;

#define FRAMES_PLAY_PER_READ 4 // This would create a delay when pressing stop,play ..., but lessens the load on cpu
// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(width, height, "Music vis :)");
    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
}




// make this function somehow return that it has finished playing
void resume_play_at_sec(unsigned int sec){
	#ifdef DEBUG
		std::cout << "Buffer Size for Playing Audio: " << bytesperframe * FRAMES_PLAY_PER_READ << std::endl;
	#endif
	unsigned char buf[bytesperframe * FRAMES_PLAY_PER_READ]={0};
        mpg123_seek_frame(mh,unsigned( fps * sec ),SEEK_SET); // Its not 100% accurate a second here is like 1008 ms instead of 1000 but thats fine ... i think
	unsigned int mpg123_ret = MPG123_OK;
	long unsigned int done = 0;
	long unsigned int framesplayed = 0;
	long unsigned int current_sec = 0;
	while (paused == false && mpg123_ret == MPG123_OK){
		mpg123_ret = mpg123_read(mh, buf ,bytesperframe * FRAMES_PLAY_PER_READ, &done);
		ao_play(dev,(char*)buf,bytesperframe * FRAMES_PLAY_PER_READ);
		framesplayed += FRAMES_PLAY_PER_READ;
		if (framesplayed / fps >= 1) {
			current_sec +=1;
			framesplayed = 0;
		}
		#ifdef DEBUG
//			std::cout << "current Second: "<< current_sec << std::endl;
			std::cout << "done: " << done << std::endl;
		#endif
	}
}

void mpg_print_fmt(){
	std::cout << "bits: "<< format.bits << " rate:"<<format.rate << " channels:"<<format.channels <<  " ,byte_fmt:"<<format.byte_format << std::endl;
}

//https://stackoverflow.com/questions/6220660/calculating-the-length-of-mp3-frames-in-milliseconds
double get_frame_duration_ms_MP3(unsigned int sampleratehz){
	return 1152.0 / double(sampleratehz) * 1000.0; // This should be correct ?!
}

// converts milliseconds to seconds
double milli_secs(double millisec){
	return millisec / 1000.0;
}

void initPlay(char * abspath){
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	mpg123_open(mh, abspath);
	mpg123_getformat(mh, &rate, &channels, &encoding);
	format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
        format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = 0;
        bytesperframe = (format.bits/8 * channels * 1152);
    	rawfps =  1000.0 / double(get_frame_duration_ms_MP3(format.rate));
        fps = ceil(rawfps);
        #ifdef DEBUG
	std::cout << "Error: " << err << std::endl;
	std::cout << "Buffer Size: " << buffer_size << std::endl;
	std::cout << "Frames Per Buffer Read: " << buffer_size/4608 << std::endl;
	std::cout << "Milliseconds Per Buffer Read: " << get_frame_duration_ms_MP3(format.rate) * (buffer_size/4608) << std::endl;
	std::cout << "ms per frame: " << get_frame_duration_ms_MP3(format.rate) << std::endl;
        std::cout << "bytes per frame: " << bytesperframe << std::endl;
	std::cout << "frames per second: " << fps <<std::endl;
 	std::cout << "bytes per second: " << (format.bits/8 * channels * 1152) * (1000.0 / double(get_frame_duration_ms_MP3(format.rate))) << std::endl;
	#endif
}

void cleanup(){
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}

int main(int argc, char *argv[])
{
    SetTraceLogLevel(LOG_ERROR); // Disable annoying raylib error messages
    if(argc < 2) exit(0);
    ao_initialize();
    driver = ao_default_driver_id();
    initPlay(argv[1]);
    dev = ao_open_live(driver, &format, NULL);
    mpg_print_fmt();
    bool exit = false;
    unsigned int sec = 0;
    unsigned int cmd = 0;
    paused = true;
    std::thread t1;

    while(!exit){
	std::cin >> cmd;
	if(cmd == 0){
		paused = false;
		t1 = std::thread(resume_play_at_sec,200);
	}else if(cmd == 1){
		paused = true;
		t1.join();
	}
    }
	   // std::vector<double> spectrum = traceSpectrum(g_samplebuffer,bytespf,1024,0);
/*	    BeginDrawing();
            ClearBackground(RAYWHITE);
	    int x = 0;
	    for (int i = 0; i < spectrum.size();i+=1){
		    DrawLine(x,spectrum[i], x,0,RED);
		    x++;
	    }
            EndDrawing();*/
//	    threaded_ao_play();
   cleanup();
   return 0;
}
