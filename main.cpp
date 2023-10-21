#include <ao/ao.h>
#include <cmath>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#include "include/DFT.hpp"
#include "include/playlist.hpp"
#include "include/stdafx.hpp"
#include <fftw3.h>
#include <iostream>
#include <thread>

#define GLSL_AUDIO_START 0
#define GLSL_AUDIO_STOP 1
#define GLSL_AUDIO_NEXT_SONG 2
#define GLSL_AUDIO_PREV_SONG 3
#define BITS 8
#define DEBUG
/*TODO:
Implement a ringbuffer for auiovis/ mp3 playing,
where there will be two pointers, one where mpg frames are currently played, and one where the visualization is at.
Then if the difference between the two is too great, then either skip ahead  or load more at once to keep up.

https://www.mpg123.de/api/group__mpg123__input.shtml#ga072669ae9bde29eea8cffa4be10c7345
https://mpg123.de/api/group__mpg123__seek.shtml // seeking pos i.e frames

make visualization modular
instead of always starting a thread and stopping ... just send signals to the audio thread that get executed just once, this has obvious performance and latency improvements
also make stuff more atomic and avoid possible data race conditions



add the add song to playlist functionality
add the add remove from playlist functionality
remove playlist functionality:
just removes the given file
add playlist functionality:

add always shuffle to playlist
playlist merge (high prio)
most played sqlite (low prio)
add genre and custom stuff to songs (low prio)
read mp3 metadata, title,author ... (high prio)
song duration (high prio)
conversion of frames to seconds (high prio)
sound up/down (affects only the mp3 player) (high prio)
*/

bool musicplayed= false;
bool g_samplebuffer_alloced = false;
mpg123_handle * mh;
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
std::thread t1;//audio thread
std::thread visualize_thr;
unsigned int current_song_id = 0;
unsigned int current_playlist_id = 0;
bool thread_running = false;
unsigned int g_frame = 0; // current frame play function
std::vector<Song> playlist_songs;
std::vector<Playlist> playlists;
bool g_mpg_was_init=false;
bool g_ao_was_init=false;

void initPlay(char * abspath);
#define FRAMES_PLAY_PER_READ 16 // This would create a delay when pressing stop,play ..., but lessens the load on cpu
#define PRECOMPUTE_MEMORY_LIMIT 2000000
// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(1900, height, "Music vis :)");
    SetTargetFPS(24);               // Set our game to run at 60 frames-per-second
}

std::vector<double> normalizeFFTResults(const fftw_complex* fftResult, int size) {
    std::vector<double> normalizedResult(size);
    // Find the maximum magnitude in the FFT result
    double maxMagnitude = 0.0;
    for (int i = 0; i < size; i++) {
        double magnitude = sqrt(fftResult[i][0] * fftResult[i][0] + fftResult[i][1] * fftResult[i][1]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
        }
    }
    // Normalize the FFT results by the maximum magnitude
    for (int i = 0; i < size; i++) {
        normalizedResult[i] = sqrt(fftResult[i][0] * fftResult[i][0] + fftResult[i][1] * fftResult[i][1]) / maxMagnitude;
    }
    return normalizedResult;
}

void visualize(const char * abspath){
	SetTraceLogLevel(LOG_ERROR);
	init_shader_vis(1000,1000);
	unsigned short buf[(bytesperframe * FRAMES_PLAY_PER_READ)]={0};
	int verr;
	mpg123_handle * mh_vis;
	mh_vis = mpg123_new(NULL, &verr);
	mpg123_open(mh_vis, abspath);
	mpg123_outblock(mh_vis);
    mpg123_seek_frame(mh_vis,g_frame,SEEK_SET); // Its not 100% accurate a second here is like 1008 ms instead of 1000 but thats fine ... i think
	unsigned long tt = 0;
	unsigned long t = 0;
	unsigned long current_frame=0;
	fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * bytesperframe);
	std::vector<double> doubleBuffer(bytesperframe/2);
	while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		if(!paused){ // technically this is a data race between threads, but we just dont care :)
		    //mpg123_seek_frame(mh_vis,g_frame,SEEK_SET); // Its not 100% accurate a second here is like 1008 ms instead of 1000 but thats fine ... i think
			tt = mpg123_read(mh_vis, buf ,bytesperframe, &t);
			BeginDrawing();
			ClearBackground(BLACK);
				for(unsigned int i = 0;i <  FRAMES_PLAY_PER_READ;i++){

					//std::vector<double> spectrum = traceSpectrum(buf,bytesperframe,bytesperframe,0);
					for (size_t k = 0; k < bytesperframe/sizeof(short); k+=2) {
        				unsigned short sample = (static_cast<unsigned short>(buf[k]) << 8) | buf[ k+1];
						doubleBuffer[k] = sample;
    				}
					fftw_plan plan = fftw_plan_dft_r2c_1d(bytesperframe/2, doubleBuffer.data(), out, FFTW_ESTIMATE);
					std::vector <double> spectrum_normalized = normalizeFFTResults(out,bytesperframe/2);
					fftw_execute(plan);
    				fftw_destroy_plan(plan);
					int j =0;
					//std::cout << out[0]
					for(int j = 0; j < spectrum_normalized.size(); j++){
						//std::cout << spectrum_normalized[i] <<std::endl;
						DrawLine(j,spectrum_normalized[j] * 500,j,0,BLUE);
						j++;
					}
				//WaitTime(0.024);
			}
			char str[200];
				sprintf(str,"Frame: %lu", current_frame);
				DrawText(str,200,200,20,RED);
				current_frame+=1;
					EndDrawing();

		}

	}

}

