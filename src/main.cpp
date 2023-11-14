#include <chrono>
#include <iostream>
#include <thread>
#include "../include/stdafx.hpp"
#include "../include/mp3player.hpp"



int main(int argc, char** argv)
{
    std::thread signalThread(Mp3Player::processSignals); 
    // std::cout 
    while(1){
        std::cin >> Mp3Player::signal;
        Mp3Player::recvdSignal = true;

        if(Mp3Player::signal == 7){
            signalThread.join();
            break;
        }
    std::timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000;
    nanosleep(&ts, nullptr);
    }
    return 0;
}
