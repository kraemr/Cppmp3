/*
MIT License

Copyright (c) 2023 Robin Krämer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include "../include/miniaudio.h"
#include "../include/mp3player.hpp"
#include "../include/stdafx.hpp"
namespace Mp3Player
{
ma_result result;
ma_engine engine;
ma_sound sound;

volatile bool playOnLoad=true;
volatile bool isInitialized=false;
volatile bool playlistLoaded=false;

std::atomic <bool> recvdSignal=false; // HAS to be atomic to avoid data race shenanigans
volatile bool exit=false;

i32 currentSongId=0;
i32 signal=0;
i64 nano_delay = 10000000;
bool amplification_allowed = false;
float volume = 0.8f;
Song currentSong;
std::vector<Playlist> playlistsVec;
Playlist* currentPlaylist=nullptr;

i32 initMp3Player(){
    exit = false;
    result = (ma_result)ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return -1;
    }
    std::cout << "init Mp3" << std::endl;
    isInitialized = true;
    return 0;
}



// You can pass 100% for full volume and 0% for muting
void setVolumePercent(float percent){
    if(amplification_allowed){
        volume = percent/100.0f;
        ma_engine_set_volume(&engine,volume);
    }
    else{
        percent = percent > 100.0f ? 100.0f : percent;
        volume = percent/100.0f;
        ma_engine_set_volume(&engine,volume);
    }
}

unsigned long long getCurrentFrame(){

}


float getVolumePercent(){
    return volume * 100.0f;
}


i32 initPlaySound(const std::string filepath){
    (void)ma_sound_uninit(&sound); // uninit seems to be safe to use when uninitialized :)
    result = (ma_result)ma_sound_init_from_file(&engine, filepath.c_str(), 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize sound");
        return -1;
    }
    if(playOnLoad == true){
        result = (ma_result)ma_sound_start(&sound);
    }
    return 0;
}
//Deallocate everything
void cleanupMp3Player(){
    if(isInitialized){
        ma_engine_uninit(&engine);
    }
}

i32 playSongAtIndex(i32 i){
    if(currentPlaylist == nullptr){
        printf("Error No playlist loaded, but tried to play a song from it!!\n");
        return -1;
    }
    if(i >= currentPlaylist->songs.size()){
        return 1;
    }
    currentSong = currentPlaylist->songs[i];
    i32 res = initPlaySound(currentPlaylist->songs[i].filepath);
    if(res == -1){
        printf("Failed to playSongAtIndex %d\n",i);
        return -1;
    }
    return 0;
}

void sendSignal(unsigned int signal){
    signal = signal;
    recvdSignal =  true;
}

// use this in a thread
void processSignals(){
    i32 res=0;
    initMp3Player();
    loadPlaylistsDir("../playlists");
    playOnLoad = false;
    playSongAtIndex(currentSongId);
    playOnLoad = true;
    while(!exit){
    if(ma_sound_at_end(&sound)){
        currentSongId++;        // Autoplay next Song
        if(currentPlaylist != nullptr && currentSongId < currentPlaylist->songs.size()){
            res = (i32)playSongAtIndex(currentSongId);
        }
        else if(currentPlaylist != nullptr && currentSongId >= currentPlaylist->songs.size()){
            currentSongId = 0;
            res = (i32)playSongAtIndex(currentSongId);
        }
    }
    if(recvdSignal){
        switch (signal) {
        case 1: 
        result = ma_sound_stop(&sound);
        break; // 0 Stop
        case 2: 
        result = ma_sound_start(&sound);
        break;
        case 3: 
        currentSongId++;        // Autoplay next Song
        std::cout << "current:" << currentSongId << std::endl;
        if(currentPlaylist != nullptr && currentSongId < currentPlaylist->songs.size()){
            res = (i32)playSongAtIndex(currentSongId);
        }
        else if(currentPlaylist != nullptr && currentSongId >= currentPlaylist->songs.size()){
            currentSongId = 0;
            res = (i32)playSongAtIndex(currentSongId);
        }
    ;break; // skip song
    case 4:
        currentSongId--;
        if(currentPlaylist != nullptr && currentSongId >= 0){
            res = (i32)playSongAtIndex(currentSongId);
        }
        else if(currentPlaylist != nullptr && currentSongId < 0){
            currentSongId = currentPlaylist->songs.size() - 1;
            res = (i32)playSongAtIndex(currentSongId);
        }
    ;break; // prev song
    case 5: 
    if(currentPlaylist != nullptr){
        save_changed_playlist(currentPlaylist);
    }else{
        std::cout << "ERROR currentPlaylist Pointer Was Null" << std::endl;
    }
    /*Shuffle the songs list in the current playlist and set shuffled = true in the playlist Struct*/
    ;break; // Shuffle
    case 6:
    // Go to nth frame (User has selected a second to resume play from)
    ;break;
    case 7:
    exit = true;
    ;break; // Exit Thread
    default:
    //Do nothing
    ;break;
    } // end of switch
    recvdSignal = false; // Always reset to 0 at end of loop
 //   lock.lock();
    }
    std::timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = nano_delay;
    nanosleep(&ts, nullptr);
    }
    cleanupMp3Player();
}

// searches Song By its name attribute
// returns the first match
Song findSongByName(){
    Song song;
    return song;
}
// Later enable loading playlists from multiple directories/filepaths
void loadPlaylistsDir(const std::string filepath){
    playlistsVec = read_playlists_dir(filepath);
    if(playlistsVec.size() == 0){
        return;
    }
    currentPlaylist = &playlistsVec[0]; 
    // Later read from a hidden file the last one, that was played
}


}
