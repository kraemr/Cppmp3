#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#include "include/DFT.hpp"
#include "include/playlist.hpp"
#include <iostream>
#include <cmath> // implement round() later to avoid deps
#include <thread>
#define GLSL_AUDIO_START 0
#define GLSL_AUDIO_STOP 1
#define GLSL_AUDIO_NEXT_SONG 2
#define GLSL_AUDIO_PREV_SONG 3
#define BITS 8
//#define DEBUG
/*TODO:
Implement a ringbuffer for auiovis/ mp3 playing,
where there will be two pointers, one where mpg frames are currently played, and one where the visualization is at.
Then if the difference between the two is too great, then either skip ahead  or load more at once to keep up.

https://www.mpg123.de/api/group__mpg123__input.shtml#ga072669ae9bde29eea8cffa4be10c7345
https://mpg123.de/api/group__mpg123__seek.shtml // seeking pos i.e frames
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
std::thread t1;
unsigned int current_song_id = 0;
bool thread_running = false;
unsigned int frame = 0; // current frame
std::vector<Song> playlist_songs;
std::vector<Playlist> playlists;
#define FRAMES_PLAY_PER_READ 4 // This would create a delay when pressing stop,play ..., but lessens the load on cpu

// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(width, height, "Music vis :)");
    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
}




// make this function somehow return that it has finished playing
void resume_play_at_frame(unsigned int frame,unsigned int* frame_ref){
	#ifdef DEBUG
		std::cout << "Buffer Size for Playing Audio: " << bytesperframe * FRAMES_PLAY_PER_READ << std::endl;
	#endif
	unsigned char buf[bytesperframe * FRAMES_PLAY_PER_READ]={0};
        mpg123_seek_frame(mh,frame,SEEK_SET); // Its not 100% accurate a second here is like 1008 ms instead of 1000 but thats fine ... i think
	unsigned int mpg123_ret = MPG123_OK;
	long unsigned int done = 0;
	long unsigned int framesplayed = 0;
	long unsigned int current_sec = 0;
	unsigned int ao_ret = 0;
	while (paused == false && mpg123_ret == MPG123_OK){
		mpg123_ret = mpg123_read(mh, buf ,bytesperframe * FRAMES_PLAY_PER_READ, &done);
                *frame_ref += FRAMES_PLAY_PER_READ;
		ao_ret = ao_play(dev,(char*)buf,bytesperframe * FRAMES_PLAY_PER_READ);
		framesplayed += FRAMES_PLAY_PER_READ;
		#ifdef DEBUG
			std::cout << "current Second: "<< current_sec << std::endl;
			std::cout << "ao_ret: " << ao_ret << std::endl;
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

void processCmd(unsigned int cmd){
        if(cmd == 0){       // start current song
		if(thread_running == false){
                	paused = false;
	                initPlay((char*)playlist_songs[current_song_id].filepath.c_str());
			t1 = std::thread(resume_play_at_frame,frame,&frame);
			thread_running = true;
		}
        }else if(cmd == 1){ // pause current song
		if(thread_running == true){
                	paused = true;
                	t1.join();
                	thread_running = false;
		}
        }else if(cmd == 2){ // load next mp3 in playlist
                if (current_song_id >= playlist_songs.size() - 1){
                        current_song_id = 0;
                }else{
                        current_song_id++;
                }
                initPlay((char*)playlist_songs[current_song_id].filepath.c_str());
        }else if(cmd == 3){ // load prev mp3 in playlist
                if(current_song_id == 0){
                        current_song_id = playlist_songs.size() - 1; // go to last song if on first song and skip back
                }else {
                        current_song_id--;
                }
                initPlay((char*)playlist_songs[current_song_id].filepath.c_str());
        }
//	std::cout << "currently Playing: " << playlist_songs[current_song_id].songname << std::endl;
}

int main(int argc, char *argv[])
{
    ao_initialize();
    driver = ao_default_driver_id();
    playlist_songs  = read_playlist_json("playlists/playlist.json");
    initPlay((char*)playlist_songs[0].filepath.c_str());
    dev = ao_open_live(driver, &format, NULL);
    bool exit = false;
    unsigned int cmd = 0;
    paused = true;
    while(!exit){
	std::cout << "Enter cmd: " << std::endl;
	std::cin >> cmd;
	processCmd(cmd);
   }
   cleanup();
   return 0;
}
