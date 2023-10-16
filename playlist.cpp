

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

using json = nlohmann::json;
std::vector<Song> read_playlist_json(std::string filepath){
	std::vector<Song> playlist;
	std::ifstream f(filepath);
	json data = json::parse(f);
	for (json::iterator it = data.begin(); it != data.end(); ++it) {
		struct Song song;
		song.songname = (*it)["songname"];
		song.filepath = (*it)["path"];
//		std::cout << *el << std::endl;
		playlist.push_back(song);
	}
	return playlist;
}


