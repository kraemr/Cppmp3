#include <iostream>
#define MINIAUDIO_IMPLEMENTATION
#include "../include/miniaudio.h"
#include "../include/playlist.hpp"
#include "../include/mp3player.hpp"
#include "../include/stdafx.hpp"
#include <atomic>
#include <mutex>

namespace Mp3Player
{
ma_result result;
ma_engine engine;
ma_sound sound;
volatile bool playOnLoad=true;
volatile bool isInitialized=false;
volatile bool playlistLoaded=false;
std::atomic <bool> recvdSignal=false;
volatile bool exit=false;
//i32 playStatus=STOPPED;
i32 currentSongId=0;
i32 signal=0;
std::vector<Playlist> playlistsVec;
Playlist* currentPlaylist=nullptr;
std::mutex mtx;
std::condition_variable cv;
std::unique_lock<std::mutex> lock;// Give the Mp3Player its own Thread

i32 initMp3Player(){
    result = (ma_result)ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return -1;
    }
    isInitialized = true;
    return 0;
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
   // lock = std::unique_lock(mtx);
    i32 res=0;
    initMp3Player();
    loadPlaylistsDir("../playlists");
    playOnLoad = false;
    playSongAtIndex(currentSongId);
    playOnLoad = true;
    while(!exit){
    //cv.wait(lock, []{ return recvdSignal; });
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
        case 1: result = ma_sound_stop(&sound);break; // 0 Stop
        case 2: result = ma_sound_start(&sound);break;
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
    /*Shuffle the songs list in the current playlist and set shuffled = true in the playlist Struct*/
    ;break; // Shuffle
    case 6:
    // Go to nth frame (User has selected a second to resume play from)
    ;break;
    case 7:
    exit = true;
    ;break; // Exit Thread
    } // end of switch
    recvdSignal = false; // Always reset to 0 at end of loop
 //   lock.lock();
    }
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
