#include <iostream>
#include <vector>
#include "../include/stdafx.hpp"
#include "../include/miniaudio.h"
#include <stdio.h>
#include "../include/mp3player.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

int main(int argc, char** argv)
{
//    Mp3Player::initMp3Player();
//    Mp3Player::loadPlaylistsDir("../playlists/");
//    Mp3Player::playSongAtIndex(0);

    std::thread signalThread(Mp3Player::processSignals); 
    // this starts up the Mp3player inits, and then listens for signals

//    Mp3Player::cleanupMp3Player();

    while(1){
        std::cin >> Mp3Player::signal;
        Mp3Player::recvdSignal = true;
        if(Mp3Player::signal == 7){
            signalThread.join();
            break;
        }
    }
    return 0;
}
