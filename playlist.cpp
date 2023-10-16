#include <fstream>
#include <iostream>
#include "include/json.hpp"
#include <vector>
#include "include/playlist.hpp"
#include <algorithm>
#include <random>
#include <sys/stat.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<Song> read_playlist_json(std::string filepath){
	std::vector<Song> playlist;
	std::ifstream f(filepath);
	json data = json::parse(f);
	for (json::iterator it = data.begin(); it != data.end(); ++it) {
		struct Song song;
		song.songname = (*it)["songname"];
		song.filepath = (*it)["path"];
		playlist.push_back(song);
	}
	return playlist;
}

// Use this if you load playlists from different locations
void merge_playlists(std::vector<Playlist>& playlist1,std::vector<Playlist> playlist2){

}

std::vector<Playlist> read_playlists_dir(std::string path){
	struct stat sb;
	for(const auto& entry : fs::directory_iterator(path)){
		std::filesystem::path filename = entry.path();
		std::string filename_str = filename.string();
		unsigned int stringlen = filename_str.length();
		const char * path = filename_str.c_str();
		if (stat(path, &sb) == 0 && !(sb.st_mode & S_IFDIR))std ::cout << path << std::endl;
	}
}

// This shuffles the songs in-mem
void shuffle_playlist(std::vector<Song>& songs){
	std::random_device rd;
	std::default_random_engine rng(rd());
	std::shuffle(songs.begin(), songs.end(), rng);
}
