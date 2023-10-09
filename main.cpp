#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>

#define BITS 8


/*
TODO:
Playlists:
They will look like this:
{"Playlistname":[{name:"Songname",path:"absolutepath"},{...},{...}]}

To determine Intensity create a sum of n values and divide trough n

*/


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

    if(argc < 2)exit(0);
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
    printf("%u",buffer_size);
   while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
	        for (int i = 0 ; i < buffer_size; i++){
//		printf("%d ",buffer[i]);
		}
//		printf("\n\n\n\n");
		ao_play(dev, (char*)buffer, done);
	}
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
    return 0;
}
