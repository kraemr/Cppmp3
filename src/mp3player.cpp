#include "../include/miniaudio.h"
#include "../include/playlist.hpp"
#include "../include/mp3player.hpp"
#include "../include/stdafx.hpp"
#include <cstddef>
namespace Mp3Player
{
#define STOPPED 0
#define PLAYING 1

ma_result result;
ma_engine engine;
ma_sound sound;
bool playOnLoad=true;
bool isInitialized=false;
bool playlistLoaded=false;
i32 playStatus=STOPPED; 
std::vector<Playlist> playlistsVec;
Playlist* currentPlaylist;

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