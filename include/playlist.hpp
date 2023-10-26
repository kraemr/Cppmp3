#include <string>
#include <vector>
struct Song {
        std::string songname;
        std::string filepath;
};
struct Playlist {
	std::string name; // name of playlist (is used to identify --> must be unique)
	std::string path;
	std::vector<Song> songs;
	bool shuffled;
};
Playlist read_playlist_json(std::string filepath);
void shuffle_playlist(std::vector<Song>& songs);
std::vector<Playlist> read_playlists_dir(std::string path);
