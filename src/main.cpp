#include <iostream>
#include <vector>
#include "../include/stdafx.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "../include/miniaudio.h"
#include <stdio.h>
#include "../include/mp3player.hpp"


int main(int argc, char** argv)
{

    // Give the Mp3Player its own Thread
    Mp3Player::initMp3Player();
    Mp3Player::loadPlaylistsDir("../playlists/");
    Mp3Player::playSongAtIndex(0);
    Mp3Player::processSignals();
    Mp3Player::cleanupMp3Player();
    return 0;
}