// make this function somehow return that it has finished playing
void resume_play_at_frame(unsigned int frame,unsigned int* frame_ref){
	unsigned char buf[bytesperframe * FRAMES_PLAY_PER_READ]={0};
    mpg123_seek_frame(mh,frame,SEEK_SET); // Its not 100% accurate a second here is like 1008 ms instead of 1000 but thats fine ... i think
	unsigned int mpg123_ret = MPG123_OK;
	long unsigned int done = 0;
	long unsigned int framesplayed = 0;
	unsigned int ao_ret = 0;
	while (paused == false && mpg123_ret == MPG123_OK){
		mpg123_ret = mpg123_read(mh, buf ,bytesperframe * FRAMES_PLAY_PER_READ, &done);
	        (*frame_ref) += FRAMES_PLAY_PER_READ;
		ao_ret = ao_play(dev,(char*)buf,bytesperframe * FRAMES_PLAY_PER_READ);
		framesplayed += FRAMES_PLAY_PER_READ;
		#ifdef DEBUG
		//	std::cout << "Frame: " << framesplayed << std::endl;
		#endif
	}
	std::cout << "rate: " << format.rate << std::endl;
	if(!(current_song_id >= playlists[current_playlist_id].songs.size() - 1) && paused == false){
		current_song_id++;
		initPlay((char*)playlists[current_playlist_id].songs[current_song_id+1].filepath.c_str());
		g_frame = 0;
		resume_play_at_frame(0,&g_frame);
	}
	else{
		return;
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
	if(g_mpg_was_init){
	        mpg123_close(mh);
	        mpg123_delete(mh);
	}else{
		g_mpg_was_init = true;
	}
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
	if(g_ao_was_init){
	    ao_close(dev);
	    dev = ao_open_live(driver, &format, NULL);
	}else{
	    g_ao_was_init = true;
      	    dev = ao_open_live(driver, &format, NULL);
	}
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
	                initPlay((char*)playlists[current_playlist_id].songs[current_song_id].filepath.c_str());
			g_frame = 0;
					//visualize_thr = std::thread(visualize,(char*)playlists[current_playlist_id].songs[current_song_id].filepath.c_str());
			t1 = std::thread(resume_play_at_frame,g_frame,&g_frame);
			thread_running = true;
		}
        }else if(cmd == 1){ // pause current song
		if(thread_running == true){
                	paused = true;
                	t1.join();
                	thread_running = false;
		}
        }else if(cmd == 2){ // load next mp3 in playlist
                if (current_song_id >= playlists[current_playlist_id].songs.size() - 1){
                        current_song_id = 0;
                }else{
                        current_song_id++;
                }
		g_frame = 0;
		if(thread_running){
			paused = true;
			t1.join();
		}
		paused = false;
                initPlay((char*)playlists[current_playlist_id].songs[current_song_id].filepath.c_str());
		t1 = std::thread(resume_play_at_frame,g_frame,&g_frame);
        }else if(cmd == 3){ // load prev mp3 in playlist
                if(current_song_id == 0){
                        current_song_id = playlists[current_playlist_id].songs.size() - 1; // go to last song if on first song and skip back
                }else {
                        current_song_id--;
                }
		g_frame = 0;
                if(thread_running){
		
                }
                initPlay((char*)playlists[current_playlist_id].songs[current_song_id].filepath.c_str());
                t1 = std::thread(resume_play_at_frame,g_frame,&g_frame);
        }else if(cmd == 4){ // re read playlists and
	        playlists =  read_playlists_dir("playlists/");
	}else if(cmd == 5){
		shuffle_playlist(playlists[current_playlist_id].songs);
		playlists[current_playlist_id].shuffled = true;
		for(auto elem : playlists[current_playlist_id].songs){
			std::cout << elem.songname << std::endl;
		}
		g_frame = 0;
        	if(thread_running){
			std::terminate(t1);
                }
                initPlay((char*)playlists[current_playlist_id].songs[current_song_id].filepath.c_str());
                t1 = std::thread(resume_play_at_frame,g_frame,&g_frame);
	}else if(cmd == 6){
		cleanup();
		exit(0);
	}
//	std::cout << "currently Playing: " << playlist_songs[current_song_id].songname << std::endl;
}

int main(int argc, char *argv[])
{
    ao_initialize();
    driver = ao_default_driver_id();
    playlists =  read_playlists_dir("playlists/");
    initPlay((char*)playlists[0].songs[0].filepath.c_str());
    bool exit = false;
    unsigned int cmd = 0;
    paused = true;
    while(!exit){
		std::cout  << STDAFX_RED << "Enter cmd: " << STDAFX_RESET_COLOR << std::endl;
		std::cin >> cmd;
		processCmd(cmd);
    }
   cleanup();
   return 0;
}
