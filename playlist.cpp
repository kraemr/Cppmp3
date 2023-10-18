#include <fstream>
#include <iostream>
#include "include/json.hpp"
#include <vector>
#include "include/playlist.hpp"
#include "include/stdafx.hpp"
#include <algorithm>
#include <random>
#include <sys/stat.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

// adapt the parsing to the new format of the json in playlists/playlist.json
// let read_playlist_json return a Playlist struct instead of a list of songs


Playlist read_playlist_json(std::string filepath){
	try{
	        std::vector<Song> playlist_songs;
        	Playlist playlist;
	        std::ifstream f(filepath);
		json data = json::parse(f);
		for (auto& el : data["songs"].items()){
			struct Song song;
			song.songname = el.value()["songname"];
			song.filepath = el.value()["path"];
			playlist_songs.push_back(song);
		}
		playlist.name = data["name"];
	        playlist.songs = playlist_songs;
		playlist.shuffled = false;
	        return playlist;
	}
	catch (const json::exception& e)
    	{
        	#ifdef DEBUG
			std::cout << e.what() << '\n'; 
		#endif
		Playlist playlist;
		playlist.name = "__ERROR_JSON";
		return playlist;
    	}
}

// Use this if you load playlists from different locations
void merge_playlists(std::vector<Playlist>& playlist1,std::vector<Playlist> playlist2){

}

void print_songs(){

}


std::vector<Playlist> read_playlists_dir(std::string path){
	struct stat sb;
	const std::string split = ".json";
	std::vector<Playlist> playlists;
	for(const auto& entry : fs::directory_iterator(path)){
		std::filesystem::path filename = entry.path();
		std::string filename_str = filename.string();
		unsigned int stringlen = filename_str.length();
		const char * path = filename_str.c_str();
		if (stat(path, &sb) == 0 && !(sb.st_mode & S_IFDIR)){
			Playlist playlist = read_playlist_json(filename_str);
			if(playlist.name == "__ERROR_JSON"){
				continue; // found and read playlist, but it isnt valid
			}
			playlists.push_back(playlist);
			std::cout << STDAFX_YELLOW << path << STDAFX_RESET_COLOR  << std::endl;
                        for(auto element : playlist.songs){
				std::cout << STDAFX_GREEN << element.songname << STDAFX_RESET_COLOR << std::endl;
				std::cout << element.filepath << std::endl;
			}
		}
	}
	return playlists;
}

// This shuffles the songs in-mem
void shuffle_playlist(std::vector<Song>& songs){
	std::random_device rd;
	std::default_random_engine rng(rd());
	std::shuffle(songs.begin(), songs.end(), rng);
}


