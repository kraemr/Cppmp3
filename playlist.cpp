

// A playlist is just a json file with a name and json objects that look like this:
/*
{
	"SongName":"Never Gonna Give You Up",
	"By":"Rick Astley",
	"Image":"../Path/ToImg..."
	"filepath":"../path/to/mp3file"
},
{ //Another song
   ...
}
*/
#include <fstream>
#include <iostream>
#include "include/json.hpp"
#include <vector>
#include "include/playlist.hpp"
#include <algorithm>
#include <random>

using json = nlohmann::json;
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



// This shuffles the songs in-mem
void shuffle_playlist(std::vector<Song>& songs){
	std::random_device rd;
	std::default_random_engine rng(rd());
	std::shuffle(songs.begin(), songs.end(), rng);
}



