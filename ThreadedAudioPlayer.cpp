#include <iostream>
#include <thread>

void playBufferAudio(unsigned char * buffer, unsigned int buffer_size){
	std::cout << "Threaded playBufferAudio size:" << buffer_size << std::endl;
}
