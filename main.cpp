#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#include "DFT.hpp"
#include <iostream>
#include <cmath> // implement round() later to avoid deps
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

*/
bool musicplayed= false;
bool g_samplebuffer_alloced = false;
mpg123_handle * mh;
unsigned char * g_samplebuffer; // The buffer that contains the PCM samples
size_t done;
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


// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(width, height, "Music vis :)");
    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
}



void mpg_print_fmt(){
	std::cout << "bits: "<< format.bits << " rate:"<<format.rate << " channels:"<<format.channels <<  " ,byte_fmt:"<<format.byte_format << std::endl;
}

// This fills the sample buffer with n seconds of samples
long long unsigned int fill_buffer(){

}


// gets the frame duration
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
	if (g_samplebuffer_alloced){
		free(g_samplebuffer); // later on check if buffer  is big enough for new file instead of always freeing
		g_samplebuffer_alloced = true;
	}
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
	std::cout << "frames per second" << fps <<std::endl;

 	std::cout << "bytes per second" << (format.bits/8 * channels * 1152) * (1000.0 / double(get_frame_duration_ms_MP3(format.rate))) << std::endl;
	#endif
        g_samplebuffer = (unsigned char*) malloc(100 * 1000 * 1000 * sizeof(unsigned char));
}


void start_play_At(unsigned int second){
}

void readSecond(){
    mpg123_read(mh, g_samplebuffer, fps * bytesperframe, &done);
}

void readFileIntoBuffer(){
	int ret=0;
	long unsigned int done;
	bytesread=0;
	while (ret == MPG123_OK){
		ret = mpg123_read(mh, g_samplebuffer,bytesperframe, &done);
		g_samplebuffer += bytesperframe; // Move the pointer so the buffer gets filled
		bytesread += bytesperframe;
	}
	g_samplebuffer -= bytesread; // reset the pointer to 0 index
}


void cleanup(){
    CloseWindow();        // Close window and OpenGL context
    free(g_samplebuffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}

int main(int argc, char *argv[])
{
    SetTraceLogLevel(LOG_ERROR);
    if(argc < 2) exit(0);
    ao_initialize();
    driver = ao_default_driver_id();
    initPlay(argv[1]);
    dev = ao_open_live(driver, &format, NULL);
    mpg_print_fmt();
    init_shader_vis(1900,1024);
    readFileIntoBuffer();

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
    

//    unsigned int fpsbytes = fps * bytesperframe;
    unsigned int second=0;
    while( offset < bytesread - rawfps * bytesperframe){
	//std::cin >> second;
	offset = second * fps * bytesperframe;
	ao_play(dev, (char*)g_samplebuffer+offset, rawfps * bytesperframe); // put this in its own thread, so that music and animation dont fight for cpu ressources
	second++;
  }
    #ifdef DEBUG
//	std::cout << fpsbytes <<std::endl; 
    #endif
// }
   cleanup();
   return 0;
}
