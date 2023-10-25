namespace Mp3Player
{
int initMp3Player();
int initPlaySound(const std::string filepath);
void cleanupMp3Player();
void loadPlaylistsDir(const std::string filepath);
int playSongAtIndex(int i);
}
