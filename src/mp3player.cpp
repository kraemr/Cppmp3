#include "../include/miniaudio.h"
#include "../include/playlist.hpp"
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
volatile bool recvdSignal=false;
//i32 playStatus=STOPPED;
i32 currentSongId=0;
i32 signal=0;
std::vector<Playlist> playlistsVec;
Playlist* currentPlaylist=nullptr;
std::mutex mtx;
std::condition_variable cv;
std::lock_guard<std::mutex> lock(mtx);// Give the Mp3Player its own Thread

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
    if(playOnLoad){
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


// use this in a thread
void processSignals(){
    i32 res=0;
    while(1){
    cv.wait(lock, [] { return recvdSignal; });
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
    switch (signal) {
    case 0: result = ma_sound_stop(&sound);break; // 0 Stop
    case 1: result = ma_sound_start(&sound);break;
    case 2: 
        currentSongId++;        // Autoplay next Song
        if(currentPlaylist != nullptr && currentSongId < currentPlaylist->songs.size()){
            res = (i32)playSongAtIndex(currentSongId);
        }
        else if(currentPlaylist != nullptr && currentSongId >= currentPlaylist->songs.size()){
            currentSongId = 0;
            res = (i32)playSongAtIndex(currentSongId);
        }
    ;break; // skip song
    case 3:
        currentSongId--;
        if(currentPlaylist != nullptr && currentSongId < 0){
            res = (i32)playSongAtIndex(currentSongId);
        }
        else if(currentPlaylist != nullptr && currentSongId < 0){
            currentSongId = currentPlaylist->songs.size() - 1;
            res = (i32)playSongAtIndex(currentSongId);
        }
    ;break; // prev song
    case 4: 
    /*Shuffle the songs list in the current playlist and set shuffled = true in the playlist Struct*/
    ;break; // Shuffle
    case 5:
    // Go to nth frame (User has selected a second to resume play from)
    ;break;
    } // end of switch

  
    }
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
}


}
