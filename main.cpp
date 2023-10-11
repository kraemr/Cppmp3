#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <vector>
#include "include/raylib.h"
#define BITS 8






/*TODO:
Playlists:
They will look like this:
{"Playlistname":[{name:"Songname",path:"absolutepath"},{...},{...}]}
To determine Intensity create a sum of n values and divide trough 
// https://www.matheretter.de/wiki/shazam-pulse-code-modulation
n*/


// for now this uses raylib
// Also this will handle all the ui
void init_shader_vis(int width,int height){
    InitWindow(width, height, "raylib [core] example - input mouse wheel");
    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
}

int main(int argc, char *argv[])
{
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;
    int driver;
    ao_device *dev;
    ao_sample_format format;
    int channels, encoding;
    long rate;
    if(argc < 2) exit(0);
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    mpg123_open(mh, argv[1]);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
    fprintf(stdout," %u  ",buffer_size);
    unsigned int nblocks=0;
    unsigned int res = 0;
    int boxPosY=0;
    init_shader_vis(600,600);
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK && !WindowShouldClose() ){
	    res = 0;
	    nblocks++;
	    for (int i = 0; i < buffer_size; i++){
			res += buffer[i];
	    }
	    boxPosY = res/ ( buffer_size/2) * 2;
	    BeginDrawing();
            ClearBackground(RAYWHITE);
	    DrawRectangle(0,boxPosY,80,80,GREEN);
	    EndDrawing();
	    printf("summed-Intensity%u , blockid:%d, buffer_size:%u\n",res/ ( buffer_size/2),nblocks,buffer_size);
//		fprintf(fp,"\n\n\n\n");
	    ao_play(dev, (char*)buffer, done);
    }
    CloseWindow();        // Close window and OpenGL context
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
    return 0;
}
