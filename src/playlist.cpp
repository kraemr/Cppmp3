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

#include <cstddef>
#include <fstream>
#include <iostream>
#include "../include/json.hpp"
#include <vector>
#include "../include/playlist.hpp"
#include "../include/stdafx.hpp"
#include <algorithm>
#include <random>
#include <sys/stat.h>
using json = nlohmann::json;
namespace fs = std::filesystem;
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
		playlist.path = filepath;
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

// This creates a json_file with the same name as
void save_changed_playlist(Playlist* playlist){ 
	if(playlist == nullptr || playlist == NULL ){
		return;
	}
	try{
	json data;
	Playlist p = read_playlist_json(playlist->path);
	if(p.name == "__ERROR_JSON"){
		return;
	}
	data["name"] = p.name;
	data["path"] = p.path;
	std::cout << "orig Path" << p.path << std::endl;
	json songsJsonArr;
	for (const Song& song : p.songs){
		json songJson;
		songJson["songname"] = song.songname;
		songJson["path"] = song.filepath;
		songsJsonArr.push_back(songJson);
	}
    data["songs"] = songsJsonArr;
    std::string jsonStr = data.dump();
    std::ofstream outputFile(playlist->path);
	if (outputFile.is_open()) {
        outputFile << jsonStr;
        outputFile.close();
    } else {

    }
	}
	catch (const json::exception& e){
		std::cout << "Error Saving Playlist "<< std::endl;
		return;
	}
	


}

// Use this if you load playlists from different locations
void merge_playlists(std::vector<Playlist>& playlist1,std::vector<Playlist> playlist2){
	return;
}

void print_songs(Playlist& playlist){
	/*std::cout << STDAFX_YELLOW << playlist->name << ": " << std::endl;
	for (struct Song song : playlist->songs){
		std::cout << "songname: " << song.songname << "filepath: " << song.filepath << std::endl;
	}
	std::cout << STDAFX_RESET_COLOR <<std::endl;*/
}



std::vector<Playlist> read_playlists_dir(std::string path){
	struct stat sb;
	const std::string split = ".json";
	std::vector<Playlist> playlists;
	for(const auto& entry : fs::directory_iterator(path)){
		std::filesystem::path filename = entry.path();
		std::string filename_str = filename.string();
		const char * path = filename_str.c_str();
		if (stat(path, &sb) == 0 && !(sb.st_mode & S_IFDIR)){
			Playlist playlist = read_playlist_json(filename_str);
			if(playlist.name == "__ERROR_JSON"){
				continue; // found and read playlist, but it isnt valid
			}
			playlist.path = filename_str;
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
void shuffle_playlist(Playlist* playlist){
	if(playlist == nullptr || playlist == NULL){
		return;
	}
	std::random_device rd;
	std::default_random_engine rng(rd());
	playlist->shuffled = true;
	std::shuffle(playlist->songs.begin(), playlist->songs.end(), rng);
}


