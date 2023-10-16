#include <string>
struct Song {
        std::string songname;
        std::string filepath;
};
std::vector<Song> read_playlist_json(std::string filepath);
