#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#include "DFT.hpp"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#define BITS 8

/*TODO:
Playlists:
They will look like this:
{"Playlistname":[{name:"Songname",path:"absolutepath"},{...},{...}]}
To determine Intensity create a sum of n values and divide trough 
// https://www.matheretter.de/wiki/shazam-pulse-code-modulation
n*/

bool musicplayed= false;
bool g_samplebuffer_alloced = false;
std::mutex mtx;
std::condition_variable cv;

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

// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(width, height, "Music vis :)");
    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
}


void threaded_ao_play(){
	ao_play(dev, (char*)g_samplebuffer, done); // put this in its own thread, so that music and animation dont fight for cpu ressources
//	std::unique_lock<std::mutex> lock(mtx);
//	musicplayed = true;
//	cv.notify_one();
}

void initPlay(char * abspath){
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	if (g_samplebuffer_alloced){
		free(g_samplebuffer); // later on check if buffer  is big enough for new file instead of always freeing
		g_samplebuffer_alloced = true;
	}
	g_samplebuffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
	mpg123_open(mh, abspath);
	mpg123_getformat(mh, &rate, &channels, &encoding);
	std::cout << "Error: " << err << std::endl;
	format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
    	format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
   	format.matrix = 0;
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
    if(argc < 2) exit(0);
    ao_initialize();
    driver = ao_default_driver_id();
    initPlay(argv[1]);
    dev = ao_open_live(driver, &format, NULL);
    fprintf(stdout,"Buffer Size: %u  ",buffer_size);
    unsigned int nblocks=0;
    unsigned int res = 0;
    int boxPosY=0;
    init_shader_vis(1900,1024);
    while (mpg123_read(mh, g_samplebuffer, buffer_size, &done) == MPG123_OK && !WindowShouldClose() ){
//	    std::thread worker(threaded_ao_play);
	    std::vector<double> spectrum = traceSpectrum(g_samplebuffer,buffer_size,1024,10000);
	    BeginDrawing();
            ClearBackground(RAYWHITE);
	    int x=0;
	    for (int i = 0; i < spectrum.size();i+=8){
		    DrawLine(x,spectrum[i], x,0,RED);
		    x++;
	    }
            EndDrawing();
	    threaded_ao_play();
   }
   cleanup();
   return 0;
}
